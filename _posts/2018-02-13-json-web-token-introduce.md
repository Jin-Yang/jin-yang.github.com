---
title: SSO JWT 简介
layout: post
comments: true
language: chinese
category: [python]
keywords: react,introduce
description: 在程序开发中，用户认证授权是一个绕不过的重难点。以前的开发模式下，cookie 和 session 认证是主流，随着前后端分离的趋势，基于 Token 的认证方式成为主流，而 JWT(RFC 7519) 是基于 Token 认证方式的一种机制，是实现单点登录认证的一种有效方法。
---

在程序开发中，用户认证授权是一个绕不过的重难点。

以前的开发模式下，cookie 和 session 认证是主流，随着前后端分离的趋势，基于 Token 的认证方式成为主流，而 JWT(RFC 7519) 是基于 Token 认证方式的一种机制，是实现单点登录认证的一种有效方法。

<!-- more -->

## JSON Web Token, JWT

JWT 定义了一个紧凑且自包含的方式，通过 JSON 对象安全地传输信息，这些信息可以通过数字签名进行验证和信任，可以使用 HMAC 算法或使用 RSA 的公私钥对来对 JWT 进行签名。

体积足够小，可以通过 URL、POST 参数或 HTTP 头发送，而且其有效载荷包含有关用户的所有必需信息，避免了多次查询数据库。

