---
title: HTTPS 协议详解
layout: post
comments: true
language: chinese
category: [linux,webserver,misc]
keywords: https,ssl,tls
description: 我们知道 HTTP 采用的是明文传输，而在互联网中，比如要在淘宝买个东西，使用支付宝转个帐，必须要保证这些信息只有客户端和服务器才知道的，也就是通过 HTTPS 协议。接下来，我们就看看 HTTPS 协议是如何实现的。
---

我们知道 HTTP 采用的是明文传输，而在互联网中，比如要在淘宝买个东西，使用支付宝转个帐，必须要保证这些信息只有客户端和服务器才知道的，也就是通过 HTTPS 协议。

接下来，我们就看看 HTTPS 协议是如何实现的。

<!-- more -->

![https logo]({{ site.url }}/images/linux/https-logo.jpg "https logo"){: .pull-center }

## 简介

Secure Hypertext Transfer Protocol, HTTPS 也就是安全超文本传输协议，是一个安全通信通道，它基于 HTTP 开发用于在客户计算机和服务器之间交换信息。

实际上，简单来说，它是 HTTP 的安全版，是使用 SSL(Secure Socket Layer)/TLS(Transport Layer Security) 加密的 HTTP 协议。通过 TLS/SSL 协议的的身份验证、信息加密和完整性校验的功能，从而避免信息窃听、信息篡改和信息劫持的风险。


## 执行流程

我们直接从 WireShark 的官网网站上下载了一个 HTTPS 的示例，详细的下载地址可以参考文章末尾的参考内容。

首先，是正常的 TCP 开始的三次握手连接，在此就不做过多的介绍了；完成之后才会开始 SSL 之间的沟通，这也是接下来重点介绍的内容。

在进行正常的数据传输之前，会有一个协商的过程，包括了协议的版本、会话ID、加密算法 (含有一套，后面介绍)、压缩方法；另外，在该阶段，还会相互交换服务端和客户端的随机值。

### 1. Client Hello

