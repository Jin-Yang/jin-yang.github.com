---
title: Linux 时间函数
layout: post
comments: true
language: chinese
category: [linux,program]
keywords: linux,program
description: 简单介绍下 Linux 中与时间相关的函数。
---

简单介绍下 Linux 中与时间相关的函数。


<!-- more -->

## 简介

现在通用的标准时间为 `Coordinated Universal Time, UTC`，由世界上最精确的原子钟提供计时，而本地时间是 `UTC+TimeZone`，也就是日常使用的时间。

另外，说到 UTC 就不得不提格林威治平均时 `Greenwich Mean Time, GMT`，这是时区 0 的本地时间，也即是 `GMT=UTC+0`，所以 GMT 和 UTC 时间值是相等的。

在 `*nix` 系统中，还有一个词 `Epoch`，它指的是一个特定时间 `1970-01-01 00:00:00 +0000 (UTC)`。

## 获取时间函数

简单介绍下与获取时间相关的系统调用。

### time() 秒级

{% highlight c %}
#include <time.h>

//----- time_t一般为long int，不过不同平台可能不同，打印可通过printf("%ju", (uintmax_t)ret)打印
time_t time(time_t *tloc);

char *ctime(const time_t *timep);                // 错误返回NULL
char *ctime_r(const time_t *timep, char *buf);   // buf至少26bytes，返回与buf相同
{% endhighlight %}

如果 tloc 不为 `NULL`，那么数据同时会保存在 tloc 中，成功返回从 epoch 开始的时间，单位为秒；否则返回 `-1` 。

### ftime() 毫秒级

{% highlight c %}
#include <sys/timeb.h>
struct timeb {
	time_t         time;      // 为1970-01-01至今的秒数
	unsigned short millitm;   // 毫秒
	short          timezonel; // 为目前时区和Greenwich相差的时间，单位为分钟，东区为负
	short          dstflag;   // 非0代表启用夏时制
};
int ftime(struct timeb *tp);
{% endhighlight %}

总是返回 0 。


### gettimeofday() 微秒级

`gettimeofday()` 函数可以获得当前系统的绝对时间。

{% highlight c %}
struct timeval {
	time_t      tv_sec;
	suseconds_t tv_usec;
};

int gettimeofday(struct timeval *tv, struct timezone *tz);
{% endhighlight %}

可以通过如下函数测试。

{% highlight c %}
#include <stdio.h>
#include <sys/time.h>

int main(void)
{
        int i = 30000000;
        struct timeval begin, end, diff;

        gettimeofday(&begin, NULL);
        while (--i)
                ;
        gettimeofday(&end, NULL);
        timersub(&end, &begin, &diff);

        printf("time: %ld.%06ld\n", diff.tv_sec, diff.tv_usec);
        printf("time: %.6f\n", diff.tv_sec + diff.tv_usec / 1e6);

        return 0;
}
{% endhighlight %}

### clock_gettime() 纳秒级

编译连接时需要加上 ```-lrt```，不过不加也可以编译，应该是非实时的。```struct timespect *tp``` 用来存储当前的时间，其结构和函数声明如下，返回 0 表示成功，-1 表示失败。

{% highlight c %}
struct timespec {
    time_t tv_sec;    /* seconds */
    long tv_nsec;     /* nanoseconds */
};
int clock_gettime(clockid_t clk_id, struct timespect *tp);
{% endhighlight %}

clk_id 用于指定计时时钟的类型，简单列举如下几种，详见 ```man 2 clock_gettime``` 。

* CLOCK_REALTIME/CLOCK_REALTIME_COARSE<br>
系统实时时间 (wall-clock time)，即从 `UTC 1970.01.01 00:00:00` 开始计时的秒数，随系统实时时间改变而改变，包括通过系统函数手动调整系统时间，例如 `settime()`、`settimeofday()`，或者通过 `adjtime()`、`adjtimex()` 或者 NTP 调整时间。

* CLOCK_MONOTONIC/CLOCK_MONOTONIC_COARSE<br>
从系统启动开始计时，不受系统时间被用户改变的影响，但会受像 ```adjtime()``` 或者 NTP 之类渐进调整的影响。

* CLOCK_MONOTONIC_RAW<br>
与上述的 ```CLOCK_MONOTONIC``` 相同，只是不会受 ```adjtime()``` 以及 NTP 的影响。

* CLOCK_PROCESS_CPUTIME_ID<br>
本进程到当前代码系统 CPU 花费的时间。

* CLOCK_THREAD_CPUTIME_ID<br>
本线程到当前代码系统 CPU 花费的时间。

可以通过如下程序测试。

{% highlight c %}
#include <time.h>
#include <stdio.h>

struct timespec diff(struct timespec start, struct timespec end)
{
	struct timespec temp;

	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

int main(void)
{
	int temp, i;
	struct timespec t, begin, end;

	clock_gettime(CLOCK_REALTIME, &t);
	printf("CLOCK_REALTIME: %d, %d\n", t.tv_sec, t.tv_nsec);
	clock_gettime(CLOCK_MONOTONIC, &t);
	printf("CLOCK_MONOTONIC: %d, %d\n", t.tv_sec, t.tv_nsec);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t);
	printf("CLOCK_THREAD_CPUTIME_ID: %d, %d\n", t.tv_sec, t.tv_nsec);

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &begin);
	printf("CLOCK_PROCESS_CPUTIME_ID: %d, %d\n", begin.tv_sec, begin.tv_nsec);
	for (i = 0; i < 242000000; i++)
	temp+=temp;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
	t = diff(begin, end);
	printf("diff CLOCK_PROCESS_CPUTIME_ID: %d, %d\n", t.tv_sec, t.tv_nsec);

	return 0;
}
{% endhighlight %}


