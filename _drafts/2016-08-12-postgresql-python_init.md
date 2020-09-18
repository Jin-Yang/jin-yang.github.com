---
Date: October 19, 2013
title: 通过 Python 访问 PG
layout: post
comments: true
language: chinese
category: [sql]
---

实际连接 PG 可以通过至少三种方式连接：PsyCopg、PyPgSQL、PyGreSQL，在此使用 psycopg2，相比来说其代码少，速度块，稳定。


<!-- more -->

# 简介

PsyCopg 满足 [Python DB API 2.0](https://www.python.org/dev/peps/pep-0249/) 标准，如下是一个简单的示例。

{% highlight python %}
#!/usr/bin/env python
#-*- coding:utf-8 -*-
import psycopg2
import pprint

conn = psycopg2.connect(
    database="postgres",
    user="postgres",
    password="",
    host="127.1",
    port="5432")

cur = conn.cursor()
cur.execute("SELECT * FROM pg_tablespace LIMIT 10")
rows = cur.fetchall()
pprint.pprint(rows)

conn.close()
{% endhighlight %}

cursor 实际保存了一个 fetch 操作的上下文，在 Python DB API 2.0 中规定，如果操作系统不支持，那么建议通过软件自己实现。


# 参考

相关的介绍可以直接参考官方文档 [initd.org/psycopg](https://initd.org/psycopg/) ，以及 PG 中与此相关的介绍，包括了主要的使用方式 [Using psycopg2 with PostgreSQL](https://wiki.postgresql.org/wiki/Using_psycopg2_with_PostgreSQL) 。

{% highlight text %}
{% endhighlight %}
