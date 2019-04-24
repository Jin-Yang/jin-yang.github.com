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
    rm -rf /tmp/{mysql-master1,mysql-master2}
    mkdir /tmp/{mysql-master1,mysql-master2}
    echo_message "Done"

    echo -n "1. Init MySQL"
    $MYSQL_BASE/bin/mysqld --initialize-insecure --basedir=$MYSQL_BASE  \
            --datadir=/tmp/mysql-master1 --user=mysql 1>/dev/null 2>&1
    echo_message "Master1 Done"
    $MYSQL_BASE/bin/mysqld --initialize-insecure --basedir=$MYSQL_BASE \
            --datadir=/tmp/mysql-master2 --user=mysql 1>/dev/null 2>&1
    echo_message "Master2 Done"

    echo -n "2. Prepare configuration"
    cat <<- EOF > /tmp/mysql-master1/my.cnf
[mysqld]
binlog_format   = mixed
log_warnings    = 1
log_error       = localhost.err
log-bin         = mysql-bin
basedir         = $MYSQL_BASE
socket          = /tmp/mysql-master1.sock
pid-file        = /tmp/mysql-master1.pid
datadir         = /tmp/mysql-master1
port            = 3307
server-id       = 1
relay_log_index = relay-bin.index
relay_log       = relay-bin
report_host     = 127.1
report_port     = 3307

gtid-mode       = ON
enforce-gtid-consistency = ON
EOF
    echo_message "Master1 Done"
    cat << EOF > /tmp/mysql-master2/my.cnf
[mysqld]
binlog_format   = mixed
log_warnings    = 1
log_error       = localhost.err
log-bin         = mysql-bin
basedir         = $MYSQL_BASE
socket          = /tmp/mysql-master2.sock
pid-file        = /tmp/mysql-master2.pid
datadir         = /tmp/mysql-master2
port            = 3308
server-id       = 2
relay_log_index = relay-bin.index
relay_log       = relay-bin
report_host     = 127.1
report_port     = 3308

