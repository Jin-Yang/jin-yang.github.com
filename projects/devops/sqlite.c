#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sqlite3.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define SQL_CREATE_TBL_DAEMON                                     \
        "CREATE TABLE IF NOT EXISTS foobar("                      \
                "name CHAR(64) PRIMARY KEY NOT NULL, "            \
                "flag INT NOT NULL, "                             \
                "version CHAR(64) NOT NULL, "                     \
                "gmt_modify NOT NULL DEFAULT CURRENT_TIMESTAMP, " \
                "gmt_create NOT NULL DEFAULT CURRENT_TIMESTAMP"   \
        ");"

#define SQL_INSERT_STRING                                         \
        "INSERT INTO foobar(name, flag, version) VALUES('%s', %d, '%s')"

/* NOTE: This function maybe block, because of /dev/random */
static int get_random(unsigned char *buff, int len)
{
        int rc, fd, i, idx;
        unsigned char *bcd;
        static unsigned char BCD2ASC[16] = {
                '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                'a', 'b', 'c', 'd', 'e', 'f'
        };

        fd = open("/dev/urandom", O_RDONLY);
        if (fd < 0) {
                fprintf(stderr, "Open(/dev/random) failed, rc %d.", fd);
                return -1;
        }
        len >>= 1;
        bcd = buff + len;

        rc = read(fd, bcd, len);
        if (rc < 0 || rc != len) {
                fprintf(stderr, "Read(/dev/random) failed, rc %d len %d.", rc, len);
                close(fd);
                return -1;
        }

        for(i = 0; i < len; i++) {
                idx = *bcd >> 4;
                *buff++ = BCD2ASC[idx];

                idx = *bcd & 0x0f;
                *buff++ = BCD2ASC[idx];
                bcd++;
        }

        close(fd);

        return 0;
}
int prepare_run(sqlite3 *db, const char *sql, int verbose)
{
        int rc, nrow = 0;
        sqlite3_stmt *stmt;

        if (verbose)
                printf("=====> SQL %s.\n", sql);

        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
                fprintf(stderr, "prepare failed, rc %d, %s.\n", rc, sqlite3_errmsg(db));
                return -1;
        }

        while (1) {
                rc = sqlite3_step(stmt);
                if (rc != SQLITE_ROW) {
                        fprintf(stderr, "Step statements failed, rc %s.\n",
                                        sqlite3_errstr(rc));
                        sqlite3_finalize(stmt);
                        return -1;
                }
                nrow++;
        }

        if (rc != SQLITE_DONE)
                fprintf(stderr, "Unexpect(%d) got rc %d for step().\n", SQLITE_DONE, rc);
        rc = sqlite3_finalize(stmt);
        if (rc != SQLITE_OK)
                fprintf(stderr, "Unexpect(%d) got rc %d for finalize().\n", SQLITE_OK, rc);

        return 0;
}

int main(void)
{
        int rc, i;
        sqlite3 *db;
        char *errmsg, bsql[1024];
        unsigned char *buff;
        int verbose = 1;

        rc = sqlite3_open("daemon.db", &db);
        if (rc != SQLITE_OK) {
                fprintf(stderr, "Cannot open database, rc %d, %s.\n", rc, sqlite3_errmsg(db));
                return -1;
        }

        rc = sqlite3_exec(db, SQL_CREATE_TBL_DAEMON, NULL, NULL, &errmsg);
        if (rc != SQLITE_OK ) {
                fprintf(stderr, "Create table SQL error, rc %d, %s\n", rc, errmsg);
                sqlite3_free(errmsg);
                sqlite3_close(db);
                return -1;
        }
        if (verbose)
                printf("Create table.\n");
        rc = sqlite3_exec(db, "DELETE FROM foobar", NULL, NULL, &errmsg);
        if (rc != SQLITE_OK ) {
                fprintf(stderr, "Delete SQL error, rc %d, %s\n", rc, errmsg);
                sqlite3_free(errmsg);
                sqlite3_close(db);
                return -1;
        }
        if (verbose)
                printf("Delete table.\n");

        buff = (unsigned char *)bsql + sizeof(bsql) - 61;
        buff[60] = 0;
        for (i = 0; i < 10000000; i++) {
                if (get_random(buff, 60) < 0) {
                        rc = -1;
                        goto failed;
                }

                rc = snprintf(bsql, sizeof(bsql), SQL_INSERT_STRING, buff, i, "1.2.3");
                if (rc < 0 || rc > (int)sizeof(bsql))
                        goto failed;

                rc = sqlite3_exec(db, bsql, NULL, NULL, &errmsg);
                if (rc != SQLITE_OK ) {
                        fprintf(stderr, "Insert SQL(%s) error, rc %d, %s\n",
                                        bsql, rc, errmsg);
                        sqlite3_free(errmsg);
                        rc = -1;
                        goto failed;
                }
/*
                if (prepare_run(db, bsql, verbose) < 0) {
                        rc = -1;
                        goto failed;
                }
*/
                usleep(10000);
        }

failed:
        rc = sqlite3_close(db);
        if (rc != SQLITE_OK)
                fprintf(stderr, "Unexpect(%d) got rc %d for close().\n", SQLITE_OK, rc);

        return rc;
}


#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <sqlite3.h>
#include <sys/time.h>

