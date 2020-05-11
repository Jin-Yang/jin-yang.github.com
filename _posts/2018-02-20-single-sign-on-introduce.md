---
title: SSO 简介
layout: post
comments: true
language: chinese
category: [python]
keywords: react,introduce
description:
---

Single Sign-On, SSO 单点登录，允许在单次登陆之后可以访问其它相关网站，也就是说，其中的一部分核心功能就是验证用户身份。

除了上述的认证，同时也会作为用户的跟踪。

<!-- more -->

## 简介

当前互联网的大部分功能都是通过 HTTP 协议构建的，但因为 HTTP 协议是无状态的，为了追踪用户提出了多种方法，例如 cookie、session、token、浏览器指纹等等。

一般会通过 `login.foobar.com` 类似的域名登陆网站，在单次登陆之后，可以免登陆访问其它相关网站，从而可以实现一次登陆全部访问。



## 实现

通常有两种方案：

1. Session，在 Cookies 中会保存类似 ID 的信息，同时在 Cookies 或者后台 (默认) 缓存信息，每次调用接口时检查是否合法。
2. Token，每次请求会带上对应的 Token ，然后直接通过 Token 检查是否合法即可。
3. AK/SK，实际上类似用户名密码，但是用途略有区别。

其中第一种方案，为了防止被破解，一般会在服务端缓存，这样就会依赖一个全局的缓存机制；而后者因为存在时效性，而且信息是保存在 Token 中的，减少了对缓存的依赖，但是无法从服务端主动取消用户登陆。

所以，建议用户登陆使用 Session 机制，而机机鉴权采用 Token 机制。

### AK/SK

通过使用 Access Key ID(AK)/Secret Access Key(SK) 来验证某个请求的发送者身份，在调用 API 时，需要对参数以及 AK 使用 SK 进行签名，生成一个签名信息；服务端会根据 AK 查询到预制的 SK 以相同方式进行签名，并确保一致，防止中间被篡改。

通常会采用 HMAC 作认证，在这一过程中可以避免传输 SK ，而且一般来说签名只会使用一次，可以避免重放攻击。




注意，客户端和服务端需要同时保存 AK/SK ，而且 AK 会在传输过程中使用，所以，需要保证 Secure Key 不能在网络中传输，以及不能在不受信任的位置存放。


## 其它

<!--
https://insights.thoughtworks.cn/api-2/
-->


{% highlight text %}
{% endhighlight %}
