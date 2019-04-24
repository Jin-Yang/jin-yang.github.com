---
title: GoLang Echo 简介
layout: post
comments: true
language: chinese
category: [program,golang,linux]
keywords: echo,golang,http
description:
---


<!-- more -->

![echo introduce]({{ site.url }}/images/go/echo-introduce.png "echo introduce"){: .pull-center width="80%" }


## 路由

Echo 的路由基于 Radix Tree ，它让路由的查询非常快，而且路由使用了 Sync Pool 来重复利用内存并且几乎达到了零内存占用。

## 中间件

中间件是一个函数，嵌入在 HTTP 的请求和响应之间，可以获得 Echo#Context 对象用来进行一些特殊的操作， 比如记录每个请求或者统计请求数。

Action的处理在所有的中间件运行完成之后。

### Pre

`Pre()` 用于注册一个在路由执行之前运行的中间件，可以用来修改请求的一些属性，比如在请求路径结尾添加或者删除一个 `/` 来使之能与路由匹配。

<!--
下面的这几个内建中间件应该被注册在这一级别：

AddTrailingSlash
RemoveTrailingSlash
MethodOverride
-->

注意: 由于在这个级别路由还没有执行，所以这个级别的中间件不能调用任何 `echo.Context` 的 API。


## 参考

官网 [echo.labstack.com](https://echo.labstack.com/) 有很详细的示例以及文档，也可以参考中文文档 [go-echo.org](http://go-echo.org/)。

<!--
使用 FastHTTP 作为底层的 HTTP 处理，速度要快很多
https://github.com/webx-top/echo
-->

{% highlight text %}
{% endhighlight %}
