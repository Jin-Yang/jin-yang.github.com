---
title: InnoDB 锁管理
layout: post
comments: true
language: chinese
category: [mysql,database]
---


<!-- more -->

InnoDB 对于主表采用聚簇索引，也就是表数据和主键一起存储，并按照主键递增顺序排序，主键索引的叶结点存储行数据；对于普通索引(或者二级索引)，其叶子节点存储的是主键值。主键只锁单条记录；二级索引则包括了索引+主表；非索引全表扫描则会锁全表。不同隔离级别锁处理方式不同，同时死锁也不同；扫描范围同样不同；lock_sys_t@include/lock0lock.hlock_sys@lock/lock0lock.cc 全局变量(L290)lock_t@include/lock0types.hlatch闩锁(轻量级的锁)，要求锁定的时间非常短，若持闩时间长，则应用性能变的非常差；InnoDB中可分为mutex(互斥锁)和rwlock(读写锁)，其目的用来保证并发线程操作临界资源的正确性，并且没有死锁检测的机制。http://www.cnblogs.com/olinux/p/5174145.html


MySQL 加锁处理分析
http://hedengcheng.com/?p=771

MySQL数据库InnoDB存储引擎中的锁机制
http://www.searchdatabase.com.cn/showcontent_61663.htm

[MySQL5.7] Innodb的索引锁优化
https://yq.aliyun.com/articles/41087

MySQL · 引擎特性 · InnoDB 事务锁简介
https://yq.aliyun.com/articles/4270

https://blogs.oracle.com/mysqlinnodb/resource/innodb-trxlocks.pdf


{% highlight text %}
{% endhighlight %}
