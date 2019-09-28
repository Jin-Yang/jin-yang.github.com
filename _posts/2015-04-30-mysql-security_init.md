---
Date: October 19, 2013
title: MySQL 安全设置
layout: post
comments: true
language: chinese
category: [mysql,database]
---

很多时候在部署一种产品时，安全性往往是最后考虑的，不过这也带来了很大的风险。

<!-- more -->


## 权限控制

通常数据库只需要本地网络访问，所以完全没有必要将权限开放给网络。

{% highlight text %}
mysql> GRANT ALL ON *.* TO 'user'@'%';               ← 可能会存在风险
mysql> GRANT ALL ON *.* TO 'user'@'192.168.9.128';   ← 需要指定IP
{% endhighlight %}

一定要设置 root 密码，通常可以设置成只能本地访问；最好可以将 root 修改为其它名称。

{% highlight text %}
mysql> SET PASSWORD FOR 'root'@'localhost' = PASSWORD('new-passowrd');
mysql> UPDATE mysql.user SET user='foobar' WHERE user='root';
mysql> FLUSH PRIVILEGES;
{% endhighlight %}

对于系统文件，其中数据文件最好使用 mysql:mysql 用户，其中需要确保只有 mysql 和 root 可以访问；对于二进制文件，同样需要设置。




## 禁用 LOCAL INFILE

用于防止非授权用户访问本地文件，可以通过如下方式查看。

{% highlight text %}
mysql> LOAD DATA LOCAL INFILE '/etc/passwd' INTO TABLE tbl;
mysql> SELECT load_file('/etc/passwd');
{% endhighlight %}

此时，需要在配置文件中添加如下内容。

{% highlight text %}
[mysqld]
set-variable=local-infile=0
{% endhighlight %}




<!--
http://bobao.360.cn/learning/detail/436.html
http://www.ha97.com/4092.html
-->

{% highlight text %}
{% endhighlight %}
