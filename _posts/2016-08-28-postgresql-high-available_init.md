---
Date: October 19, 2013
title: PG 高可用
layout: post
comments: true
language: chinese
category: [sql]
---

<!-- more -->


# Warm Standby

Runs normal crash recovery code, just never finishes.

只要是异步都会有丢失数据的风险。

冷备：PITR 通常是在 WAL 日志 16M 打包之后，或者到了 archive_timeout 之后，从而将数据丢失的风险缩小到 timeout 。

温备：将日志归档+PITR恢复结合起来，只是从未停止恢复，采用 pg_standby 。

这样就导致如果 timeout 设置的较高就会导致延迟较高，低的话就会对磁盘 IO 造成较高的压力，这样就有了流复制。

1. 获得基线。

2. 通过restore_command恢复。

3. 开始流复制。

近乎实时，wal_sender_delay = 200ms
通过 trigger 停止，如果没有 trigger 则不停止。


recovery_connections=on



# Warm-Standy







{% highlight text %}
{% endhighlight %}
