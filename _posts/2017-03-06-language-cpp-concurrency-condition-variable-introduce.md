---
title: C++ 条件变量
layout: post
comments: true
language: chinese
category: [linux]
keywords:
description:
---


<!-- more -->

## 简介

其中 `condition_variable` 对象使用 `unique_lock()` 来锁住当前线程，并通过 `wait()` 函数阻塞等待，直到另外一个线程通过相同的变量唤醒改线程。

注意，默认 `condition_variable` 会搭配 `std::unique_lock<std::mutex>` 使用，如果要使用其它的 `lockable` 类型 (例如 `time_mutex` )，可以使用 `condition_variable_any` 类。

### wait

在 C++ 11 中，提供了两个 `wait()` 函数接口，都会导致当前线程阻塞直到被唤醒，为了防止虚假唤醒发生，同时可以选择一个判断条件。

> 所谓的虚假唤醒 (Spurious Wakeup) 是指即使没有线程调用唤醒函数，原阻塞等待的函数也可能返回，也就是线程被唤醒了但条件不满足，此时如果继续执行就会导致错误。
>
> 在唤醒时是唤醒一个线程还是多个线程没有明确的要求，而且在多核、接收到信号时，都有可能会发生，所以一般会在唤醒之后再次检查条件是否满足。

两个函数的接口声明如下。

{% highlight text %}
void wait(std::unique_lock<std::mutex> &lock);

template<class Predicate>
void wait(std::unique_lock<std::mutex> &lock, Predicate pred);
{% endhighlight %}

其中 `Predicate` 就是用来判断是否满足条件，防止虚假唤醒。

### 使用

与其它的互斥和条件变量的配合相似，在 `wait()` 函数中会自动调用 `unlock()` 释放锁，使其它线程可以继续获取锁；当前线程被唤醒的同时会对互斥量加锁，从而保证互斥量和条件变量的原子性。

{% highlight cpp %}
#include <queue>
#include <mutex>
#include <thread>
#include <iostream>
#include <condition_variable>

#define QUEUE_MAX_SIZE   100

bool quit_now = false;

std::queue<int> queue;
std::mutex queue_lock;
std::condition_variable queue_cond;

void consumer(void)
{
        while (true) {
                std::unique_lock<std::mutex> lock(queue_lock);
                queue_cond.wait(lock, []{return quit_now || queue.size() > 0;});
                if (quit_now && queue.size() <= 0) // consume all data.
                //if (quit_now) // leave now.
                        break;
                std::cout << "Consumer " << queue.front() << std::endl;
                queue.pop();
        }
        std::cout << "Consumer quit." << std::endl;
}

void producer(void)
{
        int count = 0;

        while (count < 5) {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                std::unique_lock<std::mutex> lock(queue_lock);
                if (queue.size() >= QUEUE_MAX_SIZE) {
                        std::cout << "Queue is full now, try later." << std::endl;
                        continue;
                }

                std::cout << "Producer " << count << std::endl;
                queue.push(count++);

                queue_cond.notify_all();
        }
        std::cout << "Producer quit." << std::endl;
        quit_now = true;
        queue_cond.notify_all();
}

int main(void)
{
        std::thread consumers[2], producers[1];

        for (int i = 0; i < sizeof(consumers)/sizeof(consumers[0]); ++i)
                consumers[i] = std::thread(consumer);
        for (int i = 0; i < sizeof(producers)/sizeof(producers[0]); ++i)
                producers[i] = std::thread(producer);

        for (int i = 0; i < sizeof(producers)/sizeof(producers[0]); ++i)
                producers[i].join();
        for (int i = 0; i < sizeof(consumers)/sizeof(consumers[0]); ++i)
                consumers[i].join();

        return 0;
}
{% endhighlight %}

另外，还有 `wait_for()` 等待一段时间后超时，或者通过 `wait_until()` 等待到某个时间点。在唤醒时，可以通过 `notify_one()` 以及 `notify_all()` 分别唤醒一个或者多个线程。

同时还提供了 `notify_all_at_thread_exit()` 方便在线程退出的时候进行通知，不过 `libstdc++` 要在 `GCC 5.0` 之后才支持。

<!--
https://www.cnblogs.com/haippy/p/3252041.html
-->

{% highlight text %}
{% endhighlight %}
