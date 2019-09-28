---
Date: October 19, 2013
title: OpenStack Keystone 服务
layout: post
comments: true
language: chinese
category: [network]
---



<!-- more -->









业务端口：5000
管理端口：35357
2、业务API 测试：

####


获取api扩展：
curl http://0.0.0.0:35357/v2.0/extensions | python -mjson.tool

#### 获取版本号

{% highlight text %}
----- 获取管理API的版本号
$ curl -s http://127.1:5000/ | python -mjson.tool
$ curl -s http://127.1:5000/v2.0/ | python -mjson.tool

----- 获取管理API的版本号
$ curl -s http://127.1:35357/ | python -mjson.tool
$ curl -s http://127.1:35357/v2.0/ | python -mjson.tool
{% endhighlight %}

#### 获取 API 扩展

{% highlight text %}
$ curl -s http://127.1:5000/v2.0/extensions | python -mjson.tool
{% endhighlight %}

#### 用普通用户登录

{% highlight text %}
$ curl -s -X POST -d '{"auth": {"passwordCredentials":{"username": "admin", "password": "123"}}}' \
 -H "Content-type: application/json" http://127.1:5000/v2.0/tokens | python -mjson.tool
{
    "access": {
        ... ...
        "token": {
            "id": "0d71cf53ad2d4bf5aa49296ffdd12a2f",
        },
        ... ...
    }
}

{% endhighlight %}


#### 查看自己的租户

{% highlight text %}
$ curl -s -H "X-Auth-Token:0d71cf53ad2d4bf5aa49296ffdd12a2f" http://127.1:5000/v2.0/tenants | python -mjson.tool
{
    "tenants": [
        {
            "description": "Admin Project",
            "enabled": true,
            "id": "f3f0c9353afc4a1ab74d143f85dfabe4",
            "name": "admin"
        }
    ],
    "tenants_links": []
}

{% endhighlight %}


#### 用角色admin登录

{% highlight text %}
$ curl -s -X POST -d '{"auth": {"tenantId": "f3f0c9353afc4a1ab74d143f85dfabe4",
  "passwordCredentials":{"username": "admin", "password": "123"}}}' \
  -H "Content-type: application/json" http://127.1:35357/v2.0/tokens | python -mjson.tool
{
    {
        ... ...
        "token": {
            "id": "0db5896265f1446c99fc79c1f99da121",
            ... ...
        },
        ... ...
    }
}

{% endhighlight %}

#### 校验token的有效，并返回token的信息

{% highlight text %}
$ curl -s -H "X-Auth-Token: 0db5896265f1446c99fc79c1f99da121" \
  http://127.1:35357/v2.0/tokens/0db5896265f1446c99fc79c1f99da121 | python -mjson.tool
{% endhighlight %}

#### 使用HEAD校验，如果返回码是20X，表示token有效

{% highlight text %}
$ curl -I -H "X-Auth-Token: 0db5896265f1446c99fc79c1f99da121" \
  http://127.1:35357/v2.0/tokens/0db5896265f1446c99fc79c1f99da121
{% endhighlight %}

#### 查看endpoints

{% highlight text %}
$ curl -H "X-Auth-Token: 0db5896265f1446c99fc79c1f99da121" \
  http://127.1:35357/v2.0/tokens/0db5896265f1446c99fc79c1f99da121/endpoints
{% endhighlight %}

#### 返回租户

{% highlight text %}
$ curl -H "X-Auth-Token: 0db5896265f1446c99fc79c1f99da121" \
  http://0.0.0.0:35357/v2.0/tenants|python -mjson.tool
{% endhighlight %}

#### 返回某个租户

{% highlight text %}
$ curl -H "X-Auth-Token: 0db5896265f1446c99fc79c1f99da121" \
  http://0.0.0.0:35357/v2.0/tenants/6a524dbe23dd4e4ab672cd163c85a27d |python -mjson.tool
{% endhighlight %}

#### 返回用户

curl -H “X-Auth-Token:5a10b008add4435f8473d2b11d3ba8a8” http://0.0.0.0:35357/v2.0/users|python -mjson.tool

#### 返回某个用户

curl -H “X-Auth-Token:5a10b008add4435f8473d2b11d3ba8a8” http://0.0.0.0:35357/v2.0/users/3ff8fbca9794436c996d8c6e41427530|python -mjson.tool

#### 返回某个租户上，用户授予的角色

curl -H “X-Auth-Token:5a10b008add4435f8473d2b11d3ba8a8” http://0.0.0.0:35357/v2.0/tenants/6a524dbe23dd4e4ab672cd163c85a27d/users/3ff8fbca9794436c996d8c6e41427530/roles |python -mjson.tool

返回某个用户的角色：（出错，没有实现，参见 https://bugs.launchpad.net/keystone/+bug/933565)
curl -H “X-Auth-Token:5a10b008add4435f8473d2b11d3ba8a8” http://0.0.0.0:35357/v2.0/users/3ff8fbca9794436c996d8c6e41427530/roles







[Keystone, the OpenStack Identity Service](http://docs.openstack.org/developer/keystone/)

[Python bindings to the OpenStack Identity API (Keystone)](http://docs.openstack.org/developer/python-keystoneclient/)
{% highlight text %}
{% endhighlight %}
