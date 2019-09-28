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
    rm -rf /tmp/{mysql1,mysql2,mysql3}
    mkdir /tmp/{mysql1,mysql2,mysql3}
    echo_message "Done"

    echo -n "1. Init MySQL"
    $MYSQL_BASE/bin/mysqld --initialize-insecure --basedir=$MYSQL_BASE \
        --datadir=/tmp/mysql1 --user=mysql 1>/dev/null 2>&1
    echo_message "MySQL #1 Done"
    $MYSQL_BASE/bin/mysqld --initialize-insecure --basedir=$MYSQL_BASE \
        --datadir=/tmp/mysql2 --user=mysql 1>/dev/null 2>&1
    echo_message "MySQL #2 Done"
    $MYSQL_BASE/bin/mysqld --initialize-insecure --basedir=$MYSQL_BASE \
        --datadir=/tmp/mysql3 --user=mysql 1>/dev/null 2>&1
    echo_message "MySQL #3 Done"

    echo -n "2. Prepare configuration"
    cat <<- EOF > /tmp/mysql1/my.cnf
# only the last two sub-sections are directly related to Group Replication
[mysqld]
server-id      = 1
port           = 3311
datadir        = /tmp/mysql1
socket         = /tmp/mysql1.sock
pid-file       = /tmp/mysql1.pid
log-error      = mysqld.log

# replication and binlog related options
#binlog-row-image          = MINIMAL
#relay-log-recovery        = ON
#sync-relay-log            = 1
#sync-master-info          = 1000
#slave-parallel-workers    = 4
#slave-parallel-type       = LOGICAL_CLOCK
#binlog-rows-query-log-events    = ON
#log-bin-trust-function-creators = ON
#slave-preserve-commit-order     = ON
#slave-rows-search-algorithms    = 'INDEX_SCAN,HASH_SCAN'
#slave-type-conversions          = ALL_NON_LOSSY

# group replication pre-requisites && recommendations
log-bin                   = mysql-bin
binlog-format             = ROW
log-slave-updates         = ON
gtid-mode                 = ON
enforce-gtid-consistency  = ON
master-info-repository    = TABLE
relay-log-info-repository = TABLE
binlog-checksum           = NONE
transaction-isolation     = 'READ-COMMITTED'

# prevent use of non-transactional storage engines
disabled_storage_engines  = "MyISAM,BLACKHOLE,FEDERATED,ARCHIVE"

# group replication specific options
plugin-load               = group_replication.so
transaction-write-set-extraction  = XXHASH64
group_replication_group_name      = 550fa9ee-a1f8-4b6d-9bfe-c03c12cd1c72
group_replication_start_on_boot   = OFF
group_replication_local_address   = '127.0.0.1:3501'
group_replication_group_seeds     = '127.0.0.1:3501,127.0.0.1:3502,127.0.0.1:3503'
group_replication_bootstrap_group = OFF
group_replication_single_primary_mode = FALSE
group_replication_enforce_update_everywhere_checks = TRUE
#group_replication   = FORCE_PLUS_PERMANENT
EOF
    echo_message "MySQL #1 Done"

    cat <<- EOF > /tmp/mysql2/my.cnf
# only the last two sub-sections are directly related to Group Replication
[mysqld]
server-id      = 2
port           = 3312
datadir        = /tmp/mysql2
socket         = /tmp/mysql2.sock
pid-file       = /tmp/mysql2.pid
log-error      = mysqld.log

# replication and binlog related options
#binlog-row-image          = MINIMAL
#relay-log-recovery        = ON
#sync-relay-log            = 1
#sync-master-info          = 1000
#slave-parallel-workers    = 4
#slave-parallel-type       = LOGICAL_CLOCK
#binlog-rows-query-log-events    = ON
#log-bin-trust-function-creators = ON
#slave-preserve-commit-order     = ON
#slave-rows-search-algorithms    = 'INDEX_SCAN,HASH_SCAN'
#slave-type-conversions          = ALL_NON_LOSSY

# group replication pre-requisites && recommendations
log-bin                   = mysql-bin
binlog-format             = ROW
log-slave-updates         = ON
gtid-mode                 = ON
enforce-gtid-consistency  = ON
master-info-repository    = TABLE
relay-log-info-repository = TABLE
binlog-checksum           = NONE
transaction-isolation     = 'READ-COMMITTED'