## 时间转换相关


函数 `mktime()` 将 `timeptr` 所指向的结构转换为一个依据本地时区的 `time_t` 值，也就是自 `1970.01.01` 以来逝去时间的秒数，当发生错误时返回 `-1` 。

注意，同时会更新其入参，包括了 `tm_wday` `tm_isdst` 等参数。其中比较关键的是夏令时的设置，其入参的影响如下：

* `0` 完全不考虑夏令时的问题，直接将日历时间转换为秒数；
* `1` 考虑夏令时。
* `-1` 自动根据时区信息进行判断。

建议使用 `-1` 如下是测试。

{% highlight c %}
struct tm {
	int tm_sec;         /* seconds [0, 59] */
	int tm_min;         /* minutes [0, 59] */
	int tm_hour;        /* hours [0, 23] */
	int tm_mday;        /* day of the month [1, 31] */
	int tm_mon;         /* month [0, 11] */
	int tm_year;        /* year now.year - 1900 */
	int tm_wday;        /* day of the week, [0, 6] 0->sunday */
	int tm_yday;        /* day in the year [0, 365] */
	int tm_isdst;       /* daylight saving time */
};

time_t mktime(struct tm *timeptr);
{% endhighlight %}

如下是简单的测试用例，首先参考 [Linux 时间基本概念](/post/linux-basic-conception-for-time-details.html#DST) 中 DST 的介绍，将时区设置为 CET 时区。

{% highlight c %}
#define _XOPEN_SOURCE
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#define log_info(...)  do { printf(" info: " __VA_ARGS__); putchar('\n'); } while(0);
static void dump_tm(const struct tm *t, const char *var)
{
        log_info("---> dump <struct tm> %s", var);
        log_info("    %04d %02d %02d %02d:%02d:%02d",
                t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                t->tm_hour, t->tm_min, t->tm_sec);
        log_info("     tm_wday: %02d", t->tm_wday);
        log_info("     tm_yday: %02d", t->tm_yday);
        log_info("    tm_isdst: %02d", t->tm_isdst);
}

int main(void)
{
        time_t dstt;
        struct tm dst_tm;

        strptime("2016-03-27 01:59:58", "%Y-%m-%d %H:%M:%S", &dst_tm);
        dst_tm.tm_isdst = -1;
        dstt = mktime(&dst_tm);
        log_info("isdst == -1, result %ld", dstt);
        dump_tm(&dst_tm, "local");

        strptime("2016-03-27 02:00:01", "%Y-%m-%d %H:%M:%S", &dst_tm);
        dst_tm.tm_isdst = -1;
        dstt = mktime(&dst_tm);
        log_info("isdst == -1, result %ld", dstt);
        dump_tm(&dst_tm, "local");

        strptime("2016-03-27 03:00:00", "%Y-%m-%d %H:%M:%S", &dst_tm);
        dst_tm.tm_isdst = -1;
        dstt = mktime(&dst_tm);
        log_info("isdst == -1, result %ld", dstt);
        dump_tm(&dst_tm, "local");

	return 0;
}
{% endhighlight %}

可以看到，对于 `2016-03-27 02:00:01` 这种的非法值，`mktime()` 会将其修改为合法的时间，而返回的时间戳是没有发生跳变的。

注意，使用 `strptime()` 前，需要将 `struct tm` 结构体清空，否则不需要设置的字段 (例如 `tm_isdst` ) 会有脏数据。

### 时间转换

`mktime()` `localtime()` 会根据时间戳或者年月日来更新星期情况，也就是当天是星期几。

所以，如果要通过当前日期获取到下个的星期 N 的信息，那么可以直接通过星期计算偏移，然后添加到日期中，即使超过了当月天数的限制，`mktime()` 也能够正确计算。

可以通过 `man mktime` 查看帮助文档，也就是说 `mktime()` 会忽略 `tm_wday` 以及 `tm_yday` 字段，会使用 `tm_isdst` 判断是否采用夏令时，同时会根据其它字段来修改 `tm_wday` `tm_yday` 字段，同时其它字段如果超过了范围则会修正。

{% highlight c %}
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define log_info(...)  do { printf(" info: " __VA_ARGS__); putchar('\n'); } while(0);
static void dump_tm(const struct tm *t, const char *var)
{
        log_info("---> dump <struct tm> %s", var);
        log_info("    %04d %02d %02d %02d:%02d:%02d",
                t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                t->tm_hour, t->tm_min, t->tm_sec);
        log_info("     tm_wday: %02d", t->tm_wday);
        log_info("     tm_yday: %02d", t->tm_yday);
        log_info("    tm_isdst: %02d", t->tm_isdst);
}

int main(void)
{
        /*
         * 2017-11-29 16:34:56 Wed 1511944496
         * 2017-12-02 16:34:56 Tus 1512203696
         */
        time_t ot, tt;
        struct tm otm, ttm;

        ot = 1511944496;
        localtime_r(&ot, &ttm); /* it's Wednesday */
        ttm.tm_wday = 1; /* change to Monday */
        tt = mktime(&ttm); /* this will change to Wednesday */
        dump_tm(&ttm, "correct");

        ttm.tm_mday += 6 - ttm.tm_wday;
        tt = mktime(&ttm);
        dump_tm(&ttm, "correct");

        /*
         * 闰年
         * 2014-02-27 16:34:56 Thu 1393490096
         * 2014-03-04 16:34:56 Tus 1393922096
         */
        ot = 1393490096;
        localtime_r(&ot, &ttm);
        if (ttm.tm_wday > 2)
                ttm.tm_mday += ((6 - ttm.tm_wday) + 2 + 1); /* add Sunday */
        else
                ttm.tm_mday += 2 - ttm.tm_wday;
        tt = mktime(&ttm);
        dump_tm(&ttm, "correct");

        /*
         * 闰秒
         * 2015-06-30 23:30:00 Thu 1435678200
         * 2015-07-01 23:30:00 Thu 1435764600
         */

        /*
         * 夏令时开始
         * 2016-03-27 01:58:00 Thu 1435678200
         * 2016-03-27 03:58:00 Thu 1435678200
         */
        char *tz;
        tz = getenv("TZ");
        setenv("TZ", "CET", 1);
        tzset();

        strptime("2016-03-27 01:58:00", "%Y-%m-%d %H:%M:%S", &otm);
        otm.tm_isdst = -1;
        ot = mktime(&otm);
        log_info("isdst == -1, result %ld", ot);
        dump_tm(&otm, "local");

        strptime("2016-03-27 03:58:00", "%Y-%m-%d %H:%M:%S", &ttm);
        ttm.tm_isdst = -1;
        tt = mktime(&ttm);
        log_info("isdst == -1, result %ld", tt);
        dump_tm(&ttm, "local");

        log_info("timestamp diff %ld", tt - ot);

        /*
         * 夏令时结束
         * 2016-10-30 02:58:00 Thu 1435678200
         * 2016-10-30 02:58:00 Thu 1435678200
         */
        strptime("2016-10-30 02:58:00", "%Y-%m-%d %H:%M:%S", &otm);
        otm.tm_isdst = -1;
        ot = mktime(&otm);
        log_info("isdst == -1, result %ld", ot);
        dump_tm(&otm, "local");

        strptime("2016-10-30 03:58:00", "%Y-%m-%d %H:%M:%S", &ttm);
        ttm.tm_isdst = -1;
        tt = mktime(&ttm);
        log_info("isdst == -1, result %ld", tt);
        dump_tm(&ttm, "local");

        log_info("timestamp diff %ld", tt - ot);

        return 0;
}
{% endhighlight %}

实际上程序比较怕时间回退，那么关于夏令时比较坑的是，夏令时的停止，此时时钟会向后回拨一次，也就是说，同一个小时的时间点出现了两次。

例如 CET (欧洲中部时间) 在 `2016-10-30 02:59:59` 下一秒会跳转到 `2016-10-30 02:00:00` ，也就是说 `02:00:00` 到 `02:59:59` 这一个小时的时间窗出现了两次。

那么，此时，如果要获取到中间的时间窗，在使用 `mktime()` 时就需要手动配置其中的 `tm_isdst` 字段。

<!--
https://www.ibm.com/developerworks/cn/linux/1307_liuming_linuxtime1/
-->

### 字符串转换

其中 `strftime()` 用来格式化日期、日期时间和时间的函数，支持 date、datetime、time 等类，将其转换为字符串；而 `strptime()` 就是从字符串表示的日期时间按格式化字符串要求转换为相应的日期时间。

示例如下：

{% highlight c %}
#define _XOPEN_SOURCE

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
	struct tm tm;
	char buffer[256], *rcs;
	int rc;

	memset(&tm, 0, sizeof(struct tm));
	rcs = strptime("2001-11-12 18:31:01", "%Y-%m-%d %H:%M:%S", &tm);
	if (rcs == NULL) { /* such as: 20010-10-12 */
		printf("Format error\n");
		return -1;
	}

	rc = strftime(buffer, sizeof(buffer), "%4-%m-%d %H:%M:%S (%Z %z)", &tm);
	if (rc == 0) { /* Not enough buffer, */
		buffer[sizeof(buffer) - 1] = 0;
		printf("Got zero\n");
		return -1;
	}

	puts(buffer);

	return 0;
}
{% endhighlight %}