#define CREATE_SQL                                 \
        "CREATE TABLE IF NOT EXISTS foobar ("      \
                "id INTEGER NOT NULL PRIMARY KEY," \
                "count INTEGER NOT NULL"           \
        ");"
#define REPLACE_SQL "REPLACE INTO foobar(id, count) VALUES(1, 0)"
#define UPDATE_SQL  "UPDATE foobar SET count = 2 WHERE id = 1"
#define SELECT_SQL  "SELECT * FROM foobar"

#define RETRY_TIMES 1000
#define SLEEP_TIMES 10 * 1000
#define TIMER_COUNT 10000

static void *select_function(void *param)
{
        int rc = -1;
        char *errmsg;
        pthread_t tid;
        sqlite3 *db = (sqlite3 *)param;
        long count = 0;

        struct timeval begin, curr, diff;
        double consume;

        tid = pthread_self();
        fprintf(stdout, "[INFO] Starting thread 0x%lx\n", tid);
        if (gettimeofday(&begin, NULL) < 0) {
                fprintf(stderr, "[ERROR] Get time failed, %s\n", strerror(errno));
                return NULL;
        }

        while(1) {
                rc = sqlite3_exec(db ,SELECT_SQL, NULL, NULL, &errmsg);
                if (rc != SQLITE_OK) {
                        fprintf(stdout, "[INFO] 0x%lx Select failed, %d %s\n", tid, rc, errmsg);
                }
                if (++count >= TIMER_COUNT) {
                        if (gettimeofday(&curr, NULL) < 0) {
                                fprintf(stderr, "[ERROR] Get time failed, %s\n", strerror(errno));
                                return NULL;
                        }
                        timersub(&curr, &begin, &diff);
                        consume = diff.tv_sec + diff.tv_usec / 1e6;
                        memcpy(&begin, &curr, sizeof(struct timeval));
                        count = 0;

                        fprintf(stdout, "[INFO] Select %d %.2fs QPS: %.2f\n", TIMER_COUNT, consume, TIMER_COUNT/consume);
                }
        }
}

static void *update_function(void *param)
{
        int rc = -1;
        sqlite3 *db = (sqlite3 *)param;
        char *errmsg;
        pthread_t tid;
        long count = 0;

        struct timeval begin, curr, diff;
        double consume;

        tid = pthread_self();
        fprintf(stdout, "[INFO] Starting thread 0x%lx\n", tid);
        if (gettimeofday(&begin, NULL) < 0) {
                fprintf(stderr, "[ERROR] Get time failed, %s\n", strerror(errno));
                return NULL;
        }

        while(1) {
                rc = sqlite3_exec(db, UPDATE_SQL, NULL, NULL, &errmsg);
                if (rc != SQLITE_OK) {
                        fprintf(stdout, "[INFO] 0x%lx Update failed, %d %s\n", tid, rc, errmsg);
                }
                //fprintf(stdout, "[INFO] Update\n");
                if (++count >= TIMER_COUNT) {
                        if (gettimeofday(&curr, NULL) < 0) {
                                fprintf(stderr, "[ERROR] Get time failed, %s\n", strerror(errno));
                                return NULL;
                        }
                        timersub(&curr, &begin, &diff);
                        consume = diff.tv_sec + diff.tv_usec / 1e6;
                        memcpy(&begin, &curr, sizeof(struct timeval));
                        count = 0;

                        fprintf(stdout, "[INFO] 0x%lx Update %d %.2fs TPS: %.2f\n",
                                        tid, TIMER_COUNT, consume, TIMER_COUNT/consume);
                }
        }
}

int main()
{
        pthread_t tpid;

        sqlite3 *db = NULL;
        int rc;
        char *errmsg;
        int i = 0;

        fprintf(stdout, "[INFO] SQLite %ssupport threadsafe\n", sqlite3_threadsafe() ? "" : "does't ");

        rc = sqlite3_open("foobar.db", &db);
        if (rc != SQLITE_OK) {
                fprintf(stderr, "[ERROR] Cannot open database, %s\n", sqlite3_errmsg(db));
                return -1;
        }

        rc = sqlite3_exec(db ,CREATE_SQL, NULL, NULL, &errmsg);
        if (rc != SQLITE_OK) {
                fprintf(stderr, "[ERROR] Create db failed, %s\n", errmsg);
                sqlite3_close(db);
                return -1;
        }

        rc = sqlite3_exec(db, REPLACE_SQL, 0, 0, &errmsg);
        if (rc != SQLITE_OK)
                fprintf(stderr, "[ERROR] Replace db failed, %s\n", errmsg);

        rc = sqlite3_exec(db, "PRAGMA synchronous = 2", 0, 0, &errmsg);
        if (rc != SQLITE_OK)
                fprintf(stderr, "[ERROR] PRAGMA synchronous failed, %s\n", errmsg);

        rc = sqlite3_exec(db, "PRAGMA journal_mode=OFF", 0, 0, &errmsg);
        if (rc != SQLITE_OK)
                fprintf(stderr, "[ERROR] PRAGMA journal_mode failed, %s\n", errmsg);

        for(i = 0; i < 10; i++){
                pthread_create(&tpid, 0, select_function, db);
                pthread_detach(tpid);
        }

        i = 0;
        for(; i < 10; i++){
                pthread_create(&tpid, 0, update_function, db);
                pthread_detach(tpid);
        }

        sleep(10000);

        sqlite3_close(db);

        return 0;
}