# prevent use of non-transactional storage engines
disabled_storage_engines  = "MyISAM,BLACKHOLE,FEDERATED,ARCHIVE"

# group replication specific options
plugin-load               = group_replication.so
transaction-write-set-extraction  = XXHASH64
group_replication_group_name      = 550fa9ee-a1f8-4b6d-9bfe-c03c12cd1c72
group_replication_start_on_boot   = OFF
group_replication_local_address   = '127.0.0.1:3502'
group_replication_group_seeds     = '127.0.0.1:3501,127.0.0.1:3502,127.0.0.1:3503'
group_replication_bootstrap_group = OFF
group_replication_single_primary_mode = FALSE
group_replication_enforce_update_everywhere_checks = TRUE
#group_replication   = FORCE_PLUS_PERMANENT
EOF
    echo_message "MySQL #2 Done"

    cat <<- EOF > /tmp/mysql3/my.cnf
# only the last two sub-sections are directly related to Group Replication
[mysqld]
server-id      = 3
port           = 3313
datadir        = /tmp/mysql3
socket         = /tmp/mysql3.sock
pid-file       = /tmp/mysql3.pid
log-error      = mysqld.log

# replication and binlog related options
#binlog-row-image          = MINIMAL
#relay-log-recovery        = ON
#sync-relay-log            = 1
#sync-master-info          = 1000
#slave-parallel-workers    = 4
#slave-parallel-type       = LOGICAL_CLOCK
#binlog-rows-query-log-events    = ON
#log-bin-trust-function-creators = ON
#slave-preserve-commit-order     = ON
#slave-rows-search-algorithms    = 'INDEX_SCAN,HASH_SCAN'
#slave-type-conversions          = ALL_NON_LOSSY

# group replication pre-requisites && recommendations
log-bin                   = mysql-bin
binlog-format             = ROW
log-slave-updates         = ON
gtid-mode                 = ON
enforce-gtid-consistency  = ON
master-info-repository    = TABLE
relay-log-info-repository = TABLE
binlog-checksum           = NONE
transaction-isolation     = 'READ-COMMITTED'

# prevent use of non-transactional storage engines
disabled_storage_engines  = "MyISAM,BLACKHOLE,FEDERATED,ARCHIVE"