gtid-mode       = ON
enforce-gtid-consistency = ON
EOF
    echo_message "Master2 Done"

    echo -n "3. Start MySQL Server"
    $MYSQL_BASE/bin/mysqld --defaults-file=/tmp/mysql-master1/my.cnf  \
        --basedir=$MYSQL_BASE --datadir=/tmp/mysql-master1         \
         --skip-grant-tables > /dev/null 2>&1 &
    echo_message "Master1 Done"
    $MYSQL_BASE/bin/mysqld --defaults-file=/tmp/mysql-master2/my.cnf  \
        --basedir=$MYSQL_BASE --datadir=/tmp/mysql-master2         \
         --skip-grant-tables > /dev/null 2>&1 &
    echo_message "Master2 Done"

    echo -n "4. Reset password"
    sleep 3
    $MYSQL_BASE/bin/mysql -uroot -S/tmp/mysql-master1.sock \
        -e "UPDATE mysql.user SET authentication_string=PASSWORD('new-password') WHERE user='root'"
    echo_message "Master1 Done"
    $MYSQL_BASE/bin/mysql -uroot -S/tmp/mysql-master2.sock  \
        -e "UPDATE mysql.user SET authentication_string=PASSWORD('new-password') WHERE user='root'"
    echo_message "Master2 Done"

    echo -n "5. Restart MySQL server"
    $MYSQL_BASE/bin/mysqladmin -uroot -S /tmp/mysql-master1.sock shutdown
    echo_message "Shutdown Master1 Done"
    sleep 1
    $MYSQL_BASE/bin/mysqld --defaults-file=/tmp/mysql-master1/my.cnf  \
        --basedir=$MYSQL_BASE --datadir=/tmp/mysql-master1 > /dev/null 2>&1 &
    echo_message "Start Master1 Done"
    $MYSQL_BASE/bin/mysqladmin -p'new-password' -uroot -S /tmp/mysql-master1.sock ping \
        >/dev/null 2>&1 && echo_message "ERROR" || echo_message "Check OK"

    $MYSQL_BASE/bin/mysqladmin -uroot -S /tmp/mysql-master2.sock shutdown
    echo_message "Shutdown Master2 Done"
    sleep 1
    $MYSQL_BASE/bin/mysqld --defaults-file=/tmp/mysql-master2/my.cnf  \
        --basedir=$MYSQL_BASE --datadir=/tmp/mysql-master2  > /dev/null 2>&1 &
    echo_message "Start Master2 Done"
    $MYSQL_BASE/bin/mysqladmin -p'new-password' -uroot -S /tmp/mysql-master2.sock ping \
        >/dev/null 2>&1 && echo_message "ERROR" || echo_message "Check OK"


    echo -n "6. Start replication"
    sleep 3
    $MYSQL_BASE/bin/mysql -uroot -S/tmp/mysql-master1.sock -p"new-password" 2>/dev/null \
            -e "GRANT REPLICATION SLAVE ON *.* to 'mysync'@'localhost' IDENTIFIED BY 'kidding'"
    sleep 0.5
    sql="CHANGE MASTER TO master_host='localhost',master_port=3307,master_user='mysync',
         master_password='kidding',MASTER_AUTO_POSITION=1"
    $MYSQL_BASE/bin/mysql -uroot -S/tmp/mysql-master2.sock -p"new-password" 2>/dev/null \
            -e "RESET MASTER; $sql"
    $MYSQL_BASE/bin/mysql -uroot -S/tmp/mysql-master2.sock -p"new-password" 2>/dev/null \
            -e "START SLAVE"
    echo_message "Start Master1=>Master2 Done"

    $MYSQL_BASE/bin/mysql -uroot -S/tmp/mysql-master2.sock -p"new-password" 2>/dev/null \
            -e "GRANT REPLICATION SLAVE ON *.* to 'mysync'@'localhost' IDENTIFIED BY 'kidding'"
    sleep 0.5
    sql="CHANGE MASTER TO master_host='localhost',master_port=3308,master_user='mysync',
         master_password='kidding',MASTER_AUTO_POSITION=1"
    $MYSQL_BASE/bin/mysql -uroot -S/tmp/mysql-master1.sock -p"new-password" 2>/dev/null \
            -e "RESET MASTER; $sql"
    $MYSQL_BASE/bin/mysql -uroot -S/tmp/mysql-master1.sock -p"new-password" 2>/dev/null \
            -e "START SLAVE"
    echo_message "Start Master2=>Master1 Done"
}

case "$1" in
    build)
        mysql_build
        ;;
    stop)
        echo -n "Shutdown MySQL server"
        $MYSQL_BASE/bin/mysqladmin -uroot -pnew-password -S /tmp/mysql-master.sock shutdown >/dev/null 2>&1
        echo_message "Shutdown Master1 Done"
        $MYSQL_BASE/bin/mysqladmin -uroot -pnew-password -S /tmp/mysql-slave.sock shutdown >/dev/null 2>&1
        echo_message "Shutdown Master2 Done"
        pidof mysqld > /dev/null && echo "Something error" || echo "Check OK" ;;
    start)
        echo -n "Start MySQL server"
        $MYSQL_BASE/bin/mysqld --defaults-file=/tmp/mysql-master1/my.cnf  \
            --basedir=$MYSQL_BASE --datadir=/tmp/mysql-master1 > /dev/null 2>&1 &
        echo_message "Start Master1 Done"
        $MYSQL_BASE/bin/mysqld --defaults-file=/tmp/mysql-master2/my.cnf  \
            --basedir=$MYSQL_BASE --datadir=/tmp/mysql-master2  > /dev/null 2>&1 &
        echo_message "Start Master2 Done"
        pidof mysqld > /dev/null && echo "Check OK" || echo "Something error"
        ;;
    clean)
        rm -rf /tmp/{mysql-master1,mysql-master2}
        ;;
   *)
       echo "Usage: $1 {build|start|stop|clean}"
       echo "$MYSQL_BASE/bin/mysql -p'new-password' -uroot -S/tmp/mysql-master1.sock"
       echo "$MYSQL_BASE/bin/mysql -p'new-password' -uroot -S/tmp/mysql-master2.sock"
       exit 1
       ;;
esac
exit 0
