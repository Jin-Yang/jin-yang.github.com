---
title: OpenSSL 常用命令
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:
---

OpenSSL 除了提供一个开发库之外，还包括了一些常用的命令，这里简单介绍其使用方式。

<!-- more -->

## 安装

一般在 Linux 发行版本中已经安装了 OpenSSL ，可以通过 `openssl version` 查看其版本号，通过 `-a` 参数查看详细的编译参数；也可以通过如下方式从源码安装：

{% highlight text %}
$ ./config -fPIC no-shared --prefix=/usr/local --openssldir=/usr/local/ssl
$ make
# make install
{% endhighlight %}

其中各个参数介绍如下：

* `shared` 生成动态库，默认会生成静态库；
* `no-shared` 只生成静态库，也就是生成 `libssl.a` 以及 `libcrypto.a` 两个库文件；
* `-fPIC` 生成位置无关代码，用来防止在静态库连接到动态库的时候报错；
* `--openssldir=OPENSSLDIR` 安装目录，默认是 `/usr/local/ssl`；
* `--prefix=PREFIX` 设置 `lib` `include` `bin` 目录的前缀，默认为 `OPENSSLDIR` 目录。

## 生成证书

这里采用自签发的证书，需要生成根证书，并分别向服务端和客户端颁发对应的证书，这样可以作双向认证。

{% highlight text %}
mkdir pki/{CA,SVR,CLI} -p
{% endhighlight %}

子签CA证书。

{% highlight text %}
----- 生成根证书私钥 pem
openssl genrsa -out cakey.pem 2048
----- 生成根证书签发申请文件 csr
openssl req -new -key cakey.pem -out ca.csr    \
	-subj "/C=CN/ST=MyProvince/L=MyCity/O=MyOrganization/OU=MyGroup/CN=MyCA"
----- 自签发根证书 cer
openssl x509 -req -days 3650 -sha1 -extensions v3_ca -signkey cakey.pem -in ca.csr -out cacert.pem
{% endhighlight %}

服务端私钥和证书。

{% highlight text %}
----- 生成服务端私钥
openssl genrsa -out key.pem 2048
----- 生成证书请求文件
openssl req -new -key key.pem -out server.csr  \
	-subj "/C=CN/ST=MyProvince/L=MyCity/O=MyOrganization/OU=MyGroup/CN=MyServer"
----- 使用根证书签发服务端证书
openssl x509 -req -days 365 -sha1 -extensions v3_req -CA ../CA/cacert.pem     \
	-CAkey ../CA/cakey.pem -CAserial ca.srl -CAcreateserial -in server.csr -out cert.pem
----- 使用CA证书验证server端证书
openssl verify -CAfile ../CA/cacert.pem cert.pem
{% endhighlight %}


客户端私钥和证书。

{% highlight text %}
----- 生成客户端私钥
openssl genrsa -out key.pem 2048
----- 生成证书请求文件
openssl req -new -key key.pem -out client.csr  \
	-subj "/C=CN/ST=MyProvince/L=MyCity/O=MyOrganization/OU=MyGroup/CN=MyClient"
----- 使用根证书签发客户端证书
openssl x509 -req -days 365 -sha1 -extensions v3_req -CA ../CA/cacert.pem     \
	-CAkey ../CA/cakey.pem -CAserial ../SVR/ca.srl -in client.csr -out cert.pem
----- 使用CA证书验证客户端证书
openssl verify -CAfile ../CA/cacert.pem cert.pem
{% endhighlight %}

也可以对私钥进行加密。

{% highlight text %}
----- 通过AES256解密保护私钥
openssl genrsa -aes256 -out keysec.pem -passout pass:123456 2048
openssl req -new -key keysec.pem -out clientsec.csr   \
	-subj "/C=CN/ST=MyProvince/L=MyCity/O=MyOrganization/OU=MyGroup/CN=MyClient" -passin pass:123456
openssl x509 -req -days 365 -sha1 -extensions v3_req -CA ../CA/cacert.pem     \
	-CAkey ../CA/cakey.pem -CAserial ../SVR/ca.srl -in clientsec.csr -out certsec.pem -passin pass:123456

----- 去除私钥中的密码保护
openssl rsa -in pki/CLI/keysec.pem -out pki/CLI/keyplain.pem -passin pass:123456
{% endhighlight %}

其它。

{% highlight text %}
----- 查看证书的内容
openssl x509 -in SVR/cert.pem -text -noout
{% endhighlight %}

<!--
https://ningyu1.github.io/site/post/51-ssl-cert/
吊销证书：$ openssl ca -revoke cert.pem -config openssl.cnf
证书吊销列表：$ openssl ca -gencrl -out cacert.crl -config openssl.cnf
查看列表内容：$ openssl crl -in cacert.crl -text -noout
-->

## 服务端 客户端

OpenSSL 提供了 Server 和 Client 的相关工具，可以用来模拟客户端以及服务端进行测试。

{% highlight text %}
openssl s_server -accept 44330 -CAfile pki/CA/cacert.pem            \
	-key pki/SVR/key.pem -cert pki/SVR/cert.pem -state
openssl s_client -connect 127.0.0.1:44330 -CAfile pki/CA/cacert.pem \
	-key pki/CLI/key.pem -cert pki/CLI/cert.pem -state
{% endhighlight %}

如下是常用的参数。

{% highlight text %}
-connect          指定服务器的地址以及端口，默认是localhost:443
-key              私钥文件的路径
-cert             证书文件的路径
-CAfile           根证书文件的路径
-showcerts        显示服务器的证书信息
-state            在SSL交互过程中的各种信息
-verify           根证书校验的深度
-debug            打印调试信息
-accept           监听的端口号
-ciphersuites     指定TLSv1.3版本的安全套件，通过:分割
-tls1_3           明确使用TLSv1.3版本，或者tls1_2 tls1_1 tls1
-num_tickets <N>  在TLSv1.3的Server中Post-Handshake阶段发送几个tickets
-reconnect <N>    尝试重新连接，可以用来测试会话复用
{% endhighlight %}

### 常用命令




{% highlight text %}
{% endhighlight %}
