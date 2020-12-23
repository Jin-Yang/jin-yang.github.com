---
title: Python2 VS. Python3
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: linux
description:
---


<!-- more -->

## 函数

### xrange()

报错信息为 `NameError:name 'xrange' is not defined` ，其原因为 Python3 中将 `range()` 和 `xrange()` 函数合并为 `range()` 。

所以，转为 Python3 时，将 `xrange()` 替换为 `range()` 即可。


{% highlight text %}
{% endhighlight %}