其中 `strftime()` 会按照格式将上述内容格式输出，如果不满足则原样复制，而且格式化字符基本已经占满所有 26 个字符。





<!--
### ANSI clock()

clock() 返回值类型是 clock_t，该值除以 CLOCKS_PER_SEC (GNU 定义为 1000000) 得出消耗的 CPU 时间，一般用两次 clock() 来计算进程自身运行的时间。不过，该函数存在如下的问题：

* 对于 32bits 如果超过 72 分钟，就有可能会导致溢出；
* 该函数没有考虑 CPU 被子进程使用的情况；
* 也不能区分用户空间和内核空间。

因此，该函数在 Linux 系统上几乎没有意义，可以通过如下程序进行简单的测试。

{% highlight c %}
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int i = 1000;
    clock_t start, finish;
    double  duration;
    printf( "Time to do %ld empty loops is ", i );
    start = clock();
    while (--i){
        system("cd");
    }
    finish = clock();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    printf("%f seconds\n", duration);
    return 0;
}
{% endhighlight %}

接下来，可以通过如下方式进行测试。

{% highlight text %}
$ gcc test.c -o test
$ time ./test
Time to do 1000 empty loops is 0.070000 seconds

real    0m1.471s
user    0m0.551s
sys     0m0.749s
{% endhighlight %}

