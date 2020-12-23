---
title: SSL/TLS 简介
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: ssl,tls,openssl
description:
---

SSL/TLS 处于传输层 (一般为 TCP) 以及应用层 (常用的 HTTP) 之间，提供了加密 (Encryption)、认证 (Verification)、鉴定 (Identification) 三种功能。

如下简单介绍其基本概念。

<!-- more -->

## 简介

SSL 最初是在 1996 年由 Netscape 发布，由于一些安全的原因 SSL v1.0 和 SSL v2.0 都没有公开，直到 1996 年的 SSL v3.0。TLS 是 v3.0 的升级版，目前市面上所有的 HTTPS 都是用的是 TLS 而非 SSL，而且很多地方都混用两者。

SSL/TLS 协议在 TCP 协议之上，通过一些子协议提供了上述的安全机制。

![SSL/TLS Protocol Description]({{ site.url }}/images/network/ssl_tls_protocol_description.png "SSL/TLS Protocol Description"){: .pull-center width="80%" }

<!--
![https]({{ site.url }}/images/linux/https-ssl-tls-layer.png "https"){: .pull-center width="75%" }

其中上图中顶层的三块又组成了 SSL Handshaking Protocols，在处理时有三种状态。

{% highlight text %}
empty state ------------------ pending state ------------------ current state

             Handshake Protocol                Change Cipher Spec
{% endhighlight %}

当完成握手后，客户端和服务端确定了加密、压缩和 MAC 算法及其参数，数据通过指定算法处理；而之前的所有操作都是明文传输的。
-->

如下的解释中，假设是张三和李四在通讯，简单介绍所谓加密、认证、鉴定的概念。

{% highlight text %}
私密性(Confidentiality/Privacy):
    也就是提供信息加密；保证信息只有张三和李四知道，而不会被窃听。
可信性(Authentication):
    身份验证，主要是服务器端的，有些银行也会对客户端进行认证；用来证明李四就是李四。
完整性(Message Integrity):
    保证信息传输过程中的完整性，防止被修改；李四接收到的消息就是张三发送的。
{% endhighlight %}

如下简单介绍 SSL/TLS 中使用到的常见技术方案。

### 关键技术

首先，在实现时加密算法是公开的，而私密性是通过密钥保证的。另外，为了实现上述的机制，需要一些比较常见的技术。

其中与密码学相关的内容，可以参考 [加密算法简介](/blog/encryption-introduce.html) 。

#### 私钥交换 Key Exchange Algorithms

发送信息的时候，需要通过对称加密来加密数据，此时就涉及到了私钥的交换，通常使用的算法包括了 Diffie-Hellman、RSA 。

{% highlight text %}
Bob's Public Key + Alice's Secret Key     Bob's Secret Key (Decrypt)
          |----->-------->------->-------->------>-----|
                                              Get Alice's Secret Key
{% endhighlight %}

对于 RSA 来说，所有人都知道 Bob 的公钥，只需要将 Alice 的私钥通过公钥进行加密即可，该信息只有通过 Bob 的私钥才可以解密。

当然，在实际使用时还会增加其它的随机值。

#### 数字证书 Digital Certificates

到此，需要通过一种方法获取到 Bob 的公钥，这就是通过数字证书获取的。数字证书中除了包括了公钥信息，还有与 Bob 相关的信息。

#### 数字证书认证机构 Certification Authority, CA

接下来解决的是，如何确认数字证书的拥有着就像它声明的那样，为此引入了 Public Key Infrastructure, PKI ，包括一些与数字证书相关的软件、硬件等信息。

CA 就是一个三方机构，用来证明 Bob 确实就是 Bob 。

CA 认证分为三类：DV (domain validation)，OV (organization validation)，EV (extended validation)，证书申请难度从前往后递增，貌似 EV 这种不仅仅是有钱就可以申请的。

#### 数字签名 消息完整性

实际上有篇很不错的文章 [What is a Digital Signature?](http://www.youdzone.com/signature.html) ，也可以参考 [本地文章](/reference/linux/What is a Digital Signature.mht)，在此就不做过多介绍了。

<!--
## 通讯流程

{% highlight text %}
        -----------+
          data   --+-------------- 1. Fragment data
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
-->

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

## 其它

### 握手优化

#### False Start

一般在 TLSv1.2 版本中使用，客户端在发送 Change Cipher Spec Finished 的同时发送相关的应用数据，服务端在完成 TLS 握手之后会直接返回数据，这样也就意味着在完成 TLS 握手之前已经在传输数据了。

这里暂不详细介绍。

#### 证书优化

TLS 的身份认证是通过证书链完成，服务端在 TLS 握手阶段发送站点证书，客户端根据证书向上递归检查父证书，直到本地保存的信任根证书。

在发送的过程中，最好只发送站点证书，也有可能需要中间证书，但是，如果层数过多可能会导致报文过大，这样底层的 TCP 就可能会导致拆包，从而影响性能。

可以使用 ECC 替换，在保证安全性的前提下可以有效降低大小，不过需要注意，部分老的浏览器不支持。

#### 会话复用

简单来说，就是将辛辛苦苦算出来的对称密钥保存下来，在后续的请求中直接使用，从而可以减少证书传输的开销。


## 参考

* 很不错的参考 [Traffic Analysis of an SSL/TLS session](http://blog.fourthbit.com/2014/12/23/traffic-analysis-of-an-ssl-slash-tls-session/) 。

<!--
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
-->

{% highlight text %}
{% endhighlight %}
