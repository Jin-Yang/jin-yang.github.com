---
title: TLSv1.3 简介
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: tlsv1.3,tlsv1.2
description:
---

TLS 1.3 是时隔九年对 TLS 1.2 之前版本的新升级，也是迄今为止改动最大的一次。

<!-- more -->

## 简介


TLSv1.3 与之前的协议有较大差异，主要在于：

* 不再允许出现 DSA 证书，需要使用 ECDSA、RSA 替换；



    相比过去的的版本，引入了新的密钥协商机制 — PSK
    支持 0-RTT 数据传输，在建立连接时节省了往返时间
    废弃了 3DES、RC4、AES-CBC 等加密组件，废弃了 SHA1、MD5 等哈希算法
    ServerHello 之后的所有握手消息采取了加密操作，可见明文大大减少
    不再允许对加密报文进行压缩、不再允许双方发起重协商

对比旧协议中的不足，TLS 1.3 确实可以称得上是向前迈了一大步。既避免之前版本出现的缺陷，也减少了 TLS 握手的时间。

总结一下，TLS 1.3 与以前的版本相比具有如下两个大的优势，分别是：

## OpenSSL


### 重新协商

TLSv1.3 不再支持重新协商，所以在调用 `SSL_renogotiate()` 以及 `SSL_renegotiate_abbreviated()` 时将会报错。而重新协商最常见的场景是更新链接的密钥，可以通过 `SSL_key_update()` 进行更新。

### 会话复用

在 TLSv1.2 协议中，会话是在握手阶段完成的，通过该会话可以在后续的握手过程中简化流程，通过 `SSL_get1_session()` 函数可以获取。

而在 TLSv1.3 版本中，会话是在握手成功后创建的，在正常在握手完成后，服务端会主动发送多个与会话相关的详细信息。


<!--
The specification recommends that applications only use a session once (although this may not be enforced). For this reason some servers send multiple session messages to a client. To enforce the “use once” recommendation applications could use SSL_CTX_remove_session() to mark a session as non-resumable (and remove it from the cache) once it has been used. OpenSSL servers that accept "early_data" will automatically enforce single use sessions. Any attempt to resume with a session that has already been used will fallback to a full handshake.

The old SSL_get1_session() and similar APIs may not operate as expected for client applications written for TLSv1.2 and below. Specifically if a client application calls SSL_get1_session() before the server message containing session details has been received then an SSL_SESSION object will still be returned, but any attempt to resume with it will not succeed and a full handshake will occur instead. In the case where multiple sessions have been sent by the server then only the last session will be returned by SSL_get1_session(). Calling SSL_get1_session() after a 2 way shutdown will give a resumable session if any was sent. You can check that a session is resumable with SSL_SESSION_is_resumable().

Client application developers should consider using the SSL_CTX_sess_set_new_cb() API instead. This provides a callback mechanism which gets invoked every time a new session is established. This can get invoked multiple times for a single connection if a server sends multiple session messages.

Note that SSL_CTX_sess_set_new_cb() was also available in previous versions of OpenSSL. Applications that already used that API will still work, but they may find that the callback is invoked at unexpected times, i.e. post-handshake.

An OpenSSL server will immediately attempt to send session details to a client after the main handshake has completed. The number of tickets can be set using SSL_CTX_set_num_tickets. To server applications this post-handshake stage will appear to be part of the main handshake, so calls to SSL_get1_session() should continue to work as before.

If a client sends it's data and directly sends the close notify request and closes the connection, the server will still try to send tickets if configured to do so. Since the connection is already closed by the client, this might result in a write error and receiving the SIGPIPE signal. The write error will be ignored if it's a session ticket. But server applications can still get SIGPIPE they didn't get before.

If the server sends session tickets and you want to be able to get a resumable session, you need to either call SSL_read() after the ticket was sent or do a 2 way shutdown.
-->






会话复用有两种方式：A) Session IDs [RFC-5246](https://tools.ietf.org/html/rfc5246)；B) Session Tickets [RFC-5077](https://tools.ietf.org/html/rfc5077) 。






































<!--
https://www.openssl.org/blog/blog/2017/05/04/tlsv1.3/

https://www.linuxidc.com/Linux/2017-09/147117.htm

安全性测试
https://www.ssllabs.com/ssltest/





TLSv1.2 VS. TLSv1.3 对比
https://www.jianshu.com/p/efe44d4a7501

TLSv1.3新特性
https://juejin.im/post/5b90d9e0f265da0a92238bec
https://zhuanlan.zhihu.com/p/27524995
https://www.upyun.com/tech/article/286/%E7%A7%91%E6%99%AE%20TLS%201.3%20%E2%80%94%20%E6%96%B0%E7%89%B9%E6%80%A7%E4%B8%8E%E5%BC%80%E5%90%AF%E6%96%B9%E5%BC%8F.html

TLSv1.3
https://github.com/h2o/picotls
https://github.com/eduardsui/tlse

https://blog.helong.info/blog/2015/01/23/ssl_tls_ciphersuite_intro/

https://www.jianshu.com/p/61dba20d6e66
https://blog.csdn.net/mrpre/article/details/81532469
https://x-nagi.com/2018/nginx-tls1-3-patch.html

非常非常详细介绍TLS的文章
https://blog.helong.info/blog/2015/09/06/tls-protocol-analysis-and-crypto-protocol-design/
https://blog.helong.info/blog/2015/03/13/jump_consistent_hash/


https://github.com/halfrost/Halfrost-Field/blob/master/contents/Protocol/TLS_1.3_Handshake_Protocol.md

这个Blog中有很多关于TLS的详细介绍
https://blog.csdn.net/mrpre/article/details/81532469

很详细的一系列文章
https://halfrost.com/https_tls1-3_handshake/
-->


## 参考

* [Using TLS1.3 With OpenSSL](https://www.openssl.org/blog/blog/2017/05/04/tlsv1.3/) 官方关于 TLSv1.3 的使用介绍。

<!-- https://wiki.openssl.org/index.php/TLS1.3 -->

{% highlight text %}
{% endhighlight %}
