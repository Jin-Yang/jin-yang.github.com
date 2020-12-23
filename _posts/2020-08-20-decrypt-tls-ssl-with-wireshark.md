---
title: 使用 Wireshark 解密 SSL/TLS 流量
layout: post
comments: true
language: chinese
tag: [Linux,DevOps,Network]
keywords: ssl,tls,openssl,wireshark,调试,协议,rsa
description: SSL/TLS 是目前使用最广泛的安全通讯协议，在开发或者问题排查时，可以通过 Wireshark 对通讯过程进行解析。不过对于不同版本、不同加密协议其解密方式有所不同，这里详细介绍常见的调试方式。
---

SSL/TLS 是目前使用最广泛的安全通讯协议，最常见的 HTTPS 也是建立在该协议之上的，尤其是 TLSv1.3 版本更新之后，其性能、安全性得以有效提升，在带来安全性的同时，在开发阶段很难对通讯协议进行调试。

通过 Wireshark 可以分析绝大部分的 TCP/IP 协议流量，包括了 TLS 协议，可以用来分析协议的工作方式或者排查问题。

<!-- more -->

## 简介

可以直接在本地通过 Wireshark 解析报文，不过对于需要分析服务器上的报文的话，建议还是通过 tcpdump 捕获报文，然后再通过 Wireshark 进行分析，这里也是使用这一策略。

注意，TLSv1.3 与 TLSv1.2 的协议差距很大，所以需要确保对应的 Wireshark 版本支持，官方没有明确指出那个版本开始支持，不过基本上 2017.01 之后的版本都会支持，所以，使用最新的即可。

<!--
通过 Wireshark 的 relnotes，没有清晰的说明那个版本开始支持 TLS 1.3，只是在 Wireshark 2.4.4 版本提到 TLS/SSL 版本有过修改。

查看邮件组（日期：2017.01），有关键的引用：

If you want to download a WorkInProgress version of Wireshark that supports TLS1.3 (latest >version of draft -18 only!).   Please go to:
https://www.wireshark.org/download/automated/
THIS IS NOT THE PRODUCTION VERSION OF WIRESHARK!!!

从中可以看出 2017.01 以后的 Wireshark 版本开始支持 TLS 1.3（draft -18）。
-->

### 原理解析

解密的核心过程其实是获得 SessionKey (MasterSecrte)，也就是在密钥交换过程中，如何通过各种元素获取到会话中的密钥。

SessionKey 是通过 `ClientRandom` `ServerRandom` 以及 `PreMasterSecret` 生成，其中，最为关键是 `PreMasterSecret` 的获取，RSA 是由客户端生成，并将其用服务端的公钥加密，然后发送。

而 DH 算法则是由客户端和服务端各自生成，然后不通过网络传输完成信息交换。

#### RSA

如果是通过 RSA 算法交换秘钥，那么客户端会加密并发送给服务端，这样只需要知道服务端的私钥就可以解密出 `PreMasterSecret` 值。

#### DH

而 DH 算法中的 `PreMasterSecret` 是由服务端和客户端各自计算出来，而且没有保存到磁盘，没有通过网络传输，这样就导致无法进行解密。

如果要解密 DH 只能通过类似中间人攻击的方法，或者在浏览器中将 `PreMasterSecret` 导出。

### 使用方法

那么在解析的时候就有几种方法，对于 RSA 来说，可以直接指定服务端的密钥，但是前提是要求可以获取到密钥，通常是在自己调试时，否则基本不可用。