# group replication specific options
plugin-load               = group_replication.so
transaction-write-set-extraction  = XXHASH64
group_replication_group_name      = 550fa9ee-a1f8-4b6d-9bfe-c03c12cd1c72
group_replication_start_on_boot   = OFF
group_replication_local_address   = '127.0.0.1:3503'
group_replication_group_seeds     = '127.0.0.1:3501,127.0.0.1:3502,127.0.0.1:3503'
group_replication_bootstrap_group = OFF
group_replication_single_primary_mode = FALSE
group_replication_enforce_update_everywhere_checks = TRUE
#group_replication   = FORCE_PLUS_PERMANENT
EOF
    echo_message "MySQL #3 Done"

    echo -n "3. Start MySQL Server"
    $MYSQL_BASE/bin/mysqld --defaults-file=/tmp/mysql1/my.cnf \
        --basedir=$MYSQL_BASE --datadir=/tmp/mysql1           \
        --skip-grant-tables > /dev/null 2>&1 &
    echo_message "MySQL #1 Done"
    $MYSQL_BASE/bin/mysqld --defaults-file=/tmp/mysql2/my.cnf \
        --basedir=$MYSQL_BASE --datadir=/tmp/mysql2           \
        --skip-grant-tables > /dev/null 2>&1 &
    echo_message "MySQL #2 Done"
    $MYSQL_BASE/bin/mysqld --defaults-file=/tmp/mysql3/my.cnf \
        --basedir=$MYSQL_BASE --datadir=/tmp/mysql3           \
        --skip-grant-tables > /dev/null 2>&1 &
    echo_message "MySQL #3 Done"

    echo -n "4. Reset password"
    sleep 3
    $MYSQL_BASE/bin/mysql -uroot -S/tmp/mysql1.sock \
        -e "SET SQL_LOG_BIN=0;
            UPDATE mysql.user SET authentication_string=PASSWORD('new-password') WHERE user='root';
            SET SQL_LOG_BIN=1"
    echo_message "MySQL #1 Done"
    $MYSQL_BASE/bin/mysql -uroot -S/tmp/mysql2.sock \
        -e "SET SQL_LOG_BIN=0;
            UPDATE mysql.user SET authentication_string=PASSWORD('new-password') WHERE user='root';
            SET SQL_LOG_BIN=1"
    echo_message "MySQL #2 Done"
    $MYSQL_BASE/bin/mysql -uroot -S/tmp/mysql3.sock \
        -e "SET SQL_LOG_BIN=0;
            UPDATE mysql.user SET authentication_string=PASSWORD('new-password') WHERE user='root';
            SET SQL_LOG_BIN=1"
    echo_message "MySQL #3 Done"

    echo -n "5. Restart MySQL server"
    $MYSQL_BASE/bin/mysqladmin -uroot -S /tmp/mysql1.sock shutdown
    echo_message "Shutdown MySQL #1 Done"
    sleep 1
    $MYSQL_BASE/bin/mysqld --defaults-file=/tmp/mysql1/my.cnf \
        --basedir=$MYSQL_BASE --datadir=/tmp/mysql1 > /dev/null 2>&1 &
    echo_message "Start MySQL #1 Done"
    $MYSQL_BASE/bin/mysqladmin -p'new-password' -uroot -S /tmp/mysql1.sock ping \
        >/dev/null 2>&1 && echo_message "ERROR" || echo_message "Check OK"

    $MYSQL_BASE/bin/mysqladmin -uroot -S /tmp/mysql2.sock shutdown
    echo_message "Shutdown MySQL #2 Done"
    sleep 1
    $MYSQL_BASE/bin/mysqld --defaults-file=/tmp/mysql2/my.cnf \
        --basedir=$MYSQL_BASE --datadir=/tmp/mysql2 > /dev/null 2>&1 &
    echo_message "Start MySQL #2 Done"
    $MYSQL_BASE/bin/mysqladmin -p'new-password' -uroot -S /tmp/mysql2.sock ping \
        >/dev/null 2>&1 && echo_message "ERROR" || echo_message "Check OK"

    $MYSQL_BASE/bin/mysqladmin -uroot -S /tmp/mysql3.sock shutdown
    echo_message "Shutdown MySQL #3 Done"
    sleep 1
    $MYSQL_BASE/bin/mysqld --defaults-file=/tmp/mysql3/my.cnf \
        --basedir=$MYSQL_BASE --datadir=/tmp/mysql3 > /dev/null 2>&1 &
    echo_message "Start MySQL #3 Done"
    $MYSQL_BASE/bin/mysqladmin -p'new-password' -uroot -S /tmp/mysql3.sock ping \
        >/dev/null 2>&1 && echo_message "ERROR" || echo_message "Check OK"

    echo -n "6. Start replication"
    sleep 3
    $MYSQL_BASE/bin/mysql -uroot -S/tmp/mysql1.sock -p"new-password" 2>/dev/null \
        -e "SET SQL_LOG_BIN=0;
            GRANT REPLICATION SLAVE ON *.* to 'mysync'@'localhost' IDENTIFIED BY 'kidding';
            FLUSH PRIVILEGES;
            SET SQL_LOG_BIN=1;"
    sleep 0.5
    sql="CHANGE MASTER TO master_user='mysync', master_password='kidding'
         FOR CHANNEL 'group_replication_recovery'"
    $MYSQL_BASE/bin/mysql -uroot -S/tmp/mysql1.sock -p"new-password" 2>/dev/null \
        -e "$sql"
    sleep 0.5
    sql="SET GLOBAL group_replication_bootstrap_group=ON;
         START GROUP_REPLICATION;
         SET GLOBAL group_replication_bootstrap_group=OFF;"
    $MYSQL_BASE/bin/mysql -uroot -S/tmp/mysql1.sock -p"new-password" 2>/dev/null \
        -e "$sql"
    echo_message "MySQL #1 Done(Bootstrap)"

    $MYSQL_BASE/bin/mysql -uroot -S/tmp/mysql2.sock -p"new-password" 2>/dev/null \
        -e "SET SQL_LOG_BIN=0;
            GRANT REPLICATION SLAVE ON *.* to 'mysync'@'localhost' IDENTIFIED BY 'kidding';
            FLUSH PRIVILEGES;
            SET SQL_LOG_BIN=1;"
    sleep 0.5
    sql="CHANGE MASTER TO master_user='mysync', master_password='kidding'
         FOR CHANNEL 'group_replication_recovery'"
    $MYSQL_BASE/bin/mysql -uroot -S/tmp/mysql2.sock -p"new-password" 2>/dev/null \
        -e "$sql"
    sleep 0.5
    $MYSQL_BASE/bin/mysql -uroot -S/tmp/mysql2.sock -p"new-password" 2>/dev/null \
        -e "START GROUP_REPLICATION"
    echo_message "MySQL #2 Done"

    $MYSQL_BASE/bin/mysql -uroot -S/tmp/mysql3.sock -p"new-password" 2>/dev/null \
        -e "SET SQL_LOG_BIN=0;
            GRANT REPLICATION SLAVE ON *.* to 'mysync'@'localhost' IDENTIFIED BY 'kidding';
            FLUSH PRIVILEGES;
            SET SQL_LOG_BIN=1;"
    sleep 0.5
    sql="CHANGE MASTER TO master_user='mysync', master_password='kidding'
         FOR CHANNEL 'group_replication_recovery'"
    $MYSQL_BASE/bin/mysql -uroot -S/tmp/mysql3.sock -p"new-password" 2>/dev/null \
        -e "$sql"
    sleep 0.5
    $MYSQL_BASE/bin/mysql -uroot -S/tmp/mysql3.sock -p"new-password" 2>/dev/null \
        -e "START GROUP_REPLICATION"
    echo_message "MySQL #3 Done"
}

