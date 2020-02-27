---
title: Hash 函数简介
layout: post
comments: true
language: chinese
category: [program,misc]
keywords:
description:
---


<!-- more -->

## MurmurHash

实际上常见的 Hash 函数包括了 MD5、SHA1、SHA256 这类的，这些算法通常讲究安全性，而实际上很多场景下对安全性不敏感，例如这里的 Murmur ，一般要快几十倍。

这一算法在 Redis、Memcached、Cassandra、HBase、Lucene 上都有使用，默默无名却基本已经一统江湖。其中 `mur` 这个名字来源于其核心算法的处理方式，全称是 MUltiply and Rotate 的意思，因为算法的核心就是不断的 `x *= m; x = rotate_left(x,r);` 。

<!--
那Java自己的String的hashCode()呢？ 用的是Horner法则， s[0]*31^(n-1) + s[1]*31^(n-2) + ... + s[n-1]

实现上是循环原哈希值×31再加下一个Char的值，算法及其复杂度简单一目了然，连我都能看懂。

for (int i = 0; i < str.length(); i++) { hash = 31*hash + str.charAt[i]; }

注意，int会溢出成负数，再溢成正数，比如"abcde"是 92599395， abcdef"是 -1424385949， "abcdefgh" 是 1259673732

Eclipse的自动代码生成的hashCode()函数也是类似的，循环原哈希值×31，再加下一个属性的hashCode()值。

31有个很好的特性，就是用移位和减法来代替乘法，可以得到更好的性能：31*i==(i<<5)-i。现在的VM可以自动完成这种优化。 Java自己的HashMap，会在调用Key的hashCode()得到一个数值后，用以下算法再hash一次，免得Key自己实现的hashCode()质量太差。

static int hash(int h) {
    h ^= (h >>> 20) ^ (h >>> 12);
    return h ^ (h >>> 7) ^ (h >>> 4);
}

那为什么Cassandra们不选简单直白的Horner法则呢？

我猜原因之一是它的性能，有个评测用js来跑的，还是murmur好些。

再猜原因之二它的变化不够激烈，比如"abc"是96354， "abd"就比它多1。而用 murmur"abc"是1118836419，"abd"是413429783。像在Jedis里，它的几个Shard节点的名字就叫做shard-1,shard-2，shard-3而已，hash这几个字符串，murmur的结果还能均匀的落在一致性哈希环上，用Honer法则就不行了。
-->























<!--
DJBHash()
 - hash += (hash << 5) + (*str++);
 + hash = ((hash << 5) + hash)  + (*str++);

https://www.byvoid.com/zhs/blog/string-hash-compare
http://www.partow.net/programming/hashfunctions/index.html

https://github.com/davidar/c-hashtable  ****
https://github.com/larsendt/hashtable
https://github.com/DavidLeeds/hashmap
https://github.com/janneku/hashmap
https://github.com/ankurs/Hash-Table

C library for consistent hashing, and langauge bindings
https://github.com/RJ/ketama
https://github.com/chrismoos/hash-ring

宏实现的HashTable
https://github.com/troydhanson/uthash

介绍如何写HashTable
https://github.com/jamesroutley/write-a-hash-table

Cuckoo Hash算法
https://github.com/kroki/Cuckoo-hash

Murmur算法
https://github.com/PeterScott/murmur3

非加密算法xxHash
https://github.com/Cyan4973/xxHash

漫谈非加密哈希算法
https://segmentfault.com/a/1190000010990136



Hash算法
Robin Hood Hashing
HyperLogLog
FNV hash

Fowler-Noll-Vo, FNV 哈希算法是以三位发明人 Glenn Fowler、Landon Curt Noll、Phong Vo 的名字来命名的，最早在 1991 年提出；可以保持较小的冲突率，高度分散使它适用于 Hash 一些非常相近的字符串，比如 URL、Hostname、文件名、text、IP 地址等。


## Hash Table
允许多线程读写的并发Hash库，以及murmur3
https://github.com/efficient/libcuckoo
https://github.com/savoirfairelinux/opendht

