---
title: Bash 常用脚本
layout: post
comments: true
language: chinese
category: [program,linux,misc]
keywords: bash
description: 简单整理下常用的脚步。
---

简单整理下常用的脚步。

<!-- more -->

## 采集 CPU 使用率

<!--time timeout -k 5 -s INT 3 sleep 10 -->

{% highlight bash %}
#!/bin/bash

CAPATH="/tmp/foobar/foobar"
CAPID=`pidof ${CAPATH}`

if [[ "x${CAPID}" == "x" || ! "${CAPID}" =~ ^[0-9]*$ ]]; then
        exit 0
fi

HERTZ=`getconf CLK_TCK 2>&1`
CPUSAGE=0.0

if [[ "x${HERTZ}" == "x" || ! "${HERTZ}" =~ ^[0-9]*$ ]]; then
        exit 1
fi

function get_usage() {
        BOOTIME=`awk '{ print $1 }' /proc/uptime 2>&1`
        if [[ "x${BOOTIME}" == "x" || ! "${BOOTIME}" =~ ^[0-9.]*$ ]]; then
                exit 1
        fi

        USAGE=`awk -v bootime="${BOOTIME}" -v hz="${HERTZ}" 'BEGIN{percent=0}; {  \
                tmp = bootime * hz - $22;                                           \
                if (tmp <=0) percent = 0; else percent = ($14 + $15) * 100 / tmp    \
                } END{print int(percent)}' /proc/${1}/stat`
        if [[ "x${USAGE}" == "x" || ! "${USAGE}" =~ ^[0-9.]*$ ]]; then
                CPUSAGE=0.0
                return 1
        fi
        CPUSAGE=${USAGE}
}

for i in {1..3}; do
        get_usage ${CAPID}
        if [[ ${CPUSAGE} -lt "80" ]]; then
                exit 0
        fi
        if [[ ${i} -ge 3 ]]; then
                kill -9 `pidof ${CAPATH}`
                exit 1
        fi
        sleep 3
done
{% endhighlight %}

{% highlight text %}
{% endhighlight %}
