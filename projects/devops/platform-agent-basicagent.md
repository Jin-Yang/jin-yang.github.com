---
title: BasicAgent 实现
layout: project
comments: true
language: chinese
category: [misc]
keywords:
description:
---


## 启动流程

{% highlight text %}
1. 通过 DNS 解析获取 config.cargo.com 的 IP 地址信息，会依次尝试建立链接。
   可以先通过/etc/hosts配置，通过多行将该域名设置多个 IP 地址
       127.0.0.1   config.cargo.com
       127.0.0.2   config.cargo.com
       127.0.0.3   config.cargo.com

2. 发送初始化报文，同时可以根据上层策略决定是否重定向进行负载均衡。

3. 建立TLS链接。
{% endhighlight %}

### 0.1 初始化报文

{% highlight text %}
-----> 请求报文
{
    "method": "init",                                      # 必选
    "hostname": "edd6c192-52cb-4133-a17a-e7d8aec03de7",    # 必选，指定同步命令类型
}

<----- 响应报文
{
    "method": "redirect",                  # 必选
    "server": "127.0.10.1,10.92.1.130",    # 必选，重定向的地址
}
{% endhighlight %}



{% highlight text %}
./boot -L /tmp/booter.log -P /tmp/booter.pid -f
{% endhighlight %}

## 命令通道

用于在服务器上执行命令。

### 同步命令

{% highlight text %}
-----> 请求报文
{
    "id": "xxxxxx",
    "method": "sync.bash",    # 必选，指定同步命令类型
    "cmd": "ls",              # 必选，需要执行的命令
    "timeout": 10,            # 可选，超时时间，默认120，最小1s
    "user": "root",           # 可选，执行用户，默认root
    "group": "root"           # 可选，执行用户组，默认root
}

<---- 响应报文，成功
{
    "id": "xxxxxxx",
    "resp": "success",
    "retcode": 2,
    "message": "Normal termination"
    "data": "xxxx"
}
<---- 响应报文，失败
{
    "id": "xxxxxxx",
    "resp": "failed",
    "retcode": 3021,
    "message": "Invalid params"
}
{% endhighlight %}

### 异步任务


## 查询接口

提供通用接口获取主机信息，例如内核信息、用户信息等。

{% highlight text %}
-----> 请求报文
{
    "id": "xxxxxx",
    "method": "getinfo.users",  # 必选
    "args": "",                 # 可选，不同方法参数不同
}

<---- 响应报文，成功
{
    "id": "xxxxxxx",
    "resp": "success",
    "retcode": 2,
    "message": "Normal termination"
    "data": "xxxx"
}
<---- 响应报文，失败
{
    "id": "xxxxxxx",
    "resp": "failed",
    "retcode": 3021,
    "message": "Invalid params"
}
{% endhighlight %}

## 命令行

可以通过命令行直接加载动态库获取数据。

{% highlight text %}
BasicAgentCtl [options]
参数：
    -c <command>    指定命令，例如getinfo.kernel、getinfo.users等；
    -a <arguments>  上述命令的参数，不同的指标参数略有区别；

===> getinfo.users 获取用户信息，注意执行需要ROOT权限
./BasicAgentCtl -c "getinfo.users" -a "-U root" | python -m json.tool  # 指定用户
./BasicAgentCtl -c "getinfo.users" -a "-A" | python -m json.tool  # 所有
./BasicAgentCtl -c "getinfo.users" -a "-u" | python -m json.tool  # 普通用户
./BasicAgentCtl -c "getinfo.users" -a "-S" | python -m json.tool  # 系统用户

===> getinfo.kernel 获取内核信息
./BasicAgentCtl -c "getinfo.kernel" | python -m json.tool
{% endhighlight %}


{% highlight text %}
{% endhighlight %}
