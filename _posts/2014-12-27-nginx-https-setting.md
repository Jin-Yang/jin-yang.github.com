---
title: Nginx HTTPS 配置
layout: post
comments: true
language: chinese
category: [linux,webserver]
keywords: nginx,https,webserver
description: 简单介绍如何使用 Nginx 搭建 https 服务。
---

简单介绍如何使用 Nginx 搭建 https 服务。

<!-- more -->

## 搭建 HTTPS 服务

在 CentOS 中，默认根目录为 `/usr/share/nginx/html` ，使用域名 `www.foobar.com`，需要添加到 `/etc/hosts` 文件中。

### 1. 确认支持 SSL

通过 ```-V``` 参数查看编译时是否添加了 ```--with-http_ssl_module``` 参数，对于 CentOS 来说，一般是已经安装了的。

### 2. 生成证书

可以通过以下步骤生成一个简单的子签名证书，保存在 `/etc/pki/nginx` 目录下。

#### 2.1 生成自签 CA 证书

{% highlight text %}
----- 生成根证书私钥 pem
openssl genrsa -out cakey.pem 2048
----- 生成根证书签发申请文件 csr
openssl req -new -key cakey.pem -out ca.csr    \
	-subj "/C=CN/ST=MyProvince/L=MyCity/O=MyOrganization/OU=MyGroup/CN=MyCA"
----- 自签发根证书 cer
openssl x509 -req -days 3650 -sha1 -extensions v3_ca -signkey cakey.pem -in ca.csr -out cacert.pem
{% endhighlight %}

#### 2.2 生成服务端私钥和证书

{% highlight text %}
----- 生成服务端私钥
openssl genrsa -out key.pem 2048
----- 生成证书请求文件
openssl req -new -key key.pem -out server.csr  \
	-subj "/C=CN/ST=MyProvince/L=MyCity/O=MyOrganization/OU=MyGroup/CN=*.foobar.com"
----- 使用根证书签发服务端证书
openssl x509 -req -days 365 -sha1 -extensions v3_req -CA cacert.pem     \
	-CAkey cakey.pem -CAserial ca.srl -CAcreateserial -in server.csr -out cert.pem
----- 使用CA证书验证server端证书
openssl verify -CAfile cacert.pem cert.pem
{% endhighlight %}

注意上述的 `CN` 设置，需要匹配访问的网站，可以使用通配符 `*`，不过只能匹配一级，上述可以满足 `www.foobar.com` 但不能满足 `main.www.foobar.com` 域名。

### 3. 修改配置文件

在默认的配置文件 `/etc/nginx/nginx.conf` 文件中，添加如下的内容。

{% highlight text %}
    server {
        listen       80 default_server;
        listen       [::]:80 default_server;
        server_name  www.foobar.com;
        rewrite ^(.*) https://$server_name$1 permanent;
    }

    server {
        listen       443 ssl default_server;
        listen       [::]:443 ssl default_server;
        server_name  www.foobar.com;
        root         /usr/share/nginx/html;

        ssl_certificate "/etc/pki/nginx/cert.pem";
        ssl_certificate_key "/etc/pki/nginx/key.pem";
    }
{% endhighlight %}

会将 `http` 的请求跳转到 `https` 中。

### 4. 重启 Nginx

需要注意的是，如果采用 ```systemctl``` 重启，上述的配置不能保存在 tmp 目录下，因为 systemd 会为进程启动不同的 tmp 目录，从而导致无法查找到上述的配置文件。

这样就可以访问 [https://www.foobar.com](https://www.foobar.com)，注意需要将 ```www.foobar.com``` 添加到 ```/etc/hosts``` 中。另外，还可以加入如下代码实现 80 端口重定向到 443。

{% highlight text %}
server {
    listen 80;
    server_name www.foobar.com;
    rewrite ^(.*) https://$server_name$1 permanent;
}
{% endhighlight %}

不过现在无法识别根证书，在访问时会有些安全提示，对于不同的浏览器可以通过如下方式设置。

#### Firefox

在 `Preference->Privacy&Security->Certificates->Certificate Manager` 选项中导入上述生成的 `cacert.pem` 文件即可，列表中是按照 Organize 排序的。

### 5. 其它

在 2 中的配置中，可设置密钥，此时会在重启 nginx 输入密码，而且此时 systemctl 将会失效。


<!--
## 优化

nginx下https配置的优化点，主要有:

    session ticket
    session id cache
    ocsp stapling
    http KeepAlive
    ECDHE等ciphersuite优化
    openssl 编译优化

https://blog.helong.info/blog/2015/05/08/https-config-optimize-in-nginx/
-->


{% highlight text %}
{% endhighlight %}
