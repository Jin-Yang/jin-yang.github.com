---
title: Systemd 服务管理
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: linux,notify,watchdog
description:
---

Systemd 提供了很多系统服务的基本功能，包括了系统的系统进程、启动级别、cgroup 管理、进程管理等等。

这里简单介绍进程管理功能。

<!-- more -->

## 简介

这里整理一下常用的配置参数。

{% highlight text %}
Restart=always                       # 只要不通过stop停止，任何情况下都必须重启服务
RestartSec=5                         # 重启间隔，异常后等待5秒再启动，默认为100ms
RestartPreventExitStatus=12 SIGKILL  # 当退出码为12或者受到KILL信号时不再自动重启
{% endhighlight %}

<!--
StartLimitInterval: 无限次重启，默认是10秒内如果重启超过5次则不再重启，设置为0表示不限次数重启
-->

## 自动重启

常见的大概有如下的几种场景。

### 始终重启

大概每 3 秒重启一次，时间不会是严格的 3 秒，可能会略有出入。

{% highlight text %}
Restart=always
RestartSec=3
{% endhighlight %}

如果设置为 `on-failure` ，那么当退出码为 0 时，将不会再重启，此时应该设置为 `always` 模式。

### 限制重启次数

主要是为了防止程序异常，导致进程一直重复被拉起，可以限制某段时间内的重启次数。

systemd 可以限制重启的次数，当服务在 `StartLimitInterval` 内超过了 `StartLimitBurst` 次后，将不会再次拉起服务。默认的配置会从 `/etc/systemd/system.conf` 中继承相关参数。

{% highlight text %}
DefaultStartLimitInterval=10s
DefaultStartLimitBurst=5
{% endhighlight %}

也就是说，如果在 `10s` 内超过了 5 次，那么就不会再重新拉起，此时会打印 `start request repeated too quickly for XXX.service` ，然后服务进入 `failed` 状态。

当 `StartLimitBurst=0` 时，实际上是不会限制重启次数的。

#### 版本依赖

在 `v229` 版本之前，采用的是如下的配置，是在 `Service` 段中。

{% highlight text %}
[Service]
StartLimitInterval=200s
StartLimitBurst=5
{% endhighlight %}

在 `v230` 之后，采用的是下面配置，注意，此时在 `Unit` 段中，当然为了向前兼容，也支持上述的方式。

{% highlight text %}
[Unit]
StartLimitIntervalSec=200s
StartLimitBurst=5
{% endhighlight %}

#### 注意

这里需要配合 `RestartSec` 参数，才可以，否则仍然可能会一直生效。

<!--
{% highlight text %}
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/time.h>

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

enum {
        LOG_INFO,
        LOG_WARN,
        LOG_ERROR,
        LOG_MAX,
};
static const char *LVLS[LOG_MAX] = {
        "INFO ",
        "WARN ",
        "ERROR",
};

void log_it(int severity, const char *const fmt, ...)
{
        int rc, len;
        va_list ap;
        char buff[128], *ptr, *end;
        struct tm tm;
        struct timeval now;

        if (unlikely(gettimeofday(&now, NULL) < 0))
                return;
        localtime_r(&now.tv_sec, &tm);
        rc = strftime(buff, sizeof(buff), "%m-%d %T.", &tm);
        end = buff + sizeof(buff);
        ptr = buff + rc;
        rc = snprintf(ptr, end - ptr, "%03d %s", (int)(now.tv_usec / 1e3), LVLS[severity]);
        ptr += rc;
        len = end - ptr;

        va_start(ap, fmt);
        rc = vsnprintf(ptr, len, fmt, ap);
        va_end(ap);

        if (rc >= len) {
                buff[sizeof(buff) - 1] = '\n';
                buff[sizeof(buff) - 2] = '.';
                buff[sizeof(buff) - 3] = '.';
                buff[sizeof(buff) - 4] = '.';
                len = sizeof(buff);
        } else {
                *(ptr + rc) = '\n';
                len = ptr - buff + 1 + rc;
        }
        write(1, buff, len);
}

#define log_info(...)  log_it(LOG_INFO, __VA_ARGS__);
#define log_error(...) log_it(LOG_ERROR, __VA_ARGS__);


int main(int argc, char *argv[])
{
        log_info("======= BEGIN >>>>>>>>");

        return 0;
}
{% endhighlight %}

[Unit]
Description=Just for test
After=network.target

[Service]
Type=simple
ExecStart=/tmp/systemd/foobar
KillMode=process
KillSignal=SIGINT
Restart=always
RestartSec=1

StartLimitBurst=0
StartLimitInterval=10s

[Install]
WantedBy=multi-user.target

systemctl daemon-reload
systemctl restart foobar
-->

## notify

systemd 提供了一种 notify 机制，用于服务向 systemd 发送当前状态信息，然后可以直接通过通用的 `systemctl status XXX.service` 命令查看。

为了方便使用，同时提供了 `systemd-notify(1)` 命令，以及相关的 C API 接口 `sd_notify(3)` ，当然，C 接口使用需要依赖 `libsystemd-devel` 库。