实际上，程序调用 ```system("cd");``` 主要是系统模式子进程的消耗，如上程序不能体现这一点 。




二)times()时间函数
1)概述:
原型如下：
clock_t times(struct tms *buf);


tms结构体如下:
strace tms{
 clock_t tms_utime;
 clock_t tms_stime;
 clock_t tms_cutime;
 clock_t tms_cstime;
}

注释:
tms_utime记录的是进程执行用户代码的时间.
tms_stime记录的是进程执行内核代码的时间.
tms_cutime记录的是子进程执行用户代码的时间.
tms_cstime记录的是子进程执行内核代码的时间.

2)测试:
vi test2.c
#include <sys/times.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

static void do_cmd(char *);
static void pr_times(clock_t, struct tms *, struct tms *);


int main(int argc, char *argv[]){
        int i;
        for(i=1; argv[i]!=NULL; i++){
                do_cmd(argv[i]);
        }
        exit(1);
}
static void do_cmd(char *cmd){
        struct tms tmsstart, tmsend;
        clock_t start, end;
        int status;
        if((start=times(&tmsstart))== -1)
                puts("times error");
        if((status=system(cmd))<0)
                puts("system error");
        if((end=times(&tmsend))== -1)
                puts("times error");
        pr_times(end-start, &tmsstart, &tmsend);
        exit(0);
}
static void pr_times(clock_t real, struct tms *tmsstart, struct tms *tmsend){
        static long clktck=0;
        if(0 == clktck)
                if((clktck=sysconf(_SC_CLK_TCK))<0)
                           puts("sysconf err");
        printf("real:%7.2f\n", real/(double)clktck);
        printf("user-cpu:%7.2f\n", (tmsend->tms_utime - tmsstart->tms_utime)/(double)clktck);
        printf("system-cpu:%7.2f\n", (tmsend->tms_stime - tmsstart->tms_stime)/(double)clktck);
        printf("child-user-cpu:%7.2f\n", (tmsend->tms_cutime - tmsstart->tms_cutime)/(double)clktck);
        printf("child-system-cpu:%7.2f\n", (tmsend->tms_cstime - tmsstart->tms_cstime)/(double)clktck);
}

编译:
gcc test2.c -o test2

测试这个程序:
time ./test2 "dd if=/dev/zero f=/dev/null bs=1M count=10000"
10000+0 records in
10000+0 records out
10485760000 bytes (10 GB) copied, 4.93028 s, 2.1 GB/s
real:   4.94
user-cpu:   0.00
system-cpu:   0.00
child-user-cpu:   0.01
child-system-cpu:   4.82

real    0m4.943s
user    0m0.016s
sys     0m4.828s

3)总结:
(1)通过这个测试,系统的time程序与test2程序输出基本一致了.
(2)(double)clktck是通过clktck=sysconf(_SC_CLK_TCK)来取的,也就是要得到user-cpu所占用的时间,就要用
(tmsend->tms_utime - tmsstart->tms_utime)/(double)clktck);
(3)clock_t times(struct tms *buf);返回值是过去一段时间内时钟嘀嗒的次数.
(4)times()函数返回值也是一个相对时间.


三)实时函数clock_gettime
在POSIX1003.1中增添了这个函数,它的原型如下：
int clock_gettime(clockid_t clk_id, struct timespec *tp);

它有以下的特点:
1)它也有一个时间结构体:timespec ,timespec计算时间次数的单位是十亿分之一秒.
strace timespec{
 time_t tv_sec;
 long tv_nsec;
}

2)clockid_t是确定哪个时钟类型.
CLOCK_REALTIME: 标准POSIX实时时钟
CLOCK_MONOTONIC: POSIX时钟,以恒定速率运行;不会复位和调整,它的取值和CLOCK_REALTIME是一样的.
CLOCK_PROCESS_CPUTIME_ID和CLOCK_THREAD_CPUTIME_ID是CPU中的硬件计时器中实现的.

3)测试:
#include<time.h>
#include<stdio.h>
#include<stdlib.h>

#define MILLION 1000000

int main(void)
{
        long int loop = 1000;
        struct timespec tpstart;
        struct timespec tpend;
        long timedif;
        clock_gettime(CLOCK_MONOTONIC, &tpstart);
        while (--loop){
                system("cd");
        }

        clock_gettime(CLOCK_MONOTONIC, &tpend);
        timedif = MILLION*(tpend.tv_sec-tpstart.tv_sec)+(tpend.tv_nsec-tpstart.tv_nsec)/1000;
        fprintf(stdout, "it took %ld microseconds\n", timedif);

        return 0;
}

