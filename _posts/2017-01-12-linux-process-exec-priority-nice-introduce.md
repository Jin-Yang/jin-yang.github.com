---
title: Linux 进程优先级
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---


<!-- more -->


## 优先级

静态优先级和 nice 值的区别。

只有 nice 值对用户可见，而静态优先级则隐藏在内核中，用户可以通过修改 nice 值间接修改静态优先级，而且只会影响静态优先级，不会影响动态优先级。对于普通进程来说，动态优先级是基于静态优先级算出来的。

静态优先级在进程描述符中为 `static_prio` 成员变量，该优先级不会随着时间而改变，内核不会修改它，只能通过系统调用 `nice` 去调整。

nice值是每个进程的一个属性，不是进程的优先级，而是一个能影响优先级的数字；取值范围为-20~19，默认为0。现在内核不再存储 nice 值，而是存储静态优先级 static_prio，两者在内核中通过两个宏进行转换。

{% highlight c %}
#define NICE_TO_PRIO(nice)  (MAX_RT_PRIO + (nice) + 20)
#define PRIO_TO_NICE(prio)  ((prio) - MAX_RT_PRIO - 20)
#define TASK_NICE(p)        PRIO_TO_NICE((p)->static_prio)
{% endhighlight %}

上述的宏存在于 `kernel/sched/sched.h` 中，静态优先级的取值范围为 `[MAX_RT_PRIO, MAX_PRIO-1]` 也即 `[100, 139]` 。

动态优先级的值影响任务的调度顺序，调度程序通过增加或减少进程静态优先级的值来奖励 IO 消耗型进程或惩罚 CPU 消耗型进程，调整后的优先级称为动态优先级，在进程描述符中用 prio 表示，通常所说的优先级指的是动态优先级。

动态优先级的取值范围为 `[0, MAX_PRIO-1]` 也即 `[0, 139]`，其中 `[0, MAX_RT_PRIO-1]` 也即 `[0, 99]` 为实时进程范围，数值越大表示优先级越小。

优先级可以通过 `top`、`ps -o pid,comm,nice -p PID` 或者 `ps -el` 查看，如果是实时进程就是静态优先级，如果是非实时进程，就是动态优先级。

{% highlight text %}
# nice -n 10 commands               # 指定启动时的优先级为10
# nice -10 commands                 # 同上
# nice --10 commands                # 指定优先级为-10
# renice 10 -p PID                  # 设置已经启动进程的优先级，只有root可以设置为负值
# renice -10 PID                    # 设置为-10
{% endhighlight %}

可以修改 `/etc/security/limits.conf` 的值，指定特定用户的优先级 `[username] [hard|soft] priority [nice value]` 。