可以直接将会话密钥保存起来使用，这种方式就比较通用了，浏览器 Firefox 和 Chrome 都支持 [NSS Key Log Format](https://developer.mozilla.org/en-US/docs/Mozilla/Projects/NSS/Key_Log_Format) 格式，只需要在启动前设置好 `SSLKEYLOGFILE` 环境变量。

还可以使用中间人的方式，常见的工具如 [mitmproxy](https://mitmproxy.org/)、Fiddler 。

### 指定算法

所以在测试时可以使用 RSA 算法，这样方便对通讯过程数据解密进行分析，如果一切正常，那么就可以自由使用相关的算法。

{% highlight text %}
openssl ciphers -V | column -t
{% endhighlight %}

注意，查看上述 `Kx=XXX` 中的内容，需要保证是 RSA 算法。

{% include ads_content01.html %}

## 示例

测试时使用 OpenSSL 提供的命令行作为简单示例，关于证书的生成过程可以参考 [OpenSSL 常用命令](/post/security-openssl-commands-usage-introduce.html) 中的相关介绍。

相关的常用命令有。

{% highlight text %}
----- 抓取如下示例生成的报文，用于WireShark分析
tcpdump -i lo -w /tmp/tls.cap -nnnX port 44330

----- 查看私钥信息，如果无密码可以忽略最后的参数
openssl rsa -in private.pem -noout -text -passin pass:123456
{% endhighlight %}

通过如下命令启动服务端，客户端可以使用不同的场景。

{% highlight text %}
----- 服务端
openssl s_server -accept 44330 -CAfile pki/CA/cacert.pem            \
	-key pki/SVR/key.pem -cert pki/SVR/cert.pem -state

----- 客户端，使用TLSv1.2版本，并指定RSA加密套件
openssl s_client -connect 127.0.0.1:44330 -CAfile pki/CA/cacert.pem \
	-key pki/CLI/key.pem -cert pki/CLI/cert.pem -state          \
	-cipher AES128-SHA256 -tls1_2

----- 客户端，使用TLSv1.3版本，并记录Master Secret
openssl s_client -connect 127.0.0.1:44330 -CAfile pki/CA/cacert.pem \
	-key pki/CLI/key.pem -cert pki/CLI/cert.pem -state          \
	-tls1_3 -keylogfile key.log
{% endhighlight %}

在测试时，建立链接后，通过客户端发送 `Hello World!` 字符串，然后直接退出。

### RSA

简单来说，就是通过指定服务端的私钥以及通讯的内容，对报文进行解密。

如上示例，也就是使用 TLSv1.2 版本以及 `AES128-SHA256` 加密套件，可以使用 [tlsv1.2_rsa.cap](/reference/linux/network/tlsv1.2_rsa.cap) 测试。其实在指定了 `-cipher AES128-SHA256` 参数之后，其中的 `-tls1_2` 参数可以省略。

在 WireShark 中，通过 `[Edit]->[Preference]->[Protocols]->[SSL]` 设置 `RSA keys list` 选项即可，也就是上述的 `pki/SVR/key.pem` 文件，内容如下。

![TLS WireShark RSA Decrypt Example Picture](/{{ site.imgdir }}/network/TLS_WireShark_RSA_Decrypt_Example.png "TLS WireShark RSA Decrypt Example")

其中的 `Protocol` 可以不用指定，如果对私钥进行了加密，那么可以同时设置密码。

然后再打开上述抓包文件，双击 `Application Data` 中的内容，在弹出的窗口中查看 `Decrypted SSL` 标签页，内容如下。

![TLS WireShark RSA Decrypt Contents Picture](/{{ site.imgdir }}/network/TLS_WireShark_RSA_Decrypt_Contents.png "TLS WireShark RSA Decrypt Contents")

### SSLKEYLOGFILE

也就是通过 `SSLKEYLOGFILE` 环境变量设置文件路径，将每个 HTTPS 连接产生的 Premaster Secret 或 Master Secret 存下来，这样 Wireshark 就可以解密 HTTPS 流量，即使是使用了 ECDHE 这种具有前向安全性的密钥交换。

目前 Chrome 和 Firefox 都支持，只需要设置好环境变量即可。

另外，如果使用 OpenSSL 可以通过 `-keylogfile key.log` 生成 KEYLOG 文件，其内容类似如下。

```
SERVER_HANDSHAKE_TRAFFIC_SECRET 23f579bda4206b808ec...
EXPORTER_SECRET 23f579bda4206b...
SERVER_TRAFFIC_SECRET_0 23f579bda4206b80...
CLIENT_HANDSHAKE_TRAFFIC_SECRET 23f579bda4206b...
CLIENT_TRAFFIC_SECRET_0 23f579bda4206b808ec...
```

{% include ads_content02.html %}

## 其它

### 常见问题

#### 无法识别解析协议

当通过 WireShark 打开 HTTPS 流量包时，在协议部分只会显示 TCP 而非 SSL 信息，尤其是在使用非标的端口时。可以在 `[Edit]->[Preference]->[Protocols]->[HTTP]` 中设置，确保 `SSL/TLS Ports` 中含有该端口，且在 `TCP ports(s)` 中不含。

#### 其它客户端支持

其中 curl 在 7.58.0 之后的版本也支持使用上述的环境变量，所以可以使用与 Firefox 和 Chrome 类似的方式。

如果使用的是 OpenSSL 客户端，可以通过 `-debug` 参数将相关的信息打印出来，直接使用 `SSL-Session` 中的内容即可。

在 [SSL Key Log](https://git.lekensteyn.nl/peter/wireshark-notes/tree/src/sslkeylog.c) 中还提供了通过 `PRE_LOAD` 环境变量设置的方式，同样用来 dump 密钥信息。

另外 [Extract Pre-master keys from an OpenSSL application](https://security.stackexchange.com/questions/80158/extract-pre-master-keys-from-an-openssl-application) 中也介绍了通过 GDB 查看内存中的信息。

<!--
关于SSL的详细介绍
https://juejin.im/post/5b305758e51d4558ce5ea0d9


很详细介绍了TLS协议
http://blog.fourthbit.com/2014/12/23/traffic-analysis-of-an-ssl-slash-tls-session/

Linux中也可以使用SSLDUMP
http://ssldump.sourceforge.net/
http://www.361way.com/ssldump/5518.html
MasterSecrt创建方法
https://www.linuxidc.com/Linux/2015-07/120230.htm

if (SSL_set_cipher_list(ssl, "AES256-GCM-SHA384") != 1)

OpenSSL获取MasterKey的方法
ssl/ssl_txt.c 也可以参考SSL_SESSION_print
https://github.com/hallelujah-shih/start-learn/tree/master/debug_tls


OpenSSL的非阻塞代码
https://www.cnblogs.com/dongfuye/p/4121066.html




通过STARTTLS将SMTP服务器切换到SSL
https://zhuanlan.zhihu.com/p/32562251
https://gist.github.com/kennwhite/ba9c4015fac2b23ceab2
https://gist.github.com/jim3ma/b5c9edeac77ac92157f8f8affa290f45

https://stackoverflow.com/questions/13110713/upgrade-a-connection-to-tls-in-go

Transport Layer Security (TLS)
https://hpbn.co/transport-layer-security-tls/

https://github.com/akamensky/golang-upgrade-tcp-to-tls
TLS HiJack
https://gist.github.com/Soulou/6048212
https://www.nginx.com/blog/running-non-ssl-protocols-over-ssl-port-nginx-1-15-2/














使用Wireshark解密TLS 1.3流量
https://mp.weixin.qq.com/s/QhodMl210xWMK9XKjVtfAQ

不同的三种解密方法
https://imququ.com/post/how-to-decrypt-https.html

TLS/SSL抓包常见方法 包括了JAVA的解密方法
https://www.jianshu.com/p/13d96afb47ce

【网络】实践：Wireshark分析HTTPS协议
https://blog.csdn.net/xhdxhdxhd/article/details/103849383

How to Decrypt SSL with Wireshark 介绍的很详细
http://comparitech.com/net-admin/decrypt-ssl-with-wireshark/
-->


{% highlight text %}
{% endhighlight %}