详细可以参考 [jwt.io](https://jwt.io/introduction/) 中的相关入门介绍，如下仅仅介绍相关的概念。

### 结构

JWT 包含三个由点 `.` 分隔的部分：头部、有效载荷、签名，所以看起来基本上类似 `xxx.yyy.zzz` 的格式。

#### 头部

头部通常由两部分组成：令牌的类型 (JWT) 和正在使用的散列算法 (例如HMAC SHA256 RSA）。

{% highlight text %}
{
	"alg": "HS256",
	"typ": "JWT"
}
{% endhighlight %}

然后，这个 JSON 被 `Base64Url` 编码，形成 JWT 的第一部分。

#### 有效载荷

第二部分是包含声明的有效载荷，有三种类型的声明 (Claims) ：保留，公开和私有声明。

* 保留的声明 (Reserved Claims)，一组预先定义的声明，非强制性但推荐使用，例如 iss(发行人)，exp(到期时间)，sub(主题)，aud(听众)等等。
* 公开声明 (Public Claims)，这些可以由使用JWT的人员任意定义。 但为避免冲突，应在IANA JSON Web令牌注册表中定义它们，或者将其定义为包含防冲突命名空间的URI。
* 私有声明(Private Claims)，：这是为了同意使用它们的各方之间共享信息而创建的自定义claims。

{% highlight text %}
{
	"sub": "1234567890",
	"name": "John Doe",
	"admin": true
}
{% endhighlight %}

其中保留声明主要包含了如下几种：

{% highlight text %}
sub   该JWT所面向的用户 
iss   该JWT的签发者 
iat   Issued AT 在什么时候签发的token
exp   Expires token什么时候过期 
nbf   Not Before token在此时间之前不能被接收处理
jti   JWT ID为web token提供唯一标识
{% endhighlight %}

#### 签名

签名部分会计算 `编码头部` `编码有效载荷` 中的内容，例如，如果想使用 `HMAC SHA256` 算法，签名将按以下方式创建：

{% highlight text %}
HMACSHA256(
	base64UrlEncode(header) + "." +
	base64UrlEncode(payload),
	secret
)
{% endhighlight %}

签名用于验证 JWT 是否被修改。

### 加密算法

{% highlight text %}
HS256 HS384 HS512
	HMAC   + SHA256/384/512

ES256 ES384 ES512
	ECDSA  + SHA256/384/512
RS256 RS384 RS512
	RSA    + SHA256/384/512
PS256 PS384 PS512
	RSAPSS + SHA256/384/512
{% endhighlight %}



### 如何工作

当用户使用自己的凭证 (Credentials) 成功登录时，将返回一个 JSON Web Token，并且必须保存在本地 (可以是本地存储中，也可以使用 Cookie)，当然为了安全考虑，需要确认其有效时间。

无论何时用户想要访问受保护的路由或资源时，需要同时带上 JWT ，一般来说在 `Authorization` 头部的 `Bearer` 模式中，类似如下：

{% highlight text %}
Authorization: Bearer <token>
{% endhighlight %}

这是一种无状态身份验证机制，因为用户状态永远不会保存在服务器内存中，服务器通过检查是否为有效的 JWT 判断其权限，由于 JWT 是独立的，所有必要的信息都在那里，减少了多次查询数据库的需求。



<!--
1，JWT token的组成

头部（Header），格式如下：
{
“typ”: “JWT”,
“alg”: “HS256”
}
由上可知，该token使用HS256加密算法，将头部使用Base64编码可得到如下个格式的字符串：

    eyJhbGciOiJIUzI1NiJ9

有效载荷（Playload）：
{
“iss”: “Online JWT Builder”,
“iat”: 1416797419,
“exp”: 1448333419,
…….
“userid”:10001
}

有效载荷中存放了token的签发者（iss）、签发时间（iat）、过期时间（exp）等以及一些我们需要写进token中的信息。有效载荷也使用Base64编码得到如下格式的字符串：

    eyJ1c2VyaWQiOjB9

签名（Signature）：
将Header和Playload拼接生成一个字符串str=“eyJhbGciOiJIUzI1NiJ9.eyJ1c2VyaWQiOjB9”，使用HS256算法和我们提供的密钥（secret,服务器自己提供的一个字符串）对str进行加密生成最终的JWT，即我们需要的令牌（token），形如：str.”签名字符串”。
2，token在服务与客户端的交互流程

1：客户端通过用户名和密码登录
2：服务器验证用户名和密码，若通过，生成token返回给客户端。
3：客户端收到token后以后每次请求的时候都带上这个token，相当于一个令牌，表示我有权限访问了
4：服务器接收（通常在拦截器中实现）到该token，然后验证该token的合法性（为什么能验证下面说）。若该token合法，则通过请求，若token不合法或者过期，返回请求失败。
3，关于Token的思考

服务如何判断这个token是否合法？
由上面token的生成可知，token中的签名是由Header和有效载荷通过Base64编码生成再通过加密算法HS256和密钥最终生成签名，这个签名位于JWT的尾部，在服务器端同样对返回过来的JWT的前部分再进行一次签名生成，然后比较这次生成的签名与请求的JWT中的签名是否一致，若一致说明token合法。由于生成签名的密钥是服务器才知道的，所以别人难以伪造。

token中能放敏感信息吗？
不能，因为有效载荷是经过Base64编码生成的，并不是加密。所以不能存放敏感信息。
4，Token的优点

（1）相比于session，它无需保存在服务器，不占用服务器内存开销。
（2）无状态、可拓展性强：比如有3台机器（A、B、C）组成服务器集群，若session存在机器A上，session只能保存在其中一台服务器，此时你便不能访问机器B、C，因为B、C上没有存放该Session，而使用token就能够验证用户请求合法性，并且我再加几台机器也没事，所以可拓展性好就是这个意思。
（3）由（2）知，这样做可就支持了跨域访问。
-->








<!--
https://segmentfault.com/a/1190000010312468
-->

## JWT 实现方案

### JWT-GO

[jwt-go](https://github.com/dgrijalva/jwt-go)

#### 简单示例

如下是 `HS256` 的示例。

{% highlight go %}
package main

import (
	"fmt"
	"time"

	"github.com/dgrijalva/jwt-go"
)

func main() {
	key := "It's your secret key"
	token := jwt.New(jwt.SigningMethodHS256)
	claims := make(jwt.MapClaims)
	claims["username"] = "Your Name"
	claims["iss"] = "Cargo JWT Builder"
	claims["aud"] = "www.cargo.com"
	claims["exp"] = time.Now().Add(time.Hour * time.Duration(1)).Unix()
	claims["iat"] = time.Now().Unix()
	token.Claims = claims

	tokenString, err := token.SignedString([]byte(key))
	if err != nil {
		fmt.Printf("[ERROR] Sign the token failed, %v\n", err)
		return
	}
	fmt.Printf("[ INFO] Got signed token '%v'\n", tokenString)

	newtoken, err := jwt.Parse(tokenString, func(token *jwt.Token) (interface{}, error) {
		if _, ok := token.Method.(*jwt.SigningMethodHMAC); !ok {
			return nil, fmt.Errorf("Unexpected signing method: %v", token.Header["alg"])
		}
		return []byte(key), nil
	})

	if claims, ok := newtoken.Claims.(jwt.MapClaims); ok && newtoken.Valid {
		fmt.Printf("[ INFO] Got claims '%v'\n", claims)
	} else {
		fmt.Printf("[ERROR] Invalid claims '%v'\n", err)
	}
}
{% endhighlight %}

### PyJWT

[PyJWT](http://pyjwt.readthedocs.io/en/latest/) 是一个用来编码和解码 JSON Web Tokens, JWT 的 Python 库，可以通过 `pip install pyjwt` 命令安装。

#### 生成 Token

需要使用 PyJWT 的 `encode()` 方法，需要传入三个参数：

{% highlight python %}
jwt.encode(payload, config.SECRET_KEY, algorithm='HS256')
{% endhighlight %}

上面代码的方法中传入了三个参数：A）payload 认证依据的主要信息；B) 密钥，这里是读取配置文件中的SECRET_KEY配置变量；C) 生成 Token 的算法。

注意，payload 是认证的依据，也是后续解析 token 后定位用户的依据，需要包含特定用户的特定信息，例如可以记录用户 ID 和登陆时间，其中 pyjwt 内置了几个声明：

{% highlight text %}
exp: 过期时间
nbf: 表示当前时间在nbf里的时间之前，则Token不被接受
iss: token签发者
aud: 接收者
iat: 发行时间
{% endhighlight %}



## RBAC

也就是 Role Based Access Control 基于角色的访问控制，RBAC 认为权限授权实际上是 Who、What、How 的问题，从而构成了访问权限三元组，也就是 "Who 对 What 进行 How 的操作"。

<!--
Who：权限的拥用者或主体（如Principal、User、Group、Role、Actor等等）
What：权限针对的对象或资源（Resource、Class）。
How：具体的权限（Privilege,正向授权与负向授权）。
基于角色的访问控制方法（RBAC）的显著的两大特征是：1.由于角色/权限之间的变化比角色/用户关系之间的变化相对要慢得多，减小了授权管理的复杂性，降低管理开销。2.灵活地支持企业的安全策略，并对企业的变化有很大的伸缩性。
-->


## 其它

### 关于 Token

近年来 RESTful API 开始风靡，使用 HTTP Header 来传递认证令牌似乎变得理所应当，而单页应用、前后端分离架构似乎正在促成越来越多的 WEB 应用放弃历史悠久的 Cookie-Session 认证机制，转而使用 JWT 来管理用户 Session。

#### 水平扩展

在 Cookie-Session 方案中，Cookie 内仅包含一个 SessionID ，而诸如用户信息、授权列表等都保存在服务端的 Session 中，如果把 Session 中的认证信息都保存在 JWT 中，在服务端就没有保存 Session 的必要了。

因此，当服务端水平扩展的时候，就不用处理 Session 复制或者引入外部 Session 存储了。

<!--
可以防止 CSRF 但是无法防止 XSS

https://www.jianshu.com/p/af8360b83a9f
-->


### Token Cookie Session

这三个概念经常出现，而且容易混淆。

#### Cookie

Cookie 总是保存在客户端中，是浏览器实现的一种数据保存方式，可以保存在内存或者磁盘中，前者由浏览器维护，关闭后就消失了，而后者通常会设置一个过期时间。

由服务器生成，发送给浏览器，浏览器把 Cookie 以 KV 形式保存到某个目录下的文本文件内，下一次请求同一网站时会把该 Cookie 发送给服务器。

由于 Cookie 是存在客户端上的，所以浏览器加入了一些限制确保 Cookie 不会被恶意使用，同时不会占据太多磁盘空间，所以每个域的 Cookie 数量是有限的。

#### Session

简单来说，客户端与服务器进行交互时，需要确认发送端是谁，为了做这种区分，服务器就要给每个客户端分配不同的 "身份标识"，然后客户端每次向服务器发请求的时候，都带上这个 "身份标识"，服务器就知道这个请求来自于谁了。

至于客户端怎么保存这个"身份标识"，可以有很多种方式，对于浏览器客户端，大家都默认采用 Cookie 的方式。

服务器使用 Session 把用户的信息临时保存在了服务器上，用户离开网站后 Session 会被销毁，这种用户信息存储方式相对 Cookie 来说更安全。

##### Token

也就是 "令牌" 是用户身份的验证方式，最简单的 Token 组成：uid(用户唯一的身份标识)、time(当前时间的时间戳)、sign(签名，由token的前几位+盐以哈希算法压缩成一定长的十六进制字符串，可以防止恶意第三方拼接 Token 请求服务器)，也可以把不变的参数放进 Token，避免多次查库。

JWT 就是 Token 的一种实现方式。

<!--
具体来说cookie机制采用的是在客户端保持状态的方案，而session机制采用的是在服务器端保持状态的方案。同时我们也看到，由于采用服务器端保持状态的方案在客户端也需要保存一个标识，所以session机制可能需要借助于cookie机制来达到保存标识的目的，但实际上它还有其他选择。
-->

## 参考

https://jwt.io/

{% highlight text %}
{% endhighlight %}
