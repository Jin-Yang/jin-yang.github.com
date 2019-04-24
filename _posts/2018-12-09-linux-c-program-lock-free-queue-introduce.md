---
title: lock-free 编程
layout: post
comments: true
language: chinese
category: [program]
keywords: lock free,queue
description: 简单介绍下无锁编程 (Lock Free) 的概念，并实现一个基本的 Queue 。
---

简单介绍下无锁编程 (Lock Free) 的概念，并实现一个基本的 Queue 。

<!-- more -->

## CAS

在开始说无锁队列或者无锁编程之前，首先需要知道一个很重要的 CAS 操作，也就是 `Compare & Set` 或者 `Compare & Swap` 的简称，现在几乎所有的 CPU 都支持 CAS 的原子操作，其中 x86 下对应的是 `CMPXCHG` 汇编指令。

一般来说，会在一个循环中重复地执行 CAS 操作以完成一个事务操作，这个过程分为 3 步：

* 从指定的内存位置读取原始的值；
* 根据读取到的原始的值计算出新的值；
* 检测如果内存位置仍然是原始的值时，则将新值写入该内存位置；

如下是一个简单示例，检查内存中的 `*mem` 的值是否为 `oldval` ，如果是则对其赋值 `newval` 。

{% highlight c %}
int compare_and_swap (int *mem, int oldval, int newval)
{
	int oldmem = *mem;
	if (oldmem == oldval)
		*mem = newval;
	return oldmem;
}
{% endhighlight %}

## 添加节点

添加节点时分成了两步操作：A) 将新 `node` 添加到最后节点的下个位置；B) 修改 `root` 中 `tail` 指针的指向。

{% highlight text %}
while (1) {
	n = root->tail;
	if (CAS(&(n->next), NULL, node))
		break;
}
CAS(&(root->tail), n, node);
{% endhighlight %}

如上实际上是一个死循环处理，也就是说在准备在队列尾加入结点时，可能别的线程已经加成功了，于是原保存的 `tail` 指针就不再是 `NULL` 了，那么此时的 `CAS` 操作就会返回 `false` ，于是程序会一直重试直到成功。

另外，也可以看到，在处理 `root->tail` 指针时，实际上并没有判断是否成功，这是因为：

1. 线程 A 在 CAS 操作中更新成功，那么所有的其它线程都会操作失败，然后再进入循环；
2. 此时 `tail->next` 指向的已经不再是 NULL 了，那么其它线程会一直失败并陷入到死循环中；
3. 直到 A 线程更新完 `root->tail` 指针，于是其它的线程中的某个线程就可以再次更新 tail 指针，然后继续往下处理。

注意，这里还会有个问题，假设线程 A 在更新 `root->tail` 前线程挂掉了，那么此时其它线程就会进入到死循环。

### 优化

接上，假设在线程 B 中当发现 CAS 失败了，那么会执行如下 `2` 中的 CAS 操作，也即是如果此时的 `root->tail` 与 `n` 相同，会自动更新 `tail` 。

实际上，这一步与在线程 A 中执行 `3` 操作的结果相同，不过这时在线程 A 中 `3` 操作就会直接失败掉，当然，没有对返回结果进行判断，自然也不影响程序的正确性。

{% highlight text %}
while (1) {
	n = root->tail;
	if (CAS(&(n->next), NULL, node)) // 1
		break;
	else
		CAS(&(root->tail), n, node); // 2
}
CAS(&(root->tail), n, node); // 3
{% endhighlight %}

## 获取节点

示例代码如下。

