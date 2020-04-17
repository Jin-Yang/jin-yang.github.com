---
title: C++ 互斥量
layout: post
comments: true
language: chinese
category: [linux]
keywords:
description:
---


<!-- more -->

## 简介

C++ 11 中的互斥量保存在 `<mutex>` 头文件中，

* `std::mutex` 基本的互斥锁，成员函数包括了 `lock()` `unlock()` `try_lock()` 。
* `std::recursive_mutex` 递归锁，可防止同一线程中多次调用发生死锁，也就是允许多次调用。
* `std::time_mutex` 带有定时器，新增 `try_lock_for()` `try_lock_until()` 接口用于定时。
* `std::recursive_timed_mutex` 定时递归。

相比最基础的互斥量，同时增加了递归、定时互斥量，

### RAII

Resource Acquisition Is Initialization, RAII 由 C++ 之父 Bjarne Stroustrup 提出，也就是资源获取即初始化。在 C++ 中，实际上会利用栈的能力，对于锁来说，在初始化的时候获取锁，然后在退出时类似调用析构释放锁，从而可以降低发生死锁的风险。

C++11 提供了两种：A) `std::lock_guard` 简单使用；B) `std::unique_lock` 更高级的方法。

#### lock_guard

通过一局部变量，当函数退出的时候，变量会同时被销毁，也就是在销毁的时候同时会将锁释放。

{% highlight cpp %}
#include <mutex>
#include <thread>
#include <iostream>
#include <stdexcept>

std::mutex mtx;

void dump_thread(void)
{
        try {
                // local lock_guard to guarante unlocking on exception.
                std::lock_guard<std::mutex> lock(mtx);
                throw(std::logic_error("some error"));
        } catch (std::logic_error &) {
                std::cout << "exception" << std::endl;
        }
}

int main(void)
{
        std::thread thd(dump_thread);
        thd.join();

        return 0;
}
{% endhighlight %}

#### unique_lock

通过互斥锁将原有的并发操作修改成了串行操作，为了保证正确性的前提下，同时提高并发，需要尽量降低加锁的粒度，而 `lock_guard` 只能在析构的时候解锁，不够灵活。

`unique_lock` 提供了更丰富的接口，但因为要维护内部的状态，如果在析构中锁已经释放，那么就不会再调用解锁，所以相比来说性能会有所损耗。

{% highlight cpp %}
#include <mutex>
#include <thread>
#include <iostream>

std::mutex mtx;

void dump_thread(void)
{
        std::unique_lock<std::mutex> lock(mtx, std::defer_lock);

        std::cout << "Before lock it" << std::endl;
        lock.lock();
        std::cout << "Step #1" << std::endl;
        lock.unlock();

        std::cout << "Step #2" << std::endl;

        lock.lock();
        std::cout << "Step #3" << std::endl;
        lock.unlock();
}

int main(void)
{
        std::thread thd(dump_thread);
        thd.join();

        return 0;
}
{% endhighlight %}

如上的示例中，如果要使用 `unique_lock` 则需要有两个代码块完成。通过 `std::defer_lock` 声明在初始化的时候不加锁。

<!--
https://www.cnblogs.com/haippy/p/3237213.html
-->

{% highlight text %}
{% endhighlight %}
