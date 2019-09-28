---
title: ResTest
layout: project
comments: true
language: chinese
category: [misc]
keywords:
description:
---

一个通过 Echo 框架搭建的 RestFull API 测试工具，类似于 JSONPlaceHolder 。



## 登陆

用户登陆通过 JWT 实现，在调用 API 接口时需要在头 `Authorization` 中添加 `Bearer TOKEN` 进行鉴权。

{% highlight text %}
----- 用户登陆
POST /v1/login
{
	"username":"t000001",
	"password":"Secret"
}

----- 获取当前登陆用户信息，需要鉴权
GET  /v1/user/current
{
}

----- 显示所有的用户信息，需要鉴权
/v1/user/info
{% endhighlight %}

在 JWT 的 Token 中会保存 uid、角色等相关的信息。

可以通过 curl 命令查看。

{% highlight text %}
curl -i -H 'Content-Type: application/json'  --data '{"username":"t000001", "password":"Secret"}' http://127.0.0.1:1323/v1/login
curl -X GET -i -H 'Authorization: Bearer TOKEN' http://127.0.0.1:1323/v1/user/current
curl -X GET -i -H 'Authorization: Bearer TOKEN' http://127.0.0.1:1323/v1/user/info
{% endhighlight %}

<!--
// 登陆成功
curl -i -H 'Content-Type: application/json'  --data '{"username":"t000001", "password":"Secret"}' http://127.0.0.1:1323/v1/login
curl -i --data 'username=bob&password=Secret' http://127.0.0.1:1323/login
// 登陆失败，秘密错误
curl -i -H 'Content-Type: application/json'  --data '{"username":"bob", "password":"wrong password"}' http://127.0.0.1:1323/v1/login
// 登陆失败，没有请求体
curl -X POST -i -H 'Content-Type: application/json' http://127.0.0.1:1323/v1/login
// 登陆失败，请求体为空
curl -i -H 'Content-Type: application/json'  --data '{}' http://127.0.0.1:1323/v1/login
// 登陆失败，密码错误
curl -i -H 'Content-Type: application/json'  --data '{"username":"bob", "password":""}' http://127.0.0.1:1323/v1/login

// 获取当前登陆用户信息
curl -X GET -i -H 'Authorization: Bearer TOKEN' http://127.0.0.1:1323/v1/user/info
-->

{% highlight text %}
{% endhighlight %}
