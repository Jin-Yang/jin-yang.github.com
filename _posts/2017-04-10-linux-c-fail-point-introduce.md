---
title: Linux Fail Points
layout: post
comments: true
language: chinese
category: [misc]
keywords: linux,inline,assembly
description: 对于一些异常一般很难构建，尤其是像系统 API 调用、硬件故障等等，通过 FailPoints 可以在指定的代码段注入一些异常处理。实际上，BSD 已经提供了一套相关的 FailPoints 机制。
---

对于一些异常一般很难构建，尤其是像系统 API 调用、硬件故障等等，通过 FailPoints 可以在指定的代码段注入一些异常处理。

实际上，BSD 已经提供了一套相关的 FailPoints 机制。

<!-- more -->

## BSD

首先，通过一个示例介绍如何使用。

假设有如下的函数，然后需要修改 `DO STUFF` 中的内容，那么应该如何进行测试。

{% highlight c %}
rc = func(foo, bar, blatz);
if (rc) {
	/* DO STUFF */
}
{% endhighlight %}

在原有的代码处添加一个 FailPoint ，如下。

{% highlight c %}
#include <sys/fail.h>

rc = func(foo, bar, blatz);
KFAIL_POINT_CODE(FP_KERN, myfailpoint, error = RETURN_VALUE);
if (rc) {
	/* DO STUFF */
}
{% endhighlight %}

然后可以通过 `sysctl` 处理。

{% highlight text %}
----- 默认是关闭
# sysctl fail_point.kern.myfailpoint
fail_point.kern.myfailpoint: off

----- 设置异常场景
# sysctl fail_point.kern.myfailpoint=".1%return(5)"
fail_point.kern.myfailpoint: off -> .1%return(5)
{% endhighlight %}

也就是在执行到对应的代码块之后，会有 `0.1%` 的概率返回 sysctl 指定的值。

其它的操作还有。

{% highlight text %}
1%sleep(100)    # sleep for 100ms
1%panic()       # panic immediately
1%break()       # break to debugger
1%print()       # print to console
{% endhighlight %}

<!--
https://www.freebsd.org/cgi/man.cgi?query=fail
https://www.bsdcan.org/2009/schedule/attachments/113_ZachLoafman.pdf
https://github.com/freebsd/freebsd/blob/master/sys/kern/kern_fail.c
-->

{% highlight text %}
{% endhighlight %}
