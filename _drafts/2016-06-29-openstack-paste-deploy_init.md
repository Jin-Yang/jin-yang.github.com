---
Date: October 19, 2013
title: OpenStack 基础之 Paste-Deploy
layout: post
comments: true
language: chinese
category: [network]
---

WSGI (Web Server Gateway Interface) 是 Python 应用程序和 Web 服务器之间定义的一套用来通讯的规范或者标准；按照这套规范，应用程序的实现以及修改将会非常简单。

Python paste 是一个 WSGI 工具包，在 WSGI 的基础上包装了几层，让应用管理和实现变得更加方便。

<!-- more -->

# 简介

通过 WSGI 标准，应用端只需要实现一个可以接收两个参数的可调用






## 执行过程

在配置文件中，通过 use 用来定义使用的方法，确切来说就是调用在 /usr/lib/python2.7/site-packages 目录下的 paste/urlmap.py 模块。

另外，Python paste.deploy 中的 filter、filter_factory、app、app_factory 都是一个 callable object，也就是可调用的对象，只是其接口有些区别。

* app 接受 WSGI 规范的参数 (environ, start_response)，这是由 paste 系统交给 app 的。app 需要响应 envrion 中的请求，准备好响应头和消息体，然后交给 start_response 处理，并返回响应消息体。

* filter 唯一参数是 (app)，这是 WSGI 的 application 对象。filter 需要完成的工作是中间做些过滤操作，将 app 包装成另一个 app，并返回这个包装后的 app 。

* app_factory 接受的参数是一些关于 app 的配置信息 (global_conf, **kwargs)，其中 global_conf 是在 ini 文件中 default section 中定义的一系列 key-value，而 **kwargs 是一些本地配置，在 ini 文件中相应 app:xxx section 中定义的一系列 key-value；其返回值是一个 app 对象。

* filter_factory 与 app_factory 接收的参数相似，只是其会返回一个 filter 对象。

## 配置文件

配置段设置。

* DEFAULT section，这个是全局的默认配置。

* composite section，用于在一个配置文件中定义多个应用入口，其中 use=egg:Paste#urlmap 表明使用 Paste 安装包中 urlmap，这是一个通用的应用，主要是将 URL 映射到具体的应用。

* pipeline section，由一系列的 filter 以及最后的 app 组成，前面是做一系列的过滤。

如下的是配置文件 foobar.ini 。

{% highlight text %}
[DEFAULT]
key=value

[composite:foobar]
use = egg:Paste#urlmap
/       :root
/double :double

[pipeline:root]
pipeline = logrequest showversion

[pipeline:double]
pipeline = logrequest calculate

[filter:logrequest]
paste.filter_factory = foobar:LogFilter.factory

[app:showversion]
version = 1.0.0
paste.app_factory = foobar:ShowVersion.factory

[app:calculate]
description = Just double it
paste.app_factory = foobar:Calculator.factory
{% endhighlight %}


## 程序示例

如下是 foobar.py 文件，其中 factory 实现的函数可以是 classmethod 或者 staticmethod，两者都会在加载类的时候加载，也就是说其中的 print 只会在初始化的时候执行。

另外，需要注意的是 loadapp() 使用的是绝对路径，可以使用 os.path.abspath() 获得。

{% highlight python %}
import os
import webob
from webob import Request
from webob import Response
from paste.deploy import loadapp
from wsgiref.simple_server import make_server

#Filter
class LogFilter():
    def __init__(self, app):
        self.app = app
    def __call__(self, environ, start_response):
        print "filter:LogFilter is called."
        return self.app(environ, start_response)
    @classmethod
    def factory(cls, global_conf, **kwargs):
        print "in LogFilter.factory", global_conf, kwargs
        return LogFilter

# Application
class ShowVersion():
    def __init__(self, version):
        self.version = version
    def __call__(self, environ, start_response):
        start_response("200 OK",[("Content-type", "text/plain")])
        return ["FOOBAR: Version = %s\r\n" % self.version, ]
    @staticmethod
    def factory(global_conf, **kwargs):
        print "in ShowVersion.factory", global_conf, kwargs
        return ShowVersion(kwargs["version"])

# Application
class Calculator():
    def __call__(self, environ, start_response):
        req = Request(environ)
        #print req.GET       # Just for test

        res = Response()
        res.status = "200 OK"
        res.content_type = "text/plain"

        # get arguments
        name = req.GET.get("name", None)
        value = req.GET.get("arg", None)

        res.text = "Hi %s, RESULT = %d\r\n" % (name, 2*int(value))
        return res(environ, start_response)

    @classmethod
    def factory(cls,global_conf,**kwargs):
        print "in Calculator.factory", global_conf, kwargs
        return Calculator()

if __name__ == '__main__':
    wsgi_app = loadapp("config:%s" % os.path.abspath("foobar.ini"), "foobar")
    server = make_server('localhost', 8080, wsgi_app)
    server.serve_forever()
{% endhighlight %}

接下来可以直接通过如下方式进行测试。

{% highlight text %}
$ curl http://127.0.0.1:8080/
FOOBAR: Version = 1.0.0

$ curl 'http://127.1:8080/double?name=foo-bar&arg=2'
Hi foo-bar, RESULT = 4
{% endhighlight %}

假设 pipeline 的顺序是 filter1, filter2, filter3，那么 app 会依次调用 filter1.\_\_call\_\_(env, start_response) 被首先调用，如果条件通过，则继续执行 filter2.\_\_call\_\_(env, start_response) ... ...






# 参考

[Paste Deployment](http://pythonpaste.org/deploy/)

[Keystone, the OpenStack Identity Service](http://docs.openstack.org/developer/keystone/)

[Understanding OpenStack Authentication: Keystone PKI](https://www.mirantis.com/blog/understanding-openstack-authentication-keystone-pki/)

[理解 Keystone 的四种 Token](http://www.openstack.cn/?p=5120)

[keystone源码分析（一）——Paste Deploy的应用](http://www.cnblogs.com/Security-Darren/p/4058148.html)

[keystoneclient分析](http://www.cinlk.com/2015/07/26/keystoneclient/)


{% highlight text %}
{% endhighlight %}
