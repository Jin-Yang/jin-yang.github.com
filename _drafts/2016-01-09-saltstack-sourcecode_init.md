---
Date: October 19, 2013
title: SaltStack 源码解析
layout: post
comments: true
language: chinese
category: [misc]
---


<!-- more -->



对于 master 可以通过 salt-master -d 启动。


实际上执行的是 scripts.py:salt_master() 函数。

{% highlight text %}
salt_master()@scripts.py      #
{% endhighlight %}

命令是通过 salt 客户端发送的，实际上对应源码中的 class LocalClient，会将请求通过 4506 发送到 master，然后监听 master_event_pub.ipc 等待返回的消息。


# 执行流程

假设通过 salt 执行如下命令。

{% highlight text %}
# salt '*' test.ping
{% endhighlight %}

### 1. LocalClient

salt 命令会通过 LocalClient 生成一个请求，然后将请求发送到 master 中 ReqServer 的 TCP:4506 端口。


### 2. ReqServer

在 salt-master 中的 ReqServer 发现上述的请求，并通过 workers.ipc 将请求发送给可用的 MWorker 。

### 3. MWorker

其中的一个 worker 会处理上述发送的请求，首先会检查请求的用户是否有权限，然后通过 ClearFuncs.publish() 将请求发送所有连接的 minions 。



# Client


![SaltStack Commands Syntax]({{ site.url }}/images/network/saltstack/cmd-syntax.png "SaltStack Commands Syntax"){: .pull-center}

{% highlight text %}
{% endhighlight %}