{% highlight text %}
while (1) {
	n = root->head;
	if (n->next == NULL)
		return NULL;
	if (CAS(&(root->head), n, n->next)
		break;
}
return n->next->data;
{% endhighlight %}

实际上可以看到，这里返回的实际上是 `head->next` 中的数据，而非 `head` 中保存的数据。

这里主要是为了处理一个边界条件，这里通过一个哨兵 (Sentinel) 节点来处理链表中只有一个元素时的问题，同时简化了添加、获取时的操作。

## ABA 问题

ABA 问题，简单来说，就是如果一个线程修改 V 值，假设原来是 A，先修改成 B，再修改回成 A，当前线程的 CAS 操作无法分辨当前 V 值是否发生过变化。

在 Lock-Free 编程中，ABA 问题是最容易发生的，尤其是在使用 CAS 操作时。因为 CAS 判断的是指针操作，如果这个地址被重用那么就会引起异常，实际上，在内存被释放再分配时，很容易被复用，尤其是对于相同大小的内存，可能会被认为一种优化策略。

尤其是对于没有垃圾回收机制的内存模型，例如 C/C++ ，假设有如下事件序列：

1. 线程 A 从内存位置 M 中取出 PTR 指针，PTR 指向内存位置 VAL 含有 V1 值。
2. 线程 B 从位置 M 中取出 PTR ，并进行一系列操作后释放了 PTR 指向内存 VAL 中 V1 值。
3. 线程 B 重新申请内存，并恰好得到了原内存位置 VAL ，然后将 V2 写入到 VAL 中。
4. 线程 B 将内存指针 PTR 更新到 M 。
5. 线程 A 进行 CAS 操作，发现位置 M 中仍然是 PTR 指向的内存位置 VAL ，操作成功。

也就是说，线程 A 无法感知到线程 B 的修改；如果线程 B 只是释放了内存，而线程 A 在 CAS 之前还要访问 PTR 中得内容，那么线程 A 将会访问到一个野指针。

<!--
### 队列问题

如上介绍的是 ABA 的基本概念，那么在这里实现的无锁队列又是什么样的场景。

--allmatches

关于 ABA 问题的讨论
https://github.com/mulle-c/mulle-aba


## 关于线程操作

pthread_cond_signal() 感觉这个函数中会有一个缓存队列

注意，

1. 条件信号是没有缓存队列的，如果发送信号时有线程阻塞，那么该线程就会收到信号。例如，在队列中，只有当队列中无元素时，才需要阻塞。
2. 通过signal发送信号时，只会触发一个线程，除非使用 broadcast ，不过此时需要处理惊群的问题。
3. 如果消费者会将多个消息合并进行发送，例如监控数据为了高效利用带宽，那么完全使用阻塞方式实际上是不合理的，有可能会由于线程一直没有被唤醒，导致数据发送延迟。

在示例 1 中，以阻塞方式消费信息，每拿到一条记录后直接消费处理。

1. 首先启动消费者，阻塞等待接收消息。
2. 启动生产者，开始方式消息。
3. 等待生产者数据生成。
4. 休眠10秒，等待所有的消息都被消费。(因为消费者是阻塞等待，不确定何时完全消费完成)
5. 发送停止消息，停止任务，并退出。

这里对每个消费者消费的信息数进行了统计，用于查看消费者的负载情况，可以发现，负载是真心不均衡的。

在示例 2 中，为了提高效率将多个消息合并发送，这里同时会涉及到两种策略：A) 尽量提高带宽；B) 尽量减小延迟。

1. 对于 A 来说，如果消费者中发现没有足够的报文，那么本次的数据将会缓存，直到有足够的数据，也就意味着这里的缓存等待时间不可控，无法保证实时。
2. 对于 B 来说，可以做到的是尽量利用带宽，当消息不足以填充一次报文的时候就直接发送，而不是继续缓存。


https://github.com/darkautism/lfqueue
https://github.com/haipome/lock_free_queue
https://github.com/rmind/ringbuf
https://github.com/supermartian/lockfree-queue
https://github.com/chaoran/fast-wait-free-queue
http://kaiyuan.me/2017/09/22/memory-barrier/
http://dreamrunner.org/blog/2014/06/28/qian-tan-memory-reordering/
https://blog.csdn.net/hairetz/article/details/18233935
http://chenyufei.info/blog/2013-03-10/memory-consistency-model/
http://www.rdrop.com/users/paulmck/scalability/paper/whymb.2010.07.23a.pdf
https://www.cnblogs.com/gaochundong/p/lock_free_programming.html
https://preshing.com/20120612/an-introduction-to-lock-free-programming/
https://github.com/kumpera/Lock-free-hastable
https://github.com/xant/libhl

http://valleylord.github.io/post/201606-memory-model/
https://blog.csdn.net/dd864140130/article/details/56494925

https://tour.golang.org/concurrency/2


在C中实现无锁编程需要考量的东西
https://www.reddit.com/r/C_Programming/comments/6k1ix6/how_to_write_a_lock_free_queue_in_c/
http://codemacro.com/2015/05/03/hazard-pointer/
https://www.schneems.com/2017/06/28/how-to-write-a-lock-free-queue/
https://github.com/mulle-concurrent/mulle-concurrent
-->

## 参考

[Implementing Lock-Free Queues](http://people.cs.pitt.edu/~jacklange/teaching/cs2510-f12/papers/implementing_lock_free.pdf) John D. Valois 在 1994.10 拉斯维加斯的并行和分布系统系统国际大会上的一篇论文及其相关的实现 [Github](https://github.com/supermartian/lockfree-queue) 。

[Fast Wait Free Queue](https://github.com/chaoran/fast-wait-free-queue) 除了无锁编程还包括了压测。

{% highlight text %}
{% endhighlight %}
