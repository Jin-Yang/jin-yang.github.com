---
title: 【专题】安全相关
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---

<!-- more -->

## 基本概念

![Linux Security Logo]({{ site.url }}/images/linux/linux-security-logo.png "Linux Security Logo"){: .pull-center width="300" }

主要是 Linux 下与安全相关的内容。

* [加密算法简介](/post/security-encryption-introduce.html) 常见加密算法，包括对称、非对称加密、HASH算法、文件完整性等。
* [密钥交换算法简介](/post/security-key-exchange-method-introduce.html)
* [Linux 密码管理](/post/security-how-to-save-password.html) 简单介绍下 Linux 中的密码管理。
* [SELinux 简介](/post/linux-selinux-introduce.html) 同样是 Linux 中的一种强制存取控制的实现。
* [PGP 简介](/post/security-pgp-introduce.html) 一个基于公钥加密体系的加密软件。

## 编码

* [libgcrypt 使用](/post/security-libgcrypt-practice.html)，非常成熟的加密算法库，简单介绍其使用方法。
* [HTTPS 协议详解](/post/https-introduce.html)，简单介绍下 HTTPS 协议是如何实现的

## 单点登陆

* [SSO 简介](/post/json-web-token-introduce.html) 简单介绍下 JWT 的原理及其使用示例。
* [JWT 简介](/post/json-web-token-introduce.html) 简单介绍下 JWT 的原理及其使用示例。
* [双因子验证](/post/two-factors-authenticator-introduce.html) 通过密码以及另外的安全验证方法。

## TLS

Transport Layer Security, TLS 前身是 Secure Sockets Layer, SSL，SSL 最初的几个版本 (SSL 1.0、SSL 2.0、SSL 3.0) 由网景公司开发，然后从 3.1 开始被 IETF 标准化并改名，然后有 TLS1.0、TLS1.1、TLS1.2 以及 TLS1.3 几个版本，TLS1.3 改动较大。

{% highlight text %}
   SSL1.0     SSL2.0  SSL3.0  TLS1.0  TLS1.1  TLS1.2  TLS1.3
-----+-----------+------+-------+-------+-------+-------+----->
 unpublished   1995   1996    1999    2006    2008    2018
{% endhighlight %}

其中 SSL1.0 从未公开过，而 SSL2.0 和 SSL3.0 都存在安全问题，不推荐使用，最新的 TLSv1.3 可以参考 [RFC8446](https://datatracker.ietf.org/doc/rfc8446/)，这里简单介绍。

TLS 协议的实现有多种，如 OpenSSL、GnuTLS、PolarSSL 等等。

* [SSL/TLS 简介](/post/security-ssl-tls-overview.html) 包括了一些基本的概念。
* [CipherSuites 简介](/post/security-ssl-tls-ciphersuites-introduce.html) 加密套件的基本概念，以及在 OpenSSL 中的使用。
* [OpenSSL 常用命令](/post/security-openssl-commands-usage-introduce.html) 一些命令使用方式，包括编译安装、证书、服务端、客户端等。

## SSH

![OpenSSH Logo]({{ site.url }}/images/misc/openssh-logo-1.jpg "OpenSSH Logo"){: .pull-center width="150" }

OpenSSH 是 SSH (Secure SHell) 协议的免费开源实现，一种命令行的远程登录工具，使用加密的远程登录实现，可以有效保护登录及数据的安全，同时提供了安全的文件传输功能。

这里主要介绍 SSH 一些常见的操作。

* [SSH 简介](/post/ssh-introduce.html)，简单介绍 OpenSSH 相关的内容。
* [SSH 代理设置](/post/ssh-proxy.html)，关于一些常见代理设置，如本地转发、远程转发、动态转发等。
* [SSH Simplify Your Life](/post/ssh-simplify-your-life.html)，用来配置一些常见的设置，简化登陆方式。
* [SSH 杂项](/post/ssh-tips.html)，记录一些常见的示例。

## Capture The Flag, CTF

夺旗赛，在网络安全领域中指的是网络安全技术人员之间进行技术竞技的一种比赛形式，通常包括了以下几个技能：逆向工程、密码学、ACM 编程、Web 漏洞、二进制溢出(PWN)、网络和取证等。

* [PWN 简介](/post/security-ctf-pwn-introduce.html)

<!--
夺旗工具
https://www.freebuf.com/sectool/94235.html
-->

### 参考

* [github CTFs](https://github.com/ctfs) 历年 CTF 的整理。

## 参考

### Awosome

* [DropBear](https://matt.ucc.asn.au/dropbear/dropbear.html) 一个小型的 SSHv2 客户端/服务器，包括了开源库 [libtomcrypt](https://www.libtom.net/LibTomCrypt/) 以及 [libtommath](https://www.libtom.net/LibTomMath/) 。
* 比较常用的库包括了 [OpenSSL](https://github.com/openssl/openssl)、[libsodium](https://github.com/jedisct1/libsodium)、[s2n](https://github.com/awslabs/s2n) 。
* 最开始被称为 PolarSSL ，后来更名为 [mbed TLS](https://github.com/ARMmbed/mbedtls) ，一个用于嵌入式设备的加密库，另外一个是 XySSL 不过已经不再维护了，还有 [MatrixSSL](https://github.com/matrixssl/matrixssl)、[CrySSL/wolfSSL](https://github.com/wolfSSL/wolfssl)。

### SSL 测试网站

* [www.ssllabs.com](https://www.ssllabs.com/ssltest/index.html)



<!--
SSH/gpg-agent使用
https://faner.gitlab.io/blog/2016/08/08/GPG%E5%8A%A0%E5%AF%86%E8%BD%AF%E4%BB%B6%E7%9A%84%E4%BD%BF%E7%94%A8/
http://blog.theerrorlog.com/using-gpg-keys-for-ssh-authentication.html

磁盘加密
https://github.com/veracrypt/VeraCrypt
-->

{% highlight text %}
{% endhighlight %}
