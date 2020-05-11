---
title: GoLang Echo 简介
layout: post
comments: true
language: chinese
category: [program,golang,linux]
keywords: echo,golang,http
description:
---


<!-- more -->

![echo introduce]({{ site.url }}/images/go/echo-introduce.png "echo introduce"){: .pull-center width="80%" }


## 路由

Echo 的路由基于 Radix Tree ，它让路由的查询非常快，而且路由使用了 Sync Pool 来重复利用内存并且几乎达到了零内存占用。

## 中间件

中间件是一个函数，嵌入在 HTTP 的请求和响应之间，可以获得 Echo#Context 对象用来进行一些特殊的操作， 比如记录每个请求或者统计请求数。

Action的处理在所有的中间件运行完成之后。

### Pre

`Pre()` 用于注册一个在路由执行之前运行的中间件，可以用来修改请求的一些属性，比如在请求路径结尾添加或者删除一个 `/` 来使之能与路由匹配。

<!--
下面的这几个内建中间件应该被注册在这一级别：

AddTrailingSlash
RemoveTrailingSlash
MethodOverride
-->

注意: 由于在这个级别路由还没有执行，所以这个级别的中间件不能调用任何 `echo.Context` 的 API。

## 其它

### 参数获取

#### 表单

HTML 提供的默认表单，可以使用如下方式。

{% highlight text %}
curl -X POST http://127.1:1323/v1/hello -d 'name=Andy' -d 'email=andy@foobar.com'
{% endhighlight %}

在 echo 中可以通过 `c.FormValue("name")` 获取。

#### 参数

在 URL 中，直接以 `?name=Andy&email=andy@foobar.com` 格式提交请求，例如：

{% highlight text %}
curl -X GET 'http://127.1:1323/v1/hello?name=Andy&email=andy@foobar.com'
{% endhighlight %}

在 echo 中可以通过 `c.QueryParam("name")` 获取。

#### JSON

也就是发送的数据为 JSON 格式。

{% highlight text %}
curl -X POST http://127.1:1323/v1/hello -H 'Content-Type: application/json' \
	-d '{"name":"Andy","email":"andy@foobar.com"}'
{% endhighlight %}

可以通过如下方式读取并返回 JSON 数据格式。

{% highlight text %}
msg, err := ioutil.ReadAll(r.Body)
if err != nil {
	return err
}

if len(msg) == 0 {
	return c.String(http.StatusOK, "Hello, World!")
}

var body map[string]interface{}
if err = json.Unmarshal(msg, &body); err != nil {
	return err
}
body["method"] = r.Method
return c.JSON(http.StatusOK, body)
{% endhighlight %}

#### Bind

可以通过 Bind 函数直接将接收到的数据进行转换，默认的会转换 json、xml、表单、URL 等格式的数据，也可以自己定义。



## 参考

官网 [echo.labstack.com](https://echo.labstack.com/) 有很详细的示例以及文档，也可以参考中文文档 [go-echo.org](http://go-echo.org/)。

<!--
使用 FastHTTP 作为底层的 HTTP 处理，速度要快很多
https://github.com/webx-top/echo
-->

{% highlight text %}
{% endhighlight %}
