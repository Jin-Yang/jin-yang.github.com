---
Date: October 19, 2013
title: Linux 的 RCU 机制
layout: post
comments: true
language: chinese
category: [linux]
---

<!-- more -->







# seqlock 顺序锁


用于能够区分读与写的场合，并且是读操作很多、写操作很少，写操作的优先权大于读操作。

seqlock的实现思路是，用一个递增的整型数表示sequence。写操作进入临界区时，sequence++；退出临界区时，sequence再++。写操作还需要获得一个锁（比如mutex），这个锁仅用于写写互斥，以保证同一时间最多只有一个正在进行的写操作。

当sequence为奇数时，表示有写操作正在进行，这时读操作要进入临界区需要等待，直到sequence变为偶数。读操作进入临界区时，需要记录下当前sequence的值，等它退出临界区的时候用记录的sequence与当前sequence做比较，不相等则表示在读操作进入临界区期间发生了写操作，这时候读操作读到的东西是无效的，需要返回重试。





# 参考

关于 RCU 的实现机制，可以参考 [Introduction to RCU][rcu-paulmck]，Kernel 实现者整理的相关资料，十分详细。

关于并行编程可以参考 [Is Parallel Programming Hard, And, If So, What Can You Do About It?][parallel-program]，一篇不错的介绍文章，包括了免费的 PDF，也可以从 [本地][parallel-local] 下载，版权归作者所有。



[rcu-paulmck]:           http://www2.rdrop.com/users/paulmck/RCU/                                         "RCU编程，Kernel 实现者整理的相关资料"
[parallel-program]:      https://www.kernel.org/pub/linux/kernel/people/paulmck/perfbook/perfbook.html    "关于并行编程，一本不错的书"
[parallel-local]:        /reference/linux/perfbook.2015.01.31a.pdf                                        "关于并行编程，一本不错的书，本地保存版本"