编译:
gcc test3.c -lrt -o test3


计算时间:
time ./test3
it took 3463843 microseconds

real    0m3.467s
user    0m0.512s
sys     0m2.936s


2)
clock()函数的精确度是10毫秒(ms)
times()函数的精确度是10毫秒(ms)


3)测试4种函数的精确度:
vi test4.c

#include    <stdio.h>
#include    <stdlib.h>
#include    <unistd.h>
#include    <time.h>
#include    <sys/times.h>
#include    <sys/time.h>
#define WAIT for(i=0;i<298765432;i++);
#define MILLION    1000000
    int
main ( int argc, char *argv[] )
{
    int i;
    long ttt;
    clock_t s,e;
    struct tms aaa;

    s=clock();
    WAIT;
    e=clock();
    printf("clock time : %.12f\n",(e-s)/(double)CLOCKS_PER_SEC);

    long tps = sysconf(_SC_CLK_TCK);
    s=times(&aaa);
    WAIT;
    e=times(&aaa);
    printf("times time : %.12f\n",(e-s)/(double)tps);

    struct timeval tvs,tve;
    gettimeofday(&tvs,NULL);
    WAIT;
    gettimeofday(&tve,NULL);
    double span = tve.tv_sec-tvs.tv_sec + (tve.tv_usec-tvs.tv_usec)/1000000.0;
    printf("gettimeofday time: %.12f\n",span);

    struct timespec tpstart;
    struct timespec tpend;

    clock_gettime(CLOCK_REALTIME, &tpstart);
    WAIT;
    clock_gettime(CLOCK_REALTIME, &tpend);
    double timedif = (tpend.tv_sec-tpstart.tv_sec)+(tpend.tv_nsec-tpstart.tv_nsec)/1000000000.0;
    printf("clock_gettime time: %.12f\n", timedif);

    return EXIT_SUCCESS;
}

gcc -lrt test4.c -o test4
debian:/tmp# ./test4
clock time : 1.190000000000
times time : 1.180000000000
gettimeofday time: 1.186477000000
clock_gettime time: 1.179271718000

六)内核时钟

默认的Linux时钟周期是100HZ,而现在最新的内核时钟周期默认为250HZ.
如何得到内核的时钟周期呢?

grep ^CONFIG_HZ /boot/config-2.6.26-1-xen-amd64

CONFIG_HZ_250=y
CONFIG_HZ=250

结果就是250HZ.

而用sysconf(_SC_CLK_TCK);得到的却是100HZ
例如:
#include    <stdio.h>
#include    <stdlib.h>
#include    <unistd.h>
#include    <time.h>
#include    <sys/times.h>
#include    <sys/time.h>

int
main ( int argc, char *argv[] )
{
    long tps = sysconf(_SC_CLK_TCK);
    printf("%ld\n", tps);

    return EXIT_SUCCESS;
}

为什么得到的是不同的值呢？
因为sysconf(_SC_CLK_TCK)和CONFIG_HZ所代表的意义是不同的.
sysconf(_SC_CLK_TCK)是GNU标准库的clock_t频率.
它的定义位置在:/usr/include/asm/param.h

例如:
#ifndef HZ
#define HZ 100
#endif


最后总结一下内核时间:
内核的标准时间是jiffy,一个jiffy就是一个内部时钟周期,而内部时钟周期是由250HZ的频率所产生中的,也就是一个时钟滴答,间隔时间是4毫秒(ms).
也就是说:
1个jiffy=1个内部时钟周期=250HZ=1个时钟滴答=4毫秒

每经过一个时钟滴答就会调用一次时钟中断处理程序，处理程序用jiffy来累计时钟滴答数,每发生一次时钟中断就增1.
而每个中断之后,系统通过调度程序跟据时间片选择是否要进程继续运行,或让进程进入就绪状态.
最后需要说明的是每个操作系统的时钟滴答频率都是不一样的,LINUX可以选择(100,250,1000)HZ,而DOS的频率是55HZ.

七)为应用程序计时
用time程序可以监视任何命令或脚本占用CPU的情况.

1)bash内置命令time
例如:
time sleep 1
real    0m1.016s
user    0m0.000s
sys     0m0.004s


2)/usr/bin/time的一般命令行
例如:
\time sleep 1
0.00user 0.00system 0:01.01elapsed 0%CPU (0avgtext+0avgdata 0maxresident)k
0inputs+0outputs (1major+176minor)pagefaults 0swaps


注：
在命令前加上斜杠可以绕过内部命令.
/usr/bin/time还可以加上-v看到更具体的输出:
\time -v sleep 1
        Command being timed: "sleep 1"
        User time (seconds): 0.00
        System time (seconds): 0.00
        Percent of CPU this job got: 0%
        Elapsed (wall clock) time (h:mm:ss or m:ss): 0:01.00
        Average shared text size (kbytes): 0
        Average unshared data size (kbytes): 0
        Average stack size (kbytes): 0
        Average total size (kbytes): 0
        Maximum resident set size (kbytes): 0
        Average resident set size (kbytes): 0
        Major (requiring I/O) page faults: 0
        Minor (reclaiming a frame) page faults: 178
        Voluntary context switches: 2
        Involuntary context switches: 0
        Swaps: 0
        File system inputs: 0
        File system outputs: 0
        Socket messages sent: 0
        Socket messages received: 0
        Signals delivered: 0
        Page size (bytes): 4096
        Exit status: 0

