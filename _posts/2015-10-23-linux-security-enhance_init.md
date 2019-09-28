---
Date: Auguest 05, 2015
title: Linux 安全加固
layout: post
comments: true
language: chinese
category: [linux, network]
---


<!-- more -->

# sshd

修改配置文件 /etc/ssh/sshd_config 。

{% highlight text %}
# 禁止root用户登陆
PermitRootLog no
{% endhighlight %}

3次登录密码错误，锁定账户5分钟：
vi /etc/pam.d/sshd
增加
auth     required pam_tally.so deny=3 unlock_time=5


可以直接通过 /etc/motd 该文件，设置登陆提示。


查看用户登陆历史 last -x 。

opasswd




改历史




{% highlight text %}
{% endhighlight %}
