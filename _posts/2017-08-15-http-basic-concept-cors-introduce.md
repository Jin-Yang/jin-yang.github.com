---
title: HTTP CORS 简介
layout: post
comments: true
language: chinese
category: [program,misc]
keywords: 
description:
---

出于安全原因，浏览器会限制发起跨源的 HTTP 请求，包括浏览器限制发起跨站请求，或者跨站请求可以正常发起，但是返回结果被浏览器拦截了。

也就是说 Schema、Host、Port 必须要保持一致，通过跨域资源共享 CORS 可以允许访问来自不同源服务器上的指定的资源。

这里简单介绍。

<!-- more -->

![cors]({{ site.url }}/images/webserver/cors-principle.png "cors"){: .pull-center width="80%" }

## OPTIONS

也就是预检请求，用来检查是否允许跨域请求，通过它的返回来告知浏览器是否允许跨域请求，若不允许则会报 CORS 的错误。

可以返回如下的内容。

{% highlight text %}
Access-Control-Allow-Origin    *
Access-Control-Allow-Headers   X-Requested-With
Access-Control-Allow-Methods   PUT,POST,GET,DELETE,OPTIONS
{% endhighlight %}

<!--
https://developer.mozilla.org/zh-CN/docs/Web/HTTP/Access_control_CORS
https://www.jianshu.com/p/89a377c52b48
-->

{% highlight text %}
{% endhighlight %}