其中 C 接口的使用可以详细查看 [SD_NOTIFY(3)](https://manpages.debian.org/jessie/libsystemd-dev/sd_notify.3.en.html) 中的介绍，下面仅以脚本的方式作为示例。

### 示例脚本

新建测试脚本 `/tmp/systemd/mytest.sh` 以及服务配置文件 `/etc/systemd/system/mytest.service` 。

{% highlight bash %}
#!/bin/bash

echo "Current NOTIFY_SOCKET '${NOTIFY_SOCKET}', WATCHDOG_USEC '${WATCHDOG_USEC}'"
mkfifo /tmp/waldo
sleep 10    # activating (start)
systemd-notify --ready --status="First waiting for data..."
            # active (running) && Status: "Waiting for data..."

while : ; do
        read a < /tmp/waldo
        systemd-notify --status="Processing $a"

        # Do something with $a …
        sleep 10

        systemd-notify --status="Waiting for data..."
done
{% endhighlight %}

{% highlight text %}
[Unit]
Description=My Test

[Service]
Type=notify
ExecStart=/tmp/systemd/mytest.sh

[Install]
WantedBy=multi-user.target
{% endhighlight %}

注意，脚本需要有可执行权限，否则会报错。

常用的操作如下。

{% highlight text %}
# systemctl daemon-reload
# systemctl stop mytest.service
# systemctl start mytest.service
# systemctl status mytest.service
{% endhighlight %}

启动时，`Active` 状态处于 `activating (start)`，如下。

{% highlight text %}
● mytest.service - My Test
   Loaded: loaded (/etc/systemd/system/mytest.service; disabled; vendor preset: disabled)
   Active: activating (start) since Mon 2018-05-14 19:25:17 CST; 3s ago
 Main PID: 31635 (mytest.sh)
   Memory: 296.0K
   CGroup: /system.slice/mytest.service
           ├─31635 /bin/bash /tmp/systemd/mytest.sh
           └─31637 sleep 10
{% endhighlight %}

10 秒之后，也就是服务已经正常启动，同时设置了状态信息，此时会显示如下，注意其中的 `Status` 字段。

{% highlight text %}
● mytest.service - My Test
   Loaded: loaded (/etc/systemd/system/mytest.service; disabled; vendor preset: disabled)
   Active: active (running) since Mon 2018-05-14 19:25:27 CST; 17s ago
 Main PID: 31635 (mytest.sh)
   Status: "Waiting for data..."
   Memory: 204.0K
   CGroup: /system.slice/mytest.service
           └─31635 /bin/bash /tmp/systemd/mytest.sh
{% endhighlight %}

通过 `echo somedata | tee /tmp/waldo` 命令向管道发送一些数据，那么会进入到数据处理过程，也就是处于如下状态。

{% highlight text %}
● mytest.service - My Test
   Loaded: loaded (/etc/systemd/system/mytest.service; disabled; vendor preset: disabled)
   Active: active (running) since Tue 2019-05-14 19:25:27 CST; 10min ago
 Main PID: 31635 (mytest.sh)
   Status: "Processing somedata"
   Memory: 296.0K
   CGroup: /system.slice/mytest.service
           ├─31635 /bin/bash /tmp/systemd/mytest.sh
           └─38956 sleep 10
{% endhighlight %}

实际上，该命令通过 `NOTIFY_SOCKET` 环境变量提供了一个 Unix Domain Socket 接口，在 CentOS 中，一般是在 `/run/systemd/notify` 路径下。

默认只接受主进程发送的请求，如果是单独起了一个进程发送请求，那么需要修改 `NotifyAccess` 的配置项。

#### 总结

通过 notify 接口，可以明确告知 systemd 服务何时启动正常；可以确定当前服务的执行状态。

## watchdog

默认是不开启的，可以通过 `WatchdogSec` 选项设置，此时会在启动服务的时候设置 `WATCHDOG_USEC` 环境变量，注意，单位是微秒。

`systemd-notify` 没有提供更新 watchdog 的方法，对于 `sd_notify()` 方法，直接周期调用 `sd_notify(0,"WATCHDOG=1")` 即可。

### 示例

{% highlight text %}
[Unit]
Description=My Test

[Service]
Type=notify
ExecStart=/usr/bin/sleep 1000
WatchdogSec=60s

[Install]
WantedBy=multi-user.target
{% endhighlight %}

如果脚本里没有周期更新状态，那么超时之后会进入到 failed 状态，此时会在 `/var/log/messages` 中打印如下的日志信息。

{% highlight text %}
May 14 20:00:24 systemd: Starting My Test...
May 14 20:00:24 mytest.sh: Current NOTIFY_SOCKET '/run/systemd/notify', WATCHDOG_USEC '60000000'
May 14 20:00:34 systemd: Started My Test.
May 14 20:01:35 systemd: mytest.service watchdog timeout (limit 1min)!
May 14 20:01:35 systemd: mytest.service: main process exited, code=killed, status=6/ABRT
May 14 20:01:35 systemd: Unit mytest.service entered failed state.
May 14 20:01:35 systemd: mytest.service failed.
{% endhighlight %}

当然，也可以通过如下的配置，在服务 Watchdog 异常之后，自动重新拉起。

{% highlight text %}
[Service]
Restart=always
RestartSec=3
{% endhighlight %}

另外，当一个服务异常时，可以通过 `OnFailure=notify-failed@%n` 配置的方式向另外一个服务发送消息。

## cgroup

在服务的配置文件中，可以通过 `CPUQuota` `MemoryLimit` 来设置 cgroup ，当然也可以通过如下方式临时配置。

{% highlight text %}
# systemctl set-property --runtime uagent.service CPUQuota=5% MemoryLimit=30M
{% endhighlight %}

关于资源配置的选项可以通过 `man 5 systemd.resource-control` 方式查看，默认是没有开启审计的，所以通过 `systemd-cgtop` 没有显示具体的资源。

<!--
很多相关的内核文档链接
https://www.freedesktop.org/software/systemd/man/systemd.resource-control.html

How to use systemd notify
https://askubuntu.com/questions/1120023/how-to-use-systemd-notify
-->

{% highlight text %}
{% endhighlight %}
