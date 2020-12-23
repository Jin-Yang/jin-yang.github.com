---
title: Flask Restful API
layout: post
comments: true
language: chinese
category: [webserver]
keywords: flask,restful,api,pyresttest
description:
---

这里简单介绍 Flask 中如何使用 Restful-API 以及相关的 REST-API 测试工具。

<!-- more -->

![logo]({{ site.url }}/images/python/restful-api-logo.png "logo"){: .pull-center width="70%" }

## Flask-Restful

可以通过如下方式安装。

{% highlight text %}
----- 安装最新版本
# pip install flask-restful

----- 手动下载安装指定版本
$ git clone https://github.com/flask-restful/flask-restful.git
$ cd flask-restful
# python setup.py develop
{% endhighlight %}

## 测试

这里使用的是 [Pyresttest](https://pypi.python.org/pypi/pyresttest/)，该工具依赖 pycurl ，对于 CentOS 可以通过 `yum install python-pycurl` 命令安装。

{% highlight text %}
---
- config:
    - testset: "Basic tests"
    - timeout: 100  # Increase timeout from the default 10 seconds
- test:
    - name: "Basic get"
    - url: "/api/person/"
- test:
    - name: "Get single person"
    - url: "/api/person/1/"
- test:
    - name: "Delete a single person, verify that works"
    - url: "/api/person/1/"
    - method: 'DELETE'
- test: # create entity by PUT
    - name: "Create/update person"
    - url: "/api/person/1/"
    - method: "PUT"
    - body: '{"first_name": "Gaius","id": 1,"last_name": "Baltar","login": "gbaltar"}'
    - headers: {'Content-Type': 'application/json'}
    - validators:  # This is how we do more complex testing!
        - compare: {header: content-type, comparator: contains, expected:'json'}
        - compare: {jsonpath_mini: 'login', expected: 'gbaltar'}  # JSON extraction
        - compare: {raw_body:"", comparator:contains, expected: 'Baltar' }  # Tests on raw response
- test: # create entity by POST
    - name: "Create person"
    - url: "/api/person/"
    - method: "POST"
    - body: '{"first_name": "William","last_name": "Adama","login": "theadmiral"}'
    - headers: {Content-Type: application/json}
{% endhighlight %}

如下的示例以 Github 提供的 API 为例，也就是 [https://api.github.com](https://api.github.com) 。

{% highlight text %}
$ cat tests/restapi/github_smoketest.yaml
---
- config:
    - testset: "Quickstart app tests for github"

- test:
    - name: "Basic smoketest"
    - url: "/"

----- 运行一个github API的基本测试case
$ pyresttest https://api.github.com tests/restapi/github_smoketest.yaml

----- 打印返回的消息
$ pyresttest --print-bodies true https://api.github.com tests/restapi/github_smoketest.yaml

----- 打印详细日志
$ pyresttest --log debug https://api.github.com tests/restapi/github_smoketest.yaml
{% endhighlight %}

更多的示例可以参考 [Github Pyresttest examples](https://github.com/svanoort/pyresttest/blob/master/quickstart.md) 。

## 参考

文档可以参考 [Flask Restful](http://flask-restful.readthedocs.io/en/latest/)，或者 [中文文档](http://www.pythondoc.com/Flask-RESTful/quickstart.html) 。

另外的 REST API 测试工具可以参考 [Hitchhicker](https://github.com/brookshi/Hitchhiker) 。

{% highlight text %}
{% endhighlight %}