非加密 Hash 算法使用比较多的是 [xxHash](https://github.com/Cyan4973/xxHash)、



Google实现的既简单的Consistent Hash
https://blog.helong.info/blog/2015/03/13/jump_consistent_hash/


https://github.com/DavidLeeds/hashmap




底层数据结构的实现及其压测
https://attractivechaos.wordpress.com/2008/10/07/another-look-at-my-old-benchmark/
https://attractivechaos.wordpress.com/2018/01/13/revisiting-hash-table-performance/
https://attractivechaos.wordpress.com/2008/08/28/comparison-of-hash-table-libraries/
https://attractivechaos.wordpress.com/2008/09/02/implementing-generic-hash-library-in-c/
据说谷歌的hash性能很好
https://github.com/sparsehash/sparsehash

从uthash开始修改
https://github.com/troydhanson/uthash

一个 Hash 表一般包括了如下的几个步骤。

* add/replace
* find
* delete
* count
* iterate
* sort

## Hash 算法

简单高效的字符串哈希算有 BKDRHash、APHash、DJBHash、JSHash、RSHash、SDBMHash、PJWHash、ELFHash 等，据说 BKDRHash 算法的效果最好。

### Jenkins Hash

开始 Bob Jenkins 提出了多个基于字符串的通用 Hash 算法，随后，Thomas Wang 在此基础上，针对固定整数输入做了相应的 Hash 算法，因此也被称为 Wang-Jenkins Hash 算法。

其关键特性有：A) 雪崩性，更改任意一位将引起输出有一半以上的位变化；B) 可逆性。

http://d0evi1.com/wang-jenkins-hash/
https://naml.us/tags/thomas-wang/
http://burtleburtle.net/bob/hash/integer.html

### City Hash

Google 在今年的四月份公布了一个叫做 CityHash 的方法，能比较快的生成一个 64 位的字符串签名，而且在 64 位 CPU 上做了特殊优化。

https://web.stanford.edu/class/ee380/Abstracts/121017-slides.pdf
https://github.com/google/cityhash
https://blog.csdn.net/yfkiss/article/details/7337382

### Murmur Hash

/post/hash-functions-introduce.html

https://github.com/attractivechaos/klib/blob/master/khash.h
http://troydhanson.github.io/uthash/userguide.html#hash_functions
https://github.com/attractivechaos/udb2/blob/master/common.c#L8
https://github.com/LambdaSchool/C-Hash-Tables

Three "extras" come with uthash. These provide lists, dynamic arrays and strings:

utlist.h provides linked list macros for C structures.
utarray.h implements dynamic arrays using macros.
utstring.h implements a basic dynamic string.

解决冲突的方法有：A) 链式；B) 开放寻址 (Open Addressing) 。后者可以是 Liner Probing

Lock-Free的实现
https://github.com/divfor/atomic_hash
https://preshing.com/20130605/the-worlds-simplest-lock-free-hash-table/


https://www.jianshu.com/p/5d6c9a77a317


###########################################
## 一致性Hash
###########################################

https://yikun.github.io/2016/06/09/%E4%B8%80%E8%87%B4%E6%80%A7%E5%93%88%E5%B8%8C%E7%AE%97%E6%B3%95%E7%9A%84%E7%90%86%E8%A7%A3%E4%B8%8E%E5%AE%9E%E8%B7%B5/
http://blog.csdn.net/cywosp/article/details/23397179

普通 Hash 算法的劣势是当 Node 数发生变化后，数据项会被重新 "打散"，导致大部分数据项不能落到原来的节点上，从而导致大量数据需要迁移。一致性 Hash 算法的一个重要目的就是当增加或者删除节点时，对于大多数 item，保证原来分配到的某个 Node，现在仍然应该分配到那个 Node，将数据迁移量的降到最低。

https://en.wikipedia.org/wiki/Consistent_hashing
https://github.com/chrismoos/hash-ring

https://github.com/clibs/hash
https://github.com/kroki/Cuckoo-hash
https://github.com/Cyan4973/xxHash
https://github.com/troydhanson/uthash
https://github.com/PeterScott/murmur3

-->

## 参考

[Murmur3 C](https://github.com/PeterScott/murmur3) C 语言实现的 Murmur3 哈希算法。

{% highlight text %}
{% endhighlight %}
