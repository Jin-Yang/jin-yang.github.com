---
title: Flask-React 实践
layout: post
comments: true
language: chinese
category: [program,linux]
keywords: flask,react
description:
---

这里实际上是参考 《实例讲解基于 Flask+React 的全栈开发和部署》中的介绍。

<!-- more -->

{% highlight text %}
----- 0. 如果不存在则提前安装，webpack全局安装
$ pip install virtualenv
# npm install -g webpack webpack-cli

----- 1. 新建一个虚拟的Python环境，并切换到新环境中，同时切换到源码目录
$ virtualenv --no-site-packages foobar
$ source foobar/bin/activate
$ cd your-project-source-dir
$ pip install -r requirements.txt

----- 2. 安装npm依赖，默认安装到当前目录的node_modules目录下
$ npm install
$ export PATH=`pwd`/node_modules/bin:$PATH

----- 3. 执行npm的启动
$ npm start
{% endhighlight %}






## 使用

这里通过 Socket.IO ZeroMQ 等搭建一个高效的。


https://github.com/gabrielfalcao/flask-react-bootstrap





## 参考

直接参考的 [实例讲解基于 Flask+React 的全栈开发和部署](http://funhacks.net/2016/12/06/flask_react_news/) 中的内容。



<!--
https://github.com/ethan-funny/React-News-Board
https://github.com/gabrielfalcao/flask-react-bootstrap  支持SocketIO

Flask-JWT 用户认证
-->


{% highlight text %}
{% endhighlight %}
