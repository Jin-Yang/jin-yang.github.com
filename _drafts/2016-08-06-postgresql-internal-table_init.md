---
Date: October 19, 2013
title: PG 内部表
layout: post
comments: true
language: chinese
category: [sql]
---


<!-- more -->

## pg_class

记录了几乎和表类似对象的所有字段，包括索引 (还需要参考pg_index)、序列、视图、物化视图、符合类型等。


{% highlight text %}
relkind:   r(ordinary table，普通表)
{% endhighlight %}

## pg_stat_activity

保存了当前连接的状态信息，可以通过如下 SQL 查询最近执行的 SQL 值。

{% highlight text %}
SELECT * FROM pg_stat_activity;
{% endhighlight %}

{% highlight text %}
{% endhighlight %}
