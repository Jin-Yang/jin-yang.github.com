---
title: CipherSuite 简介
layout: post
comments: true
language: chinese
tag: [TLS/SSL,Security,DevOps,Program]
keywords: tls,ssl,cipher,suite
description: 直接翻译为加密套件，在 TLS/SSL 中实际上包含了四类，用来完成握手阶段的信息交互，决定了认证、加解密、密钥交换等所使用的算法。
---

直接翻译为加密套件，在 TLS/SSL 中实际上包含了四类，用来完成握手阶段的信息交互，决定了后续信息交互的过程，包括了认证、加解密、密钥交换等所使用的算法。

这里会简单介绍所支持的算法，以及如何在 OpenSSL 中使用。

<!-- more -->

## CipherSuite

加密算法套件 CipherSuite 由各类基础的加密算法组成，主要包含了四类：

1. Authentication 认证算法；
2. Encryption 对称加密算法；
3. Message Authentication Code 消息认证码算法；
4. Key Exchange 密钥交换算法。

而 TLS Cipher Suite 在 [www.iana.org](https://www.iana.org/assignments/tls-parameters/tls-parameters.xhtml) 中注册，每一个 CipherSuite 分配一个 16bits 的数字来标识，可以在上面的网站中查询。

{% highlight text %}
0x00,0xAA 	TLS_DHE_PSK_WITH_AES_128_GCM_SHA256 	Y 	Y
{% endhighlight %}

例如如上 `0x00 0xAA` 标识了 `TLS_DHE_PSK_WITH_AES_128_GCM_SHA256` 这一算法，在 RFC5487 标准中定义了其实现方法。

也可以通过 `openssl ciphers -V | column -t | less` 命令查询。

{% highlight text %}
0xC0,0x30  ECDHE-RSA-AES256-GCM-SHA384  TLSv1.2  Kx=ECDH  Au=RSA  Enc=AESGCM(256)  Mac=AEAD
{% endhighlight %}

其中 `ECDHE-RSA-AES256-GCM-SHA384` 就是定义的 CipherSuite ，用于 TLSv1.2 版本中，对应的加密算法为：

* `ECDH` 密钥交换；
* `RSA` 认证；
* `AESGCM(256)` 做对称加密；
* `AEAD` 作 MAC 。

对于 TLSv1.3 来说，可以通过 `openssl ciphers -V | column -t | grep 'TLSv1.3'` 命令查看当前支持的加密套件。

不同的加密算法套件的安全等级也略有区别，在上述的链接中也给出了是否推荐使用，也可以通过 [ciphersuite.info](https://ciphersuite.info/cs/) 网站查询加密算法的安全性。

## OpenSSL 指定

TLS/SSL 协议的实现有很多种，例如 OpenSSL、GNUTLS、NSS、LibreSSL、Cyassl、Polarssl、Botan 等等，虽然 OpenSSL 的代码算是其中最混乱的 (现在也开始改善了)，但也是最久经考验的。

可以按照 OpenSSL 的格式指定 CipherSuite 列表，通过 `:` 分割 (也可使用 `,` ` ` 但是不常用)，每个通过如下方式指定：

* 指定单个加密套件，例如 `AES256-GCM-SHA384` ；
* 包含某个特定算法的列表，例如 `SHA1` 摘要算法都为 `SHA1` 的算法，`SSLV3` 指定版本；
* 中间使用 `+` 符号，用来表示且；
* 添加 `!` `-` `+` 符号，分别表示永久删除、可以通过后面选项添加、移动到列表最后；
* 也可以通过 `@STRENGTH` 指定按照加密算法的 Key 长度排序。

相关的单个配置项可以参考 [OpenSSL Ciphers](https://www.openssl.org/docs/apps/ciphers.html) 中的介绍，当前服务端建议的配置可以参考 [Security/Server Side TLS](https://wiki.mozilla.org/Security/Server_Side_TLS) 中的内容。

<!--
## 推荐算法

随着密码学的发展，硬件性能的提高，加密和破解的不断对抗博弈，常用的算法也在不断进化，旧的算法被破解，新的算法诞生。

CipherSuite的当前流行趋势：

authentication (认证)算法 ：常见的有 RSA/DSA/ECDSA 3种，目前最主流的是人民群众喜闻乐见，妇孺皆知的RSA ( 2048 bit及以上)， （ECDSA 是新兴趋势，例如gmail，facebook都在迁移到ECDSA，当然目前用的还不多，DSA 由于只能提供1024bit，已经没啥人敢用）。

加密算法：主流趋势是使用 aes，128/256 bit都可以，加密模式的趋势是使用gcm，cbc由于被发现有 BEAST 攻击等，比较难以正确使用，至于ecb模式，请勿使用。加密算法 还有RC4（不建议使用），3DES（不建议使用），Camellia(貌似日本人搞的) ，DES(已经被淘汰)等，

message authentication code (消息认证码 简称MAC)算法 ，主流有 sha256,sha384,sha1,等。tls中使用了HMAC模式，而不是原始的 sha256,sha1等。google已经在淘汰MD5了。（gcm是一种特殊的称为aead的加密模式，不需要配合MAC。）

key exchange(密钥交换)算法：主流有两种：DH和ECDH，自从斯诺登爆料了NSA的https破解方案以后，现在的 key exchange(密钥交换)算法，普遍流行 PFS，把DH, ECDH变成 DHE，ECDHE 。

mozilla目前推荐的 cipher list：


ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!3DES:!MD5:!PSK

mozilla的优先级选择考虑：

1.ECDHE+AESGCM最先选，目前没有已知漏洞。

2.PFS ciphersuite优先，其中ECDHE优先于DHE

3.SHA256优先于SHA1。完全禁用MD5。

4.AES 128优先于AES 256。这个问题有一些讨论。

5.在向后兼容模式中，AES优先于3DES。

6.完全禁止RC4。3DES只用于兼容老版本。

cloudflare的ssl cipher list配置：




Applications should use the SSL_CTX_set_ciphersuites() or SSL_set_ciphersuites() functions to configure TLSv1.3 ciphersuites. Note that the functions SSL_CTX_get_ciphers() and SSL_get_ciphers() will return the full list of ciphersuites that have been configured for both TLSv1.2 and below and TLSv1.3.

For the OpenSSL command line applications there is a new “-ciphersuites” option to configure the TLSv1.3 ciphersuite list. This is just a simple colon (“:”) separated list of TLSv1.3 ciphersuite names in preference order. Note that you cannot use the special characters such as “+”, “!”, “-“ etc, that you can for defining TLSv1.2 ciphersuites. In practice this is not likely to be a problem because there are only a very small number of TLSv1.3 ciphersuites.

For example:

$ openssl s_server -cert mycert.pem -key mykey.pem -cipher ECDHE -ciphersuites “TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256”

This will configure OpenSSL to use any ECDHE based ciphersuites for TLSv1.2 and below. For TLSv1.3 the TLS_AES_256_GCM_SHA384 and TLS_CHACHA20_POLY1305_SHA256 ciphersuites will be available.

提到了设置TLS 1.3密码套件时应该使用SSL_CTX_set_ciphersuites()，而TLS 1.2及以下版本使用的是SSL_CTX_set_cipher_list()，入口并不一样。
-->



## 参考

[CloudFlare SSLConfig](https://github.com/cloudflare/sslconfig/blob/master/conf) 也就是 CloudFlare 当前使用的配置，也可以查看 Google 的 Blog [Google Online Security](https://googleonlinesecurity.blogspot.com.au/2013/11/a-roster-of-tls-cipher-suites-weaknesses.html) 。

<!--
https://googleonlinesecurity.blogspot.com.au/2013/11/a-roster-of-tls-cipher-suites-weaknesses.html
-->

{% highlight text %}
{% endhighlight %}
