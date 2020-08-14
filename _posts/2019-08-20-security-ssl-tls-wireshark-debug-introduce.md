---
title: SSL/TLS 调试
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: ssl,tls,openssl,wireshark
description:
---

通过 Wireshark 可以分析绝大部分的 TCP/IP 协议流量，包括了 TLS 协议，可以用来分析协议的工作方式或者排查问题。

<!-- more -->

## 简介

可以直接在本地通过 Wireshark 解析报文，不过对于需要分析服务器上的报文的话，建议还是通过 tcpdump 捕获报文，然后再通过 Wireshark 进行分析，这里也是使用这一策略。

注意，TLSv1.3 与 TLSv1.2 的协议差距很大，所以需要确保对应的 Wireshark 版本支持。

### 示例

测试时使用 OpenSSL 提供的命令行作为简单示例，关于证书的生成过程可以参考 [OpenSSL 常用命令](/post/security-openssl-commands-usage-introduce.html) 中的相关介绍。

通过如下命令启动服务端和客户端。

{% highlight text %}
openssl s_server -accept 44330 -CAfile pki/CA/cacert.pem            \
	-ciphersuites "TLS_AES_256_GCM_SHA384"                      \
	-key pki/SVR/key.pem -cert pki/SVR/cert.pem -state

openssl s_client -connect 127.0.0.1:44330 -CAfile pki/CA/cacert.pem \
	-ciphersuites "TLS_AES_256_GCM_SHA384"                      \
	-key pki/CLI/key.pem -cert pki/CLI/cert.pem -state
{% endhighlight %}

## 原理

解密的核心过程其实是获得 SessionKey (MasterSecrt)，通过 `ClientRandom` `ServerRandom` 以及 `PreMasterSecret` 生成，关键是 `PreMasterSecret` 的获取，RSA 是由客户端生成，并将其用服务端的公钥加密，然后发送。

而 DH 算法则是由客户端和服务端各自生成，然后不通过网络传输完成信息交换。

#### RSA

如果是通过 RSA 算法交换秘钥，那么客户端会加密并发送给服务端，这样只需要知道服务端的私钥就可以解密出 `PreMasterSecret` 值。

#### DH

而 DH 算法中的 `PreMasterSecret` 是由服务端和客户端各自计算出来，而且没有保存到磁盘，没有通过网络传输，这样就导致无法进行解密。

如果要解密 DH 只能通过类似中间人攻击的方法，或者在浏览器中将 `PreMasterSecret` 导出。

## 其它

当通过 WireShark 打开 HTTPS 流量包时，在协议部分只会显示 TCP 而非 SSL 信息，尤其是在使用非标的端口时。可以在 `[Edit]->[Preference]->[Protocols]->[HTTP]` 中设置，确保 `SSL/TLS Ports` 中含有该端口，且在 `TCP ports(s)` 中不含。



<!--

## 原理

通过 `SSLKEYLOGFILE` 环境变量设置文件路径，将每个 HTTPS 连接产生的 Premaster Secret 或 Master Secret 存下来，这样 Wireshark 就可以解密 HTTPS 流量，即使是使用了 ECDHE 这种具有前向安全性的密钥交换。

`[Edit]->[Preference]->[Protocols]->[TLS]` 设置 `RSA keys list` 即可。




关于SSL的详细介绍
https://juejin.im/post/5b305758e51d4558ce5ea0d9

不同的三种解密方法
https://imququ.com/post/how-to-decrypt-https.html

----- 查看私钥信息
openssl rsa -in private.pem -noout -text -passin pass:123456
----- 查看证书信息
openssl x509 -in cert.pem -noout -text

----- 列出所有算法并按照强度排序
openssl ciphers -v 'ALL:!ADH:@STRENGTH'

openssl ciphers -v 'ALL:@STRENGTH'


AES256-GCM-SHA384

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
-->


{% highlight text %}
{% endhighlight %}
