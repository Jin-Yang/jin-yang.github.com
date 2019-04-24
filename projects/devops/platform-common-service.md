---
title: 基础服务
layout: project
comments: true
language: chinese
category: [misc]
keywords:
description:
---

服务端，支持 HTTP 请求。

## 文档服务器搭建

增加 HTTP/FTP 服务器，分别使用 nginx 和 vsftpd 搭建。为了方便使用，可以设置 `/etc/hosts` 增加如下的配置。

{% highlight text %}
127.0.0.1  files.cargo.com
{% endhighlight %}

### 目录结构

其中主要是 Agent 的发布路径，相对于根目录，默认使用的是 `/files` 。

{% highlight text %}
repos/CentOS/7/local/x86_64/RPMS            # 支持CentOS 7的RPM包
repos/CentOS/7/debuginfo/x86_64/RPMS        # 对应的debuginfo包
repos/CentOS/7/src/x86_64/RPMS              # 对应的源码包
{% endhighlight %}

### HTTP 服务

如果要增加新配置，可以直接修改 `/etc/nginx/nginx.conf` 默认配置文件，这里会在 `/etc/nginx/conf.d` 目录下包含了各种配置文件。

然后通过 `nginx -t` 检查语法是否正确，通过 `systemctl start nginx` 启动服务，如果已经启动可以直接通过 `nginx -s reload` 命令重新加载配置。

#### 安装包

在 `/etc/nginx/conf.d` 目录下新增配置文件 `fileserver.conf` ，然后通过 [http://files.cargo.com](http://files.cargo.com) 访问。

{% highlight text %}
server {
	listen 80;                       # 监听端口
	server_name files.cargo.com;     # 如果没有DNS解析，可以设置IP地址
	client_max_body_size 4G;         # 设置最大文件大小
	charset utf-8;                   # 防止出现中文乱码
	root /files;            # 指定相对路径的根目录，如下的location会相对该路径
	location / {                     # 实际存放文件的目录为/files/packages/
		#auth_basic "Restricted"; # 输入密码时的提示语
		#auth_basic_user_file /etc/nginx/pass_file; # 认证时用户密码文件存放路径
		autoindex on;            # 自动生成文件索引
		autoindex_exact_size on; # 显示文件大小
		autoindex_localtime on;  # 显示本地文件时间
	}
}
{% endhighlight %}

对于上述的配置，实际文件保存在 `/files` 目录下，另外需要保证 nginx 启动用户的访问权限，可以通过如下命令进行测试。

{% highlight text %}
$ wget http://files.cargo.com/repos/CentOS/7/local/x86_64/RPMS/MiniAgent-1.2.3-0.x86_64.rpm
{% endhighlight %}

#### 服务器静态文件

用于前端页面的 css js 类型的文件。

{% highlight text %}
# conf.d/bootsvr.conf
upstream bootsvrs {
        ip_hash;
        server 127.0.0.1:8180 max_fails=2 fail_timeout=2;
}

server {
        listen 80;
        server_name aspire.cargo.com;

        location /api/ {
                proxy_pass http://bootsvrs; 
                proxy_set_header X-Real-IP $remote_addr;
        }

        location ~ .*\.(html|htm|gif|jpg|jpeg|bmp|png|ico|txt|js|css)$ {   
                root /file/static/aspire/;
                expires 30d;
        } 
}
{% endhighlight %}


#### 其它

另外，简单的可以采用 python 提供模块，不过只能采用单线程。

{% highlight text %}
----- 启动一个简单的文件服务器
$ python -m SimpleHTTPServer 9000

$ curl http://127.0.0.1:9000
{% endhighlight %}

可以直接通过 [http://127.0.0.1:9000](http://127.0.0.1:9000) 地址访问，可以看到目录下的文件列表。

#### FAQ

##### 404

其中 Nginx 的日志保存在 `/var/log/nginx/{access.log,error.log}`，可以详细查看。


### FTP 服务

直接启动 vsftpd 服务器即可，在 CentOS 中可以通过 `systemctl start vsftpd`，详细可以参考 [FTP 服务简介]({{ production_url }}/post/network-service-ftp.html) 中的介绍，这里简单介绍其常见的配置信息。


### YUM 仓库

为了测试，可以创建一个本地仓库。

{% highlight text %}
----- 1. 创建本地yum仓库目录
$ mkdir -p /files/repos/CentOS/7/local/x86_64/RPMS
$ mkdir -p /files/repos/CentOS/7/debuginfo/x86_64/RPMS

----- 2. 复制文件
$ cp MiniAgent-* /files/repos/CentOS/7/local/x86_64/RPMS
$ cp MiniAgent-debuginfo-* /files/repos/CentOS/7/debuginfo/x86_64/RPMS

----- 3. 创建索引&更新缓存
$ createrepo /files/repos/CentOS/7/local/x86_64
$ createrepo /files/repos/CentOS/7/debuginfo/x86_64
$ yum makecache

----- 4. 创建本地repo文件
$ cat<<-"EOF">/etc/yum.repos.d/CentOS-Local.repo
[local]
name=CentOS-$releasever - local packages for $basearch
baseurl=file:///files/repos/CentOS/$releasever/local/$basearch
enabled=1
gpgcheck=0
protect=1
[local-debuinfo]
name=CentOS-$releasever - local debuginfo for $basearch
baseurl=file:///files/repos/CentOS/$releasever/debuginfo/$basearch
enabled=1
gpgcheck=0
EOF
{% endhighlight %}


## MySQL

这里数据库使用 MySQL 。

{% highlight text %}
----- 安装社区版本MySQL，新增配置
# rpm -Uvh http://dev.mysql.com/get/mysql-community-release-el7-5.noarch.rpm
# yum install mysql-community-server
# cat << EOF > /etc/my.cnf
[client]
password        = new_password
port            = 5506
socket          = /tmp/mysql-5506.sock
[mysqld]
user            = mysql
port            = 5506
socket          = /tmp/mysql-5506.sock
basedir         = /usr
datadir         = /tmp/data-5506
pid-file        = /tmp/data-5506/mysqld.pid
EOF

----- 初始化数据，可通过空白密码登陆
# mkdir /tmp/data-5506
# /usr/sbin/mysqld --initialize-insecure --basedir=/usr --user=mysql \
    --datadir=/tmp/data-5506
# systemctl start mysqld

----- 登陆并修改密码，第一次登陆时密码空白即可，分别对应了5.6以及5.7版本
$ mysql -h 127.1 -p
mysql> UPDATE mysql.user SET password=PASSWORD('new_password') WHERE user='root';
mysql> ALTER USER 'root'@'localhost' IDENTIFIED BY 'new_password';
mysql> FLUSH PRIVILEGES;

mysql> SET autocommit=ON;
mysql> SHOW SESSION VARIABLES LIKE 'autocommit';
mysql> SHOW GLOBAL VARIABLES LIKE 'autocommit';
mysql> SET SESSION autocommit=ON;
mysql> SET GLOBAL autocommit=ON;
{% endhighlight %}


<!--
### 后台服务搭建

这里基于 Flask 和 MySQL 搭建后台服务器。
-->

{% highlight text %}
{% endhighlight %}
