#!/bin/bash

RES_COL=60
MOVE_TO_COL="echo -en \\033[${RES_COL}G"
SETCOLOR_SUCCESS="echo -en \\033[1;32m"
SETCOLOR_FAILURE="echo -en \\033[1;31m"
SETCOLOR_WARNING="echo -en \\033[1;33m"
SETCOLOR_NORMAL="echo -en \\033[0;39m"

MYSQL_BASE=/opt/mysql-5.7

echo_message() {
    $MOVE_TO_COL
    echo -n "["
    $SETCOLOR_SUCCESS
    echo -n $1
    $SETCOLOR_NORMAL
    echo -n "]"
    echo -ne "\r"
    echo
    return 1
}

mysql_build() {
    echo -n "0. Clean data"
    rm -rf /tmp/{mysql-master,mysql-slave}
    mkdir /tmp/{mysql-master,mysql-slave}
    echo_message "Done"

    echo -n "1. Init MySQL"
    $MYSQL_BASE/bin/mysqld --initialize-insecure --basedir=$MYSQL_BASE  \
            --datadir=/tmp/mysql-master --user=mysql 1>/dev/null 2>&1
    echo_message "Master Done"
    $MYSQL_BASE/bin/mysqld --initialize-insecure --basedir=$MYSQL_BASE \
            --datadir=/tmp/mysql-slave  --user=mysql 1>/dev/null 2>&1
    echo_message "Slave Done"

    echo -n "2. Prepare configuration"
    cat <<- EOF > /tmp/mysql-master/my.cnf
[mysqld]
binlog_format   = mixed
log_warnings    = 1
log_error       = localhost.err
log-bin         = mysql-bin
basedir         = $MYSQL_BASE
socket          = /tmp/mysql-master.sock
pid-file        = /tmp/mysql-master.pid
datadir         = /tmp/mysql-master
port            = 3307
server-id       = 1
EOF
    echo_message "Master Done"
    cat << EOF > /tmp/mysql-slave/my.cnf
[mysqld]
binlog_format   = mixed
log_warnings    = 1
log_error       = localhost.err
log-bin         = mysql-bin
basedir         = $MYSQL_BASE
socket          = /tmp/mysql-slave.sock
pid-file        = /tmp/mysql-slave.pid
datadir         = /tmp/mysql-slave
port            = 3308
server-id       = 2
relay_log_index = relay-bin.index
relay_log       = relay-bin
report_host     = 127.1
report_port     = 3308
master-info-repository = table
relay-log-info-repository = table
EOF
    echo_message "Slave Done"

    echo -n "3. Start MySQL Server"
    $MYSQL_BASE/bin/mysqld --defaults-file=/tmp/mysql-master/my.cnf  \
        --basedir=$MYSQL_BASE --datadir=/tmp/mysql-master            \
         --skip-grant-tables > /dev/null 2>&1 &
    echo_message "Master Done"
    $MYSQL_BASE/bin/mysqld --defaults-file=/tmp/mysql-slave/my.cnf  \
        --basedir=$MYSQL_BASE --datadir=/tmp/mysql-slave \
         --skip-grant-tables > /dev/null 2>&1 &
    echo_message "Slave Done"

    echo -n "4. Reset password"
    sleep 2
    $MYSQL_BASE/bin/mysql -uroot -S/tmp/mysql-master.sock \
        -e "UPDATE mysql.user SET authentication_string=PASSWORD('new-password') WHERE user='root'"
    echo_message "Master Done"
    $MYSQL_BASE/bin/mysql -uroot -S/tmp/mysql-slave.sock  \
        -e "UPDATE mysql.user SET authentication_string=PASSWORD('new-password') WHERE user='root'"
    echo_message "Slave Done"

    echo -n "5. Restart MySQL server"
    $MYSQL_BASE/bin/mysqladmin -uroot -S /tmp/mysql-master.sock shutdown
    echo_message "Shutdown Master Done"
    sleep 1
    $MYSQL_BASE/bin/mysqld --defaults-file=/tmp/mysql-master/my.cnf  \
        --basedir=$MYSQL_BASE --datadir=/tmp/mysql-master > /dev/null 2>&1 &
    echo_message "Start Master Done"
    $MYSQL_BASE/bin/mysqladmin -p'new-password' -uroot -S /tmp/mysql-master.sock ping \
        >/dev/null 2>&1 && echo_message "ERROR" || echo_message "Check OK"

    $MYSQL_BASE/bin/mysqladmin -uroot -S /tmp/mysql-slave.sock shutdown
    echo_message "Shutdown Slave Done"
    sleep 1
    $MYSQL_BASE/bin/mysqld --defaults-file=/tmp/mysql-slave/my.cnf  \
        --basedir=$MYSQL_BASE --datadir=/tmp/mysql-slave  > /dev/null 2>&1 &
    echo_message "Start Slave Done"
    $MYSQL_BASE/bin/mysqladmin -p'new-password' -uroot -S /tmp/mysql-slave.sock ping \
        >/dev/null 2>&1 && echo_message "ERROR" || echo_message "Check OK"

    echo -n "6. Start replication"
    sleep 3
    $MYSQL_BASE/bin/mysql -uroot -S/tmp/mysql-master.sock -p"new-password" 2>/dev/null \
            -e "GRANT REPLICATION SLAVE ON *.* to 'mysync'@'localhost' IDENTIFIED BY 'kidding'"
    sleep 1
    file=`$MYSQL_BASE/bin/mysql -uroot -S/tmp/mysql-master.sock -p"new-password" 2>/dev/null     \
            -e "SHOW MASTER STATUS\G" | grep "File" | awk '{print $NF}'`
    position=`$MYSQL_BASE/bin/mysql -uroot -S/tmp/mysql-master.sock -p"new-password" 2>/dev/null \
            -e "SHOW MASTER STATUS\G" | grep "Position" | awk '{print $NF}'`
    sql="CHANGE MASTER TO master_host='localhost',master_port=3307, master_user='mysync',
         master_password='kidding', master_log_file='$file',master_log_pos=$position"
    $MYSQL_BASE/bin/mysql -uroot -S/tmp/mysql-slave.sock -p"new-password" 2>/dev/null \
            -e "$sql"
    $MYSQL_BASE/bin/mysql -uroot -S/tmp/mysql-slave.sock -p"new-password" 2>/dev/null \
            -e "START SLAVE"
    echo_message "Done"
}