这里的输出更多来源于结构体rusage.


最后，我们看到real time大于user time和sys time的总和，这说明进程不是在系统调用中阻塞,就是得不到运行的机会.
而sleep()的运用，也说明了这一点.





http://en.wikipedia.org/wiki/Year_2038_problem


    Identifier  Description
    Time
    manipulation    difftime    computes the difference between times
    time    returns the current time of the system as time since the epoch (which is usually the Unix epoch)
    clock   returns a processor tick count associated with the process

    Format
    conversions     asctime     converts a tm object to a textual representation (deprecated)
    strftime    converts a tm object to custom textual representation
    wcsftime    converts a tm object to custom wide string textual representation
    gmtime  converts time since the epoch to calendar time expressed as Coordinated Universal Time[2]
    localtime   converts time since the epoch to calendar time expressed as local time
    mktime  converts calendar time to time since the epoch
    Constants   CLOCKS_PER_SEC  number of processor clock ticks per second


    tm  calendar time type
    time_t  time since the epoch type
    clock_t     process running time type

http://www.cnblogs.com/wenqiang/p/5678451.html
http://www.cnblogs.com/xmphoenix/archive/2011/05/09/2041546.html
-->


## gettimeofday() 效率

很多时候需要获取当前时间，如计算 http 耗时，数据库事务 ID 等，那么 `gettimeofday()` 这个函数做了些什么？内核 1ms 一次的时钟中断可以支持微秒精度吗？如果在系统繁忙时，频繁的调用它是否有问题吗？

`gettimeofday()` 是 C 库提供的函数，它封装了内核里的 `sys_gettimeofday()` 系统调用。

在 x86_64 体系上，使用 `vsyscall` 实现了 `gettimeofday()` 这个系统调用，简单来说，就是创建了一个共享的内存页面，它的数据由内核来维护，但是，用户态也有权限访问这个内核页面，由此，不通过中断 `gettimeofday()` 也就拿到了系统时间。

### 函数作用

`gettimeofday()` 会把内核保存的墙上时间和 jiffies 综合处理后返回给用户。<!--解释下墙上时间和jiffies是什么：1、墙上时间就是实际时间（1970/1/1号以来的时间），它是由我们主板电池供电的（装过PC机的同学都了解）RTC单元存储的，这样即使机器断电了时间也不用重设。当操作系统启动时，会用这个RTC来初始化墙上时间，接着，内核会在一定精度内根据jiffies维护这个墙上时间。2、jiffies就是操作系统启动后经过的时间，它的单位是节拍数。有些体系架构，1个节拍数是10ms，但我们常用的x86体系下，1个节拍数是1ms。也就是说，jiffies这个全局变量存储了操作系统启动以来共经历了多少毫秒。-->先看看 `gettimeofday()` 是如何做的，首先它调用了 `sys_gettimeofday()` 系统调用。

{% highlight c %}
asmlinkage long sys_gettimeofday(struct timeval __user *tv, struct timezone __user *tz)
{
    if (likely(tv != NULL)) {
        struct timeval ktv;
        do_gettimeofday(&ktv);
        if (copy_to_user(tv, &ktv, sizeof(ktv)))
            return -EFAULT;
    }
    if (unlikely(tz != NULL)) {
        if (copy_to_user(tz, &sys_tz, sizeof(sys_tz)))
            return -EFAULT;
    }
    return 0;
}
{% endhighlight %}

调用 `do_gettimeofday()` 取得当前时间存储到变量 `ktv` 上，并调用 `copy_to_user()` 复制到用户空间，每个体系都有自己的实现，这里就简单看下 x86_64 体系下 `do_gettimeofday()` 的实现：

{% highlight c %}
void do_gettimeofday(struct timeval *tv)
{
    unsigned long seq, t;
    unsigned int sec, usec;

    do {
        seq = read_seqbegin(&xtime_lock);

        sec = xtime.tv_sec;
        usec = xtime.tv_nsec / 1000;

        /* i386 does some correction here to keep the clock
           monotonous even when ntpd is fixing drift.
           But they didn't work for me, there is a non monotonic
           clock anyways with ntp.
           I dropped all corrections now until a real solution can
           be found. Note when you fix it here you need to do the same
           in arch/x86_64/kernel/vsyscall.c and export all needed
           variables in vmlinux.lds. -AK */

        t = (jiffies - wall_jiffies) * (1000000L / HZ) +
            do_gettimeoffset();
        usec += t;

    } while (read_seqretry(&xtime_lock, seq));

    tv->tv_sec = sec + usec / 1000000;
    tv->tv_usec = usec % 1000000;
}
{% endhighlight %}

可以看到，该函数只是把 `xtime` 与 `jiffies` 修正后返回给用户，而 `xtime` 变量和 `jiffies` 的维护更新频率，就决定了时间精度，而 `jiffies` 一般每 10ms 或者 1ms 才处理一次时钟中断，那么这是不是意味着精度只到 1ms ？

### 微秒级精度

