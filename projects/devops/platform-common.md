---
title: 通用规范
layout: project
comments: true
language: chinese
category: [misc]
keywords:
description:
---

## 规范

### 变量名

这里的命名规范包括了 `Tags` 。

可用字符为 ASCII ，包括了 `A-Z` `a-z` `0-9` `_` `@` `#` 。

## FAQ


### 任务校验

任务校验可以有多种方法，例如对于 BootAgent ，其上报信息包含上层进行判断的大部分数据，完全不需要再增加额外的机制。

而对于其它类型的 Agent，例如 BasicAgent ，因为会在本地保存一些任务，如果由 Agent 触发，那么可能会出现与上层的不一致(例如上报时有任务在下发，实际上一致，可能会误判为不一致)，同时，如果批量上报可能会对上层造成过大的压力。

为此，建议由服务端发起任务校验，那么一些需要持久化的任务要保证其顺序性，也就是说不同的任务需要标识其并发性。例如，同步/异步命令可以并发；CRON配置则不可以。

## 其它

### 代码量统计

{% highlight text %}
----- 当前目录下 *.h *.c 文件数量
$ find -name '*.c' -or -name '*.c' | wc -l

----- 所有 *.h *.c 文件行数
$ find -name '*.c' -or -name '*.c' | xargs cat | wc -l

----- 过滤空行
$ find -name '*.c' -or -name '*.c' | xargs cat | grep -v ^$ | wc -l
{% endhighlight %}

{% highlight text %}
{% endhighlight %}
