---
Date: October 19, 2013
title: Ansible 杂项
layout: post
comments: true
language: chinese
category: [webserver]
---



<!-- more -->

# 用户转换

Ansible 2.0 通过 become 插件完成用户的转换，可以参考 [Become (Privilege Escalation)](http://docs.ansible.com/ansible/become.html) ，例如在 inventory 中，可以通过如下方式指定。

{% highlight text %}
webserver ansible_user=manager ansible_ssh_pass=PASSWORD ansible_become=true ansible_become_method=sudo ansible_become_user=root ansible_become_pass=PASSWORD
{% endhighlight %}

假设使用的配置项如下，那么实际执行的命令为。

{% highlight python %}
ansible_become_method=sudo ansible_become_user=root
sudo -H -S -p"passwd" -u root /bin/bash -c "CMD"   # 用户密码

ansible_become_method=su ansible_become_user=root
su root -c "CMD"              # root密码
{% endhighlight %}

关于 inventory 中选项的设置可以参考 [Inventory ansile-doc](http://docs.ansible.com/ansible/intro_inventory.html) 。


<!--
http://www.mutouxiaogui.cn/blog/wp-content/uploads/2015/08/Ansible%20Intro.pdf
-->
{% highlight python %}
{% endhighlight %}