case "$1" in
    build)
        mysql_build
        ;;
    stop)
        echo -n "Shutdown MySQL server"
        $MYSQL_BASE/bin/mysqladmin -p'new-password' -uroot -S /tmp/mysql1.sock shutdown 2>/dev/null
        echo_message "Shutdown MySQL #1 Done"
        $MYSQL_BASE/bin/mysqladmin -p'new-password' -uroot -S /tmp/mysql2.sock shutdown 2>/dev/null
        echo_message "Shutdown MySQL #2 Done"
        $MYSQL_BASE/bin/mysqladmin -p'new-password' -uroot -S /tmp/mysql3.sock shutdown 2>/dev/null
        echo_message "Shutdown MySQL #3 Done"
        pidof mysqld > /dev/null && echo "Something error" || echo "Check OK"
        ;;
    start)
        echo -n "Start MySQL server"
        $MYSQL_BASE/bin/mysqld --defaults-file=/tmp/mysql1/my.cnf  \
        --basedir=$MYSQL_BASE --datadir=/tmp/mysql1> /dev/null 2>&1 &
        echo_message "Start MySQL #1 Done"
        $MYSQL_BASE/bin/mysqld --defaults-file=/tmp/mysql2/my.cnf  \
        --basedir=$MYSQL_BASE --datadir=/tmp/mysql2> /dev/null 2>&1 &
        echo_message "Start MySQL #2 Done"
        $MYSQL_BASE/bin/mysqld --defaults-file=/tmp/mysql3/my.cnf  \
        --basedir=$MYSQL_BASE --datadir=/tmp/mysql3> /dev/null 2>&1 &
        echo_message "Start MySQL #3 Done"
        pidof mysqld > /dev/null && echo "Check OK" || echo "Something error"
        ;;
    clean)
        rm -rf /tmp/{mysql1,mysql2,mysql3}
        ;;
    *)
        echo "Usage: $1 {build|start|stop|clean}"
        echo "$MYSQL_BASE/bin/mysql -p'new-password' -P3311 -uroot -S/tmp/mysql1.sock"
        echo "$MYSQL_BASE/bin/mysql -p'new-password' -P3312 -uroot -S/tmp/mysql2.sock"
        echo "$MYSQL_BASE/bin/mysql -p'new-password' -P3313 -uroot -S/tmp/mysql3.sock"
        exit 1
        ;;
esac
exit 0
