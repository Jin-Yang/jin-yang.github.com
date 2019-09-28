---
Date: October 19, 2013
title: Apache 简介
layout: post
comments: true
language: chinese
category: [webserver]
---

Apache 是北美印第安人的一个部落，叫阿帕奇族，在美国的西南部，在这里是个 web 服务器，同时也是一个基金会的名称、一种武装直升机。

目前 Apache 仍然占有很大的市场份额，是使用最广泛的一种，虽然现在 Nginx 的发展势头正盛。



<!-- more -->

# 简介

Apache 以跨平台、高效和稳定而闻名，它几乎可以运行在所有平台上，可以通过简单的 API 扩充，将 Perl、Python 等解析器编译到服务器中。

目前，它最为诟病的一点就是变得越来越重，被普遍认为是重量级的 WebServer，这也就导致了像 lighttpd、nginx 这种轻量级的替代产品，而且也越来越成熟。





Apache 基于模块化设计，其核心代码并不多，功能都被分散到各个模块中，而各个模块在系统启动的时候按需载入，其中主要的代码在 server/main.c 中。




# MPM, Multi-Processing Modules



MPM 是 Apache 的核心组件之一，该模块用于对进程和线程池进行管理，在编译的时候必须选择其中的一个，对于 *nix 来说可以选择任意的一个。

目前支持支持三种的 MPM，分别是：Prefork、Worker、Event 三种，可以通过 httpd -V 查看当前支持的模式。

## Prefork MPM


## Worker MPM


## Event MPM

# APR, Apache portable Run-time libraries



[httpd.apache.org](http://httpd.apache.org/)




# Virtual Hosts

WEB 服务实际上就是访问某个 IP 主机上的某个端口，这也就意味着可以通过访问不同的域名或者端口实现对不同网站的访问，此时就需要设置虚拟主机，目前通常有三种方式。


## 基于 IP

假设服务器当前的 IP 地址为 192.168.1.100，通过 ifconfig 在同一个网络接口 eth0 上绑定 3 个 IP 。

{% highlight text %}
----- 添加3个IP地址
# ifconfig eth0:1 192.168.1.101
# ifconfig eth0:2 192.168.1.102
# ifconfig eth0:3 192.168.1.103

----- 修改/etc/hosts文件，绑定三个域名的IP地址
# cat /etc/hosts
192.168.1.101   www.foobar1.com
192.168.1.102   www.foobar2.com
192.168.1.103   www.foobar3.com

----- 新建三个独立的目录，以及不同内容的index.html
{% endhighlight %}


 3. 建立虚拟主机存放网页的根目录，如在/www目录下建立test1、test2、test3文件夹，其中分别存放1.html、2.html、3.html
/www/test1/1.html
/www/test2/2.html
/www/test3/3.html

 4. 在httpd.conf中将附加配置文件httpd-vhosts.conf包含进来，接着在httpd-vhosts.conf中写入如下配置：

<VirtualHost 192.168.1.11:80>
　　ServerName www.test1.com
　　DocumentRoot /www/test1/
　　<Directory "/www/test1">
 　　　　Options Indexes FollowSymLinks
　　　　 AllowOverride None
　　　　 Order allow,deny
　　 　　Allow From All
 　 </Directory>
</VirtualHost>

<VirtualHost 192.168.1.12:80>
　　ServerName www.test1.com
　　DocumentRoot /www/test2/
　　<Directory "/www/test2">
 　　　　Options Indexes FollowSymLinks
　　　　 AllowOverride None
　　　　 Order allow,deny
　　 　　Allow From All
 　 </Directory>
</VirtualHost>

<VirtualHost 192.168.1.13:80>
　　ServerName www.test1.com
　　DocumentRoot /www/test3/
　　<Directory "/www/test3">
 　　　　Options Indexes FollowSymLinks
　　　　 AllowOverride None
　　　　 Order allow,deny
　　 　　Allow From All
 　 </Directory>
</VirtualHost>

复制代码

 5. 大功告成，测试下每个虚拟主机，分别访问www.test1.com、www.test2.com、www.test3.com



二、基于主机名

 1. 设置域名映射同一个IP，修改hosts：
192.168.1.10  www.test1.com
192.168.1.10  www.test2.com
192.168.1.10  www.test3.com

 2. 跟上面一样，建立虚拟主机存放网页的根目录
/www/test1/1.html
/www/test2/2.html
/www/test3/3.html

 3. 在httpd.conf中将附加配置文件httpd-vhosts.conf包含进来，接着在httpd-vhosts.conf中写入如下配置：


　　为了使用基于域名的虚拟主机，必须指定服务器IP地址（和可能的端口）来使主机接受请求。可以用NameVirtualHost指令来进行配置。 如果服务器上所有的IP地址都会用到， 你可以用*作为NameVirtualHost的参数。在NameVirtualHost指令中指明IP地址并不会使服务器自动侦听那个IP地址。 这里设定的IP地址必须对应服务器上的一个网络接口。

　　下一步就是为你建立的每个虚拟主机设定<VirtualHost>配置块，<VirtualHost>的参数与NameVirtualHost指令的参数是一样的。每个<VirtualHost>定义块中，至少都会有一个ServerName指令来指定伺服哪个主机和一个DocumentRoot指令来说明这个主机的内容存在于文件系统的什么地方。

　　如果在现有的web服务器上增加虚拟主机，必须也为现存的主机建造一个<VirtualHost>定义块。其中ServerName和DocumentRoot所包含的内容应该与全局的保持一致，且要放在配置文件的最前面，扮演默认主机的角色。
复制代码
NameVirtualHost *:80
<VirtualHost *:80>

　　ServerName *

　　DocumentRoot /www/

</VirtualHost>

<VirtualHost *:80>

　　ServerName www.test1.com

　　DocumentRoot /www/test1/

　　<Directory "/www/test1">

　　　　Options Indexes FollowSymLinks

　　　　AllowOverride None

　　　　Order allow,deny

　　　　Allow from all

　　</Directory>

</VirtualHost>



<VirtualHost *:80>

　　ServerName www.test2.com

　　DocumentRoot /www/test2/

　　<Directory "/www/test2">

　　　　Options Indexes FollowSymLinks

　　　　AllowOverride None

　　　　Order allow,deny

　　　　Allow from all

　　</Directory>
</VirtualHost>

<VirtualHost *:80>

　　ServerName www.test3.com

　　DocumentRoot /www/test3/

　　<Directory "/www/test3">

　　　　Options Indexes FollowSymLinks

　　　　AllowOverride None

　　　　Order allow,deny

　　　　Allow from all

　　</Directory>
</VirtualHost>
复制代码

 4. 大功告成，测试下每个虚拟主机，分别访问www.test1.com、www.test2.com、www.test3.com



三、基于端口
1.  修改配置文件

　　将原来的

　　 　Listen 80
      改为
    　　Listen 80
   　　 Listen 8080


2. 更改虚拟主机设置：
复制代码
<VirtualHost 192.168.1.10:80>
    DocumentRoot /var/www/test1/
    ServerName www.test1.com
</VirtualHost>

<VirtualHost 192.168.1.10:8080>
    DocumentRoot /var/www/test2
    ServerName www.test2.com
</VirtualHost>




# mod_wsgi


可以直接参考官网 [modwsgi.readthedocs.io](http://modwsgi.readthedocs.io/en/develop/index.html#)

{% highlight text %}
{% endhighlight %}