示例中有两个会话的数据传输，分别是 SSLv2.0 以及 SSLv3.0，现在一般使用的是 v3.0，其中 v2.0 在该阶段会使用 challenge 机制，不过不太清楚详细内容，在 [RFC-6101](https://tools.ietf.org/rfc/rfc6101.txt) 中略有介绍。

这里，仅介绍 v3.0 的内容。

![https]({{ site.url }}/images/linux/https-protocol-00.png "https"){: .pull-center width="80%" }

在上述的图片中，包括了协议的版本信息，还有些比较重要的信息。

##### 随机数

开始的四个字节以 Unix 时间格式记录了客户端 UTC 时间 (从1970年1月1日开始到当前时刻所经历的秒数)；接着后面有 28 字节的随机数，在后面的过程中我们会用到这个 **客户端的随机数** 。

##### Session ID, SID

一般来说，第一次建立连接的时候 ID 是一个空值 (NULL)，当然也有可能会重用之前的会话，从而避免一个完整的握手过程。

##### 密文族 (Cipher Suites)

密文族是浏览器所支持的加密算法的清单，整个密文族有 20 种。

另外，有是还会有 Server_name 的扩展，能告诉服务器浏览器当前尝试访问的域名。

### 2. Server Hello

在服务端回复的报文中，包含了三部分子信息：Server Hello、Certificate、Server Hello Done，实际上这是第一个会话的内容，如果复用会话，这里的报文内容会有所区别。

![https]({{ site.url }}/images/linux/https-protocol-01.png "https"){: .pull-center width="80%" }

#### Server Hello

在服务器发送的 Hello 握手报文中，同样包括了 4 字节的 UTC 时间戳以及 28 字节随机数；还有会话 ID 以及服务器最终选择的加密族。

可以看到，在一堆加密族中，最终选择的是 TLS_RSA_WITH_3DES_EDE_CBC_SHA，这也就意味着将会使用 RSA 公钥加密算法来区分证书签名和交换密钥；通过 3DES_EDE_CBC 加密算法来加密数据；利用 SHA 来做 hash 校验信息。

#### Certificate

也就是证书信息，该证书允许客户端在服务器上进行认证，证书的内容是明文保存的，可以通过浏览器查看，当然也包括了 WireShark 。

![https]({{ site.url }}/images/linux/https-protocol-02.png "https"){: .pull-center width="67%" }

#### Server Hello Done

这是一个零字节信息，用于告诉客户端整个协商过程已经结束，并且表明服务器不会再向客户端询问证书。

### 3. 校验证书

此时，浏览器已经获取了服务器的证书，浏览器会通过证书确认网站是否受信，它会检查支持的域名、是否在证书有效时间范围内、确认证书所携带的公共密钥已被授权用于交换密钥。

为什么我们要信任这个证书？证书中所包含的签名是一串非常长的大端格式的数字：

![https]({{ site.url }}/images/linux/https-protocol-03.png "https"){: .pull-center width="67%" }

在验证证书的有效性的时候，会逐级去寻找签发者的证书，直至根证书为结束，然后通过公钥一级一级验证数字签名的正确性。

### 4. 生成 Premaster

当客户端通过了证书校验之后，会生成 48 字节的随机数，称为 Pre-master Secret 。如前所述，最终选择 RSA 来交换密钥，此时 pre-master 会用来计算 master secret 以及 key block 。

![https]({{ site.url }}/images/linux/https-protocol-04.png "https"){: .pull-center width="67%" }

其中，pre-master secret 在发送时，会通过服务器的公钥进行加密。

另外，该报文中还包括了 "Change Cipher Spec" ，用来告知服务器，后面客户端发送的信息，都是按照之前协商好的算法进行。

而最后的 "Encrypted Handshake Message" 则包含了之前通讯中的 hash 和 MAC 信息。

### 5. 计算 Master Secret 以及 Key Block

到此为止，服务端和客户端都知道了 Pre-master secret (48Bytes) + Client.random (28Bytes) + Server.random (28Bytes)，接下来就会按照协议，通过如下的算法生成 Master Secret 。

{% highlight text %}
master_secret = PRF(pre_master_secret, "master secret", ClientHello.random + ServerHello.random)
{% endhighlight %}

其中 PRF 是一个随机函数，定义如下。

{% highlight text %}
PRF(secret, label, seed) = P_MD5(S1, label + seed) XOR P_SHA-1(S2, label + seed)
{% endhighlight %}

在计算 Master Secret 的同时，服务器和客户端还会同时计算 Key Block ，算法如下。

{% highlight text %}
key_block = PRF(SecurityParameters.master_secret, "key expansion",
    SecurityParameters.server_random + SecurityParameters.client_random);
{% endhighlight %}

Key Block 会被分为不同的 Blocks。

![https]({{ site.url }}/images/linux/https-key-blocks.png "https"){: .pull-center }

其中，MAC Key 用来对数据进行验证的，Encryption Key 用来对数据进行加解密的会话密钥。

### 6. 服务端确认加密

服务端在收到客户端发送的 "Encrypted Message" 消息之后，会对其中的值进行校验，如果校验失败则会关闭连接。

![https]({{ site.url }}/images/linux/https-protocol-05.png "https"){: .pull-center width="67%" }

校验成功则会与客户端一样发送 "Change Cipher Spec" 。

### 7. 发送加密信息

到此为止，双方就可以愉快的发送加密信息了。

### 8. 杂项

记录下，杂七杂八的东西。

#### Encrypted Handshake Message

实际上，在应用数据在传输之前，也就是在握手中的加密数据，要附加上 MAC Secret，然后再对这个数据包使用 write encryption key 进行加密。

在服务端收到密文之后，使用 Client write encryption key 进行解密，客户端收到服务端的数据之后使用 Server write encryption key 进行解密，然后使用各自的 write MAC key 对数据的完整性包括是否被串改进行验证。

#### Premaster Secret

其长度为 48 个字节，前 2 个字节是协议版本号，剩下的 46 个字节填充一个随机数。

<!--
PreMaster secret前两个字节是TLS的版本号，这是一个比较重要的用来核对握手数据的版本号，因为在Client Hello阶段，客户端会发送一份加密套件列表和当前支持的SSL/TLS的版本号给服务端，而且是使用明文传送的，如果握手的数据包被破解之后，攻击者很有可能串改数据包，选择一个安全性较低的加密套件和版本给服务端，从而对数据进行破解。所以，服务端需要对密文中解密出来对的PreMaster版本号跟之前Client Hello阶段的版本号进行对比，如果版本号变低，则说明被串改，则立即停止发送任何消息。
-->

需要注意，前两个随机数都是明文传输的，窃听者是可以轻易获取到的，只有最后一个 PreMaster Secret 是加密传输的，只有拥有服务器私钥才能解密，一旦 PreMaster Secret 泄露，那么本次通信就就完全可被破解了。

#### 关于随机数

如上，是通过三个随机数来生成最终的 Master Secret 的，SSL 协议不信任每个主机都能生成完全随机的随机数，所以这里需要服务器和客户端共生成 3 个随机数，每增加一个自由度，随机性就会相应增加。






## 其它

### Cipher Suite

在协商阶段，服务器和客户端会协商使用的加密协议，如下是客户端提供的协议，服务端会选择相应的协议。

![https cipher specs]({{ site.url }}/images/linux/https-cipher-specs.png "https cipher specs"){: .pull-center }

在示例中，会使用 TLS_RSA_WITH_AES_256_CBC_SHA (0x0035)，其中每组包含了四部分信息，分别是：

* 密钥交换算法。客户端与服务器之间在握手的过程中如何认证，用到的算法包括 RSA、Diffie-Hellman、ECDH、PSK 等；
* 加密算法。加密消息流，该名称后通常会带有两个数字，分别为密钥的长度和初始向量的长度；如果是一个数则表示相同，如 DES、RC2、RC4、AES 等；
* 报文认证信息码 (MAC) 算法。用于创建报文摘要，防止被篡改，从而确保消息的完整性，如 MD5、SHA 等。

对于上述的协议，实际使用的算法如下。

{% highlight text %}
TLS_RSA_WITH_AES_256_CBC_SHA (0x0035)
   基于 TLS 协议；
   使用 RSA 作为密钥交换算法；
   加密算法是 AES，其中密钥和初始向量的长度都是 256；
   MAC 算法，在这里就是哈希算法是 SHA。
{% endhighlight %}


## 攻击

Web 安全是一项系统工程，任何细微疏忽都可能导致整个安全壁垒土崩瓦解。

如上所述，虽然 HTTPS 提供了 内容加密、数据完整性、身份认证 三大安全保证；但是，也会受到非法根证书、服务端配置错误、SSL 库漏洞、私钥被盗等等风险的影响。

### Man-in-the-middle

也就是中间人攻击，能够与网络通讯两端分别创建连接，交换其收到的数据，使得通讯两端都认为自己直接与对方对话，事实上整个会话都被中间人所控制。

类似于一些抓包调试工具，基本都是通过创建本地 Proxy 服务，再修改浏览器 Proxy 设置来达到拦截流量的目的，它们的工作原理与中间人攻击一致，常见的有 Fiddler、Charles 和 Whistle。

在此主要讨论 HTTPS 中间人，简单示意如下：

{% highlight text %}
Server <---> Local Proxy <---> Browser
         ^                 ^
       HTTPS(1)          HTTPS(2)
{% endhighlight %}

上述 HTTPS(1) 连接，是中间人冒充客户端与服务端建立的连接，由于 HTTPS 服务端一般不认证客户端身份，这一步通常没有问题。

对于 HTTPS(2) 连接来说，中间人想要冒充服务端，必须拥有对应域名的证书私钥，而攻击者要拿到私钥，只能通过这些手段：1）去网站服务器上拿；2）从 CA 处签发证书；3）自己签发证书。

要防范前两点，需要保障主机安全避免私钥被盗，避免攻击者重签证书；对于自签证书，需要防止在客户端添加内置根证书，如下是 Fiddler 工具添加的受信根证书。

![https root]({{ site.url }}/images/linux/https-man-in-the-middle.png "https root"){: .pull-center width="60%" }

### RSA Private Key

在参考中有介绍如何通过 FireFox + Wireshark 抓包直接读取并分析网卡数据，这里的示例实际提供了私钥，所以看看如何通过私钥来解密这个网站的加密流量。

打开 Wireshark 的 SSL 协议设置，也就是 Edit => Preferences => Protocols => SSL，把 IP、端口、协议和证书私钥都配上，其中私钥必须存为 PEM 格式。

![https]({{ site.url }}/images/linux/https-protocol-10.png "https"){: .pull-center width="80%" }

然后就可以看到，密文已经被解密了，只能解密 HTTP-1，而不能解密 HTTP-2。










## 参考

一篇介绍 https 不错的文章，可以参考中文版 [HTTPS连接的前几毫秒发生了什么](http://blog.jobbole.com/48369/)，或者英文版 [The First Few Milliseconds of an HTTPS Connection](http://www.moserware.com/2009/06/first-few-milliseconds-of-https.html)，或者 [本地保存的版本](/reference/linux/The First Few Milliseconds of an HTTPS Connection.mht)；细节内容也可以参考协议的实现 [SSL V3.0](https://tools.ietf.org/rfc/rfc6101.txt) 。

其中的示例是从 [WireShark SampleCaptures](https://wiki.wireshark.org/SampleCaptures) 上下载的，也就是 [SSL with decryption keys](https://wiki.wireshark.org/SampleCaptures?action=AttachFile&do=get&target=snakeoil2_070531.tgz)，也可以从 [本地](/reference/linux/ssl-example.tar.bz2) 下载。


#### 其它

一些常见的技巧可以参考：

* [使用 Wireshark 调试 HTTP/2 流量](http://blog.jobbole.com/95106/)，通过 FireFox 导出密钥，可以参考 [本地文档](/reference/linux/Wireshark_HTTP_2.mht) 。

* 网站检测的链接 [SSL LABS](https://www.ssllabs.com/)、[SSL Shopper](https://www.sslshopper.com/ssl-checker.html)、[China SSL](https://www.chinassl.net/ssltools/ssl-checker.html) 。



<!--

openssl s_client -connect 10.44.32.91~94\81~84:443

一个关于SSL不错的Blog
https://blog.ivanristic.com/SSL_Threat_Model.png
HTTPS的七个误解（译文）
http://www.ruanyifeng.com/blog/2011/02/seven_myths_about_https.html
浅谈三种解密 HTTPS 流量的方法
http://www.2cto.com/article/201603/496004.html
很详细介绍的SSL
https://blog.cloudflare.com/keyless-ssl-the-nitty-gritty-technical-details/
TLS/SSL 高级进阶
https://segmentfault.com/a/1190000007283514
HTTPS 理论详解与实践
https://segmentfault.com/a/1190000004985253

https://www.wosign.com/faq/faq2016-0309-04.htm
http://blog.jobbole.com/86660/
http://www.itrus.cn/service_13html
http://resources.infosecinstitute.com/cryptography-101-with-ssl/
-->


{% highlight text %}
{% endhighlight %}