case "$1" in
    build)
        mysql_build
        ;;
    stop)
        echo -n "Shutdown MySQL server"
        $MYSQL_BASE/bin/mysqladmin -uroot -S /tmp/mysql-slave.sock shutdown
        echo_message "Shutdown Slave Done"
        $MYSQL_BASE/bin/mysqladmin -uroot -S /tmp/mysql-master.sock shutdown
        echo_message "Shutdown Master Done"
        pidof mysqld > /dev/null && echo "Something error" || echo "Check OK" ;;
    start)
        echo -n "Start MySQL server"
        $MYSQL_BASE/bin/mysqld --defaults-file=/tmp/mysql-master/my.cnf  \
            --basedir=$MYSQL_BASE --datadir=/tmp/mysql-master > /dev/null 2>&1 &
        echo_message "Start Master Done"
        $MYSQL_BASE/bin/mysqld --defaults-file=/tmp/mysql-slave/my.cnf  \
            --basedir=$MYSQL_BASE --datadir=/tmp/mysql-slave  > /dev/null 2>&1 &
        echo_message "Start Slave Done"
        pidof mysqld > /dev/null && echo "Check OK" || echo "Something error"
        ;;
    clean)
        rm -rf /tmp/{mysql-master,mysql-slave}
        ;;
   *)
       echo "Usage: $1 {build|start|stop|clean}"
       echo "$MYSQL_BASE/bin/mysql -p'new-password' -uroot -S/tmp/mysql-master.sock"
       echo "$MYSQL_BASE/bin/mysql -p'new-password' -uroot -S/tmp/mysql-slave.sock"
       exit 1
       ;;
esac
exit 0