获取时间是通过 High Precision Event Timer 维护，这个模块会提供微秒级的中断，并更新 xtime 和 jiffies 变量；接着，看下 x86_64 体系结构下的维护代码：

{% highlight c %}
static struct irqaction irq0 = {
    timer_interrupt, SA_INTERRUPT, CPU_MASK_NONE, "timer", NULL, NULL
};
{% endhighlight %}

这个 `timer_interrupt()` 函数会处理 HPET 时间中断，来更新 xtime 变量。

<!--
三、它的调用成本在所有的操作系统上代价一样吗？如果在系统繁忙时，1毫秒内调用多次有问题吗？

最上面已经说了，对于x86_64系统来说，这是个虚拟系统调用vsyscall！所以，这里它不用发送中断！速度很快，成本低，调用一次的成本大概不到一微秒！

对于i386体系来说，这就是系统调用了！最简单的系统调用都有无法避免的成本：陷入内核态。当我们调用gettimeofday时，将会向内核发送软中断，然后将陷入内核态，这时内核至少要做下列事：处理软中断、保存所有寄存器值、从用户态复制函数参数到内核态、执行、将结果复制到用户态。这些成本至少在1微秒以上！

四、关于jiffies值得一提的两点

先看看它的定义：

[cpp] view plain copy

    volatile unsigned long __jiffies;


只谈两点。

1、它用了一个C语言里比较罕见的关键字volatile，这个关键字用于解决并发问题。c语言编译器很喜欢做优化的，它不清楚某个变量可能会被并发的修改，例如上面的jiffies变量首先是0，如果首先一个CPU修改了它的值为1，紧接着另一个CPU在读它的值，例如 __jiffies = 0; while (__jiffies == 1)，那么在内核的C代码中，如果不加volatile字段，那么第二个CPU里的循环体可能不会被执行到，因为C编译器在对代码做优化时，生成的汇编代码不一定每次都会去读内存！它会根据代码把变量__jiffies设为0，并一直使用下去！而加了volatile字段后，就会要求编译器，每次使用到__jiffies时，都要到内存里真实的读取这个值。


2、它的类型是unsigned long，在32位系统中，最大值也只有43亿不到，从系统启动后49天就到达最大值了，之后就会清0重新开始。那么jiffies达到最大值时的回转问题是怎么解决的呢？或者换句话说，我们需要保证当jiffies回转为一个小的正数时，例如1，要比几十秒毫秒前的大正数大，例如4294967290，要达到jiffies(1)>jiffies(4294967290)这种效果。

内核是通过定义了两个宏来解决的：

[cpp] view plain copy

    #define time_after(a,b)     \
        (typecheck(unsigned long, a) && \
         typecheck(unsigned long, b) && \
         ((long)(b) - (long)(a) < 0))
    #define time_before(a,b)    time_after(b,a)


很巧妙的设计！仅仅把unsigned long转为long类型后相减比较，就达到了jiffies(1)>jiffies(4294967290)效果，简单的解决了jiffies的回转问题，赞一个。
-->


## 总结







{% highlight c %}
//----- 将时间格式转为字符串，不修改时区，使用标准格式
char *asctime(const struct tm *tm);
char *asctime_r(const struct tm *tm, char *buf);

//----- 转换为本地时间，同样使用标准格式
char *ctime(const time_t *timep);
char *ctime_r(const time_t *timep, char *buf);

//----- 将时间戳转换为GMT时区的标准时间
struct tm *gmtime(const time_t *timep);
struct tm *gmtime_r(const time_t *timep, struct tm *result);

//----- 将时间戳转换为本地时区的时间格式
struct tm *localtime(const time_t *timep);
struct tm *localtime_r(const time_t *timep, struct tm *result);

time_t mktime(struct tm *tm);
double difftime(time_t time1, time_t time0);

int gettimeofday(struct timeval *tv, struct timezone *tz);
int settimeofday(const struct timeval *tv , const struct timezone *tz);
{% endhighlight %}

示例如下。

{% highlight c %}
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

static void dump_tm(const struct tm *t, const char *var)
{
        log_info("---> dump <struct tm> %s", var);
        log_info("    %04d %02d %02d %02d:%02d:%02d",
                t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                t->tm_hour, t->tm_min, t->tm_sec);
        log_info("     tm_wday: %02d", t->tm_wday);
        log_info("     tm_yday: %02d", t->tm_yday);
        log_info("    tm_isdst: %02d", t->tm_isdst);
}

static void dump_ts(const struct timespec *ts, const char *var)
{
        log_info("---> dump <struct timespec> %s", var);
        log_info("    %lds %ldns", ts->tv_sec, ts->tv_nsec);
}

static void dump_tv(const struct timeval *v, const char *zone)
{
        log_info("---> dump <struct timeval> %s", zone);
        log_info("    %lds %ldus", v->tv_sec, v->tv_usec);
}