<!--
 prio和normal_prio为动态优先级，static_prio为静态优先级。static_prio是进程创建时分配的优先级，如果不人为的更 改，那么在这个进程运行期间不会发生变化。 normal_prio是基于static_prio和调度策略计算出的优先级。prio是调度器类考虑的优先级，某些情况下需要暂时提高进程的优先级 (实时互斥量)，因此有此变量，对于优先级未动态提高的进程来说这三个值是相等的。以上三个优先级值越小，代表进程的优先级有高。一般情况下子进程的静态 优先级继承自父进程，子进程的prio继承自父进程的normal_prio。
    rt_policy表示实时进程的优先级，范围为0～99，该值与prio，normal_prio和static_prio不同，值越大代表实时进程的优先级越高。
    那么内核如何处理这些优先级之间的关系呢？其实，内核使用0～139表示内部优先级，值越低优先级越高。其中0～99为实时进程，100～139为非实时进程。
    当static_prio分配好后，prio和normal_prio计算方法实现如下：
    首先，大家都知道进程创建过程中do_fork会调用wake_up_new_task,在该函数中会调用static int effective_prio(struct task_struct *p)函数。
    void fastcall wake_up_new_task(struct task_struct *p, unsigned long clone_flags)
    {
            unsigned long flags;
               struct rq *rq;
        ...
            p->prio = effective_prio(p);
        ...
    }
    static int effective_prio(struct task_struct *p)函数的实现如下：
    static int effective_prio(struct task_struct *p)
    {
            p->normal_prio = normal_prio(p);
                /*
             * If we are RT tasks or we were boosted to RT priority,
             * keep the priority unchanged. Otherwise, update priority
             * to the normal priority:
             */
            if (!rt_prio(p->prio))
                    return p->normal_prio;
            return p->prio;
    }
    在函数中设置了normal_prio的值，返回值有设置了prio，真是一箭双雕，对于实时进程需要特殊处理，总结主要涉及非实时进进程，就对实时进程的处理方法不解释了。
    static inline int normal_prio(struct task_struct *p)的实现如下：
        static inline int normal_prio(struct task_struct *p)
    {
            int prio;

            if (task_has_rt_policy(p))
                    prio = MAX_RT_PRIO-1 - p->rt_priority;
            else
                    prio = __normal_prio(p);
            return prio;
    }
    对于普通进程会调用static inline int __normal_prio(struct task_struct *p)函数。
    static inline int __normal_prio(struct task_struct *p)函数的实现如下：
        static inline int __normal_prio(struct task_struct *p)
    {
            return p->static_prio;
    }
    这样大家应该很清楚了，对于非实时进程prio，normal_prio和static_prio是一样的，但是也有特殊情况，当使用实时互斥量时prio会暂时发生变化。



普通进程的优先级通过一个关于静态优先级和进程交互性函数关系计算得到。随实际任务的实际运行情况得到。实时优先级和它的实时优先级成线性，不随进程的运行而改变。

3、实时优先级：

实时优先级只对实时进程有意义。在进程描述符rt_priority中。取值0~MAX_RT_PRIO-1。

    prio=MAX_RT_PRIO-1 – rt_priority
    时间片：

    在完全公平调度器CFS融入内核之前，时间片是各种调度器的一个重要的概念。它指定了进程在被抢占之前所能运行的时间。调用器的一个重要目标便是有效的分配时间片，以便提供良好的用户体验。时间片分的过长会导致交互式进程响应不佳。时间片分的过长会导致进程切换带来的消耗。为了解决这个矛盾内核采用了：

    1、提高交互进程的优先级，同时分配默认的时间片

    2、不需要进程一次性用完时间片，可多次使用。

    高的优先级可保证交互进程的频繁调用，长的时间片可保证它们可长时间处于可执行状态

           实时进程优先级：

                实时优先级分为SCHED_FIFO,SCHED_RR两类，有软实时硬实时之分，FIFO/RR 都有动态优先级，没有静态优先级。内核提供的修改优先级的函数，一般是修改rt_priority的值。rt_priority的取值范围[1,99]。

                     prio  = MAX_RT_PRIO – 1 – rt_priority     其中MAX_RT_PRIO = 100

http://www.cnblogs.com/zhaoyl/archive/2012/09/04/2671156.html
-->

## 查看

在内核中，通过 `0~139` 来标示进程的优先级，其中 `0~99` 标识实时优先级，而 `100~139` 用于用户的进程，对应了 nice 的取值范围 `-20~19` 值越小优先级越高。

{% highlight text %}
----- 可以通过普通用户降低优先级
$ nice -n 10 sleep 1000

----- 如果要提高优先级需要root权限
$ nice -n -10 sleep 1000
{% endhighlight %}

另外，通过 top 的 PR  会显示 `PR=20+NI`，而对于 ps 的参数则会比较复杂。

在 ps 中，会通过 `PRI` 或者 `BAZ` 列显示优先级，不过会根据参数不同，而显示的值也有所区别：A) `priority` 也就是 `/proc/<PID>/stat` 中的第 18 列；B) `pri` 计算方法为 `39-priority`；C) `pri_baz` 真实的优先级。

而 `/proc/<PID>/stat` 中的第 18 列，实际上是 `task_struct->prio - 100` 的值，可以参考 `fs/proc/array.c` 文件。



{% highlight text %}
{% endhighlight %}
