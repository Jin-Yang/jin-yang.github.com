---
title: SSL/TLS 简介
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:
---


<!-- more -->

{% highlight text %}
密钥交换算法
   RSA
   DH/ECDH
TLSv1.3
   0-RTT
优化
   会话复用
OpenSSL
   ASYNC模式 可以使用类似QAT硬件加解密模块
{% endhighlight %}

## 简介

SSL/TLS 处于传输层 (一般为 TCP) 以及应用层 (常用的 HTTP) 之间。

![SSL/TLS Protocol Description]({{ site.url }}/images/network/ssl_tls_protocol_description.png "SSL/TLS Protocol Description"){: .pull-center width="80%" }

## 通讯流程


{% highlight text %}
        -----------+
          data   --+--------------> 1. Fragment data
        -----------+
                                    +------------------------+
                                    |                        |
                                    |                        |
                                    +------------------------+

                                    2. Compress data (generally no compression applied)

                                    +------------------------+----+
                                    |                        |MAC | Add a Message Authentication Code
                                    |                        |    |
                                    +------------------------+----+

                                    3. Encrypt data

                                    +-----------------------------+
                                    |ciphertext                   |
                                    |                             |
                                    +-----------------------------+

                                    4. Add header

                               +----+-----------------------------+
                    TLS Record |    |ciphertext                   | Add a TLS Record header
                      header   |    |                             |
                               +----+-----------------------------+
{% endhighlight %}

## Handshake Protocol

在初始建立链接时会进行握手，TLS 的握手主要为了三个目的：A) 确认双方支持的加密套件以及参数；B) 鉴权，可以单向或者双向；C) 交换对称加密用的会话密钥。

对于 TLSv1.2 以及之前版本，完成握手需要 2 个 Round-Trip Time, RTT ，有些优化方案，例如 False Start 可以在 `Change Cipher Spec Finished` 过程中，同时带有业务数据，从而变相将握手过程简化为 1-RTT ，这里仅介绍基本。

{% highlight text %}
        <<<Client>>>  |                                 |  <<<Server>>>
       [Client Hello] o-------------------------------->|
                      |  * Supported Ciphers            |
                      |  * Random Number                |
                      |  * Session ID(any)              |
                      |  * SNI                          |
                      |                                 |
                      |<--------------------------------o [Server Hello]
 {Verify Server Cert} |  * Choosen Cipher               | [Server Certificate]
                      |  * Random Number                | [Server Hello Done]
                      |  * Session ID(reuse/new)        |
                      |  * Client Certificate(optional) |
                      |                                 |
[Client Key Exchange] |-------------------------------->|
 <<<Key Generation>>> |  * Pre-Master Secret(encrypted  | <<<Key Generation>>>
                      |    with server public key)      |
                      |  * Send Client Certificate      | {Verify Client Cert}
 [Change Cipher Spec  |                                 |
  Finished(encrypt)]  |                                 |
                      |                                 |
                      |<--------------------------------| [Change Cipher Spec
                      |                                 |  Finished(encrypt)]
                      |                                 |
   [Application Data] |<------------------------------->| [Application Data]
{% endhighlight %}

如果中间交换证书时，由于证书比较大，那么可能会将报文拆分成多个 TCP 报文。如果要减小证书，可以使用 Elliptic Curve Cryptography, ECC 替换 RSA 。

### Client Hello



{% highlight text %}
{% endhighlight %}