int main(void)
{
        /* system("date -R"); */

        time_t time_now = time(NULL);
        log_info("---> time now: %ld", time_now);

        /* int gettimeofday(struct timeval *tv, struct timezone *tz); */
        struct timeval tv;
        gettimeofday(&tv, NULL);
        dump_tv(&tv, "GMT");

        /* struct tm *gmtime_r(const time_t *timep, struct tm *result); */
        struct tm tm_gmt;
        gmtime_r(&time_now, &tm_gmt);
        dump_tm(&tm_gmt, "GMT");

        time_t time_mk_gmt = mktime(&tm_gmt);
        log_info("---> time gmt: %ld", time_mk_gmt);


        /* struct tm *localtime_r(const time_t *timep, struct tm *result); */
        struct tm tm_local;
        localtime_r(&time_now, &tm_local);
        dump_tm(&tm_local, "local");

        time_t time_mk_local = mktime(&tm_local);
        log_info("---> time local: %ld", time_mk_local);


        /* int clock_gettime(clockid_t clk_id, struct timespec *tp); */
        struct timespec tp;
        clock_gettime(CLOCK_REALTIME, &tp);
        dump_ts(&tp, "CLOCK_REALTIME");

        /* system("cat /proc/uptime"); */
        clock_gettime(CLOCK_MONOTONIC, &tp);
        dump_ts(&tp, "CLOCK_MONOTONIC");

        return 0;
}

{% endhighlight %}


<!--
time_t tt;
struct tm gmtnow, localnow;
char buffer[128];

time(&tt);
gmtime_r(&tt, &gmtnow);
localtime_r(&tt, &localnow);

printf("  ctime(): %s", ctime(&tt));
printf("asctime(): %s", asctime(&gmtnow));
printf("asctime(): %s", asctime(&localnow));





其它使用方法：

* `date -R` 返回本地时间，其中 `-R` 选项附带时区信息 `+0800`；
* `time()` 返回从 Epoch 到现在经过的秒数；
* `gettimeofday()` 是 `time()` 的高精度版本，返回的时间秒相同，大部分库的 `time()` 实现实际上也在调用 `gettimeofday()` 函数；
* `mktime()` 会将 `struct tm` 类型转换为 `time_t` 类型的秒值，转换包含时区信息，也包含夏令时信息。

7、CLOCK_REALTIME获取的是墙上时间(Wall time)，该时间由系统启动时从RTC读取，在系统运行期间由系统时钟维护并在合适的时刻和RTC芯片进行同步。
CLOCK_MONOTONIC取的是相对时间，该时间由系统通过jiffies值来计算，不受时钟源的影响。

4、Linux中_r后缀表示该函数是相应函数的可重入版本。gmtime_r()是gmtime()的可重入(reentrant)版本，用于把time_t类型的秒值转换为struct tm类型
6、localtime_r()用于把time_t类型的秒值转换为struct tm类型，转换包含时区信息。48行经过mktime()的-8，得到最初time()返回的秒值。
8、/proc/uptime文件保存从系统启动到现在的时间(以秒为单位)。

此外，还有一种叫”calendar time“的称呼，其和墙上时间是等效的。

struct tm *localtime(const time_t *timep);
struct tm *localtime_r(const time_t *timep, struct tm *result);
localtime是直接返回strcut tm*指针（如果成功的话）；这个指针是指向一个静态变量的；因此，返回的指针所指向的静态变量有可能被其他地方调用的localtime改掉，例如多线程使用的时候。

localtime_r则是由调用者在第二个参数传入一个struct tm *result指针，该函数会把结果填充到这个传入的指针所指内存里面；成功的返回值指针也就是struct tm *result。

其他的时间函数，如asctime，asctime_r；ctime，ctime_r；gmtime，gmtime_r都是类似的，所以，时间函数的 _r 版本都是线程安全的。

在 Linux 中，可以通过 clock_gettime() 函数返回两类时间。

#### CLOCK_REALTIME

代表了机器所能估计到的当前墙上时间(Wall-Clock)，可能会随着系统时间向前或者向后修改，包括手动设置以及通过 NTP 进行调整。

#### CLOCK_MONOTONIC

也就是递增的时间，一般不受修改墙上时间的影响，但是通过会受到 NTP 调整的影响。

不受 settime()、settimeofday() 影响，但是受 adjtimex() 的影响，如果想不受 NTP 影响的话可以参考 CLOCK_MONOTONIC_RAW，代表着系统独立时钟硬件对时间的统计。

另外，CLOCK_MONTONIC 同样不统计系统休眠时的时间，如果要想统计该时间的话可以使用 CLOCK_BOOTTIME，同时会累加上系统休眠的时间 (total_sleep_time)，它代表着系统上电后的总时间。

#### gettimeofday()

用来返回的是自 1970 年以来的秒数和微秒数，当 NTP 发生跳变时，同样会修改。如果将 clock_gettime() 指定为 CLOCK_REALTIME 时，除了返回的精度有所区别，其它完全一样。

NTP协议
https://github.com/lettier/ntpclient
https://www.cnblogs.com/cloudos/p/NTP.html

### 代码解析

在 glibc 中实际上是通过 vsyscall 调用的系统调用。
https://zhuanlan.zhihu.com/p/31497147

对于一些常用的系统调用，会增加消耗，实际上是通过 VDSO 实现的，其中与实践相关的包括了 time()、gettimeofday()、clock_gettime() 。

在kernel/posix-timers.c中内核实现了clock_gettime的系统调用，

sysdeps/unix/clock_gettime.c
-->




{% highlight text %}
{% endhighlight %}
