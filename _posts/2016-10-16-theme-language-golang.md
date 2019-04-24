---
title: 【专题】GoLang 编程语言
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---

<!-- more -->

![Golang Logo]({{ site.url }}/images/go/golang-logo-3.jpg "Golang Logo"){: .pull-center width="350"}

Golang 从 2007 年末由 Robert Griesemer、Rob Pike、Ken Thompson 主持开发，后来还加入了 Ian Lance Taylor、Russ Cox 等人，最终于 2009 年 11 月开源，在 2012 年发布了稳定版本。

实际上，Golang 基于现有的技术实现，例如协程 (Coroutine)、IO 多路复用 (multiplexing)、异步 IO 等，然后在此之上进行了一些原语的封装。开始 Golang 包含了很多 C 语言代码，在 1.5 版本开始，包括运行时 (runtime)、编译器 (compiler)和连接器 (linker) 也都全部是由 Golang 所编写。

现在 Golang 的开发已经是完全开放的，并且拥有一个活跃的社区。简单来说，Golang 是一个开源、高并发、高效的编程语言，支持垃圾回收，具有很好的可伸缩性。

而且，越来越多的项目开始使用 Golang 进行开发，例如 Docker、LXD、InfluxDB、etcd 等等。另外，与 Golang 类似的高并发语言还可以参考 Rust、Elixir 。

## GoLang 语言

与 GoLang 相关的配置。

* [GoLang 简介](/post/golang-introduce.html)，介绍环境搭建、常用工具、单元测试、环境变量、三方包安装等。
* [GoLang 常用模块](/post/golang-common-module-introduce.html)，常见的三方模块使用，例如 log、unsafe 等。
* [GoLang 常用工具](/post/golang-some-third-tools.html)，常用的工具，包括了 GVM、GDM、Goreman 等。

### 常用模块

* [GoLang time 模块](/post/golang-common-module-time-introduce.html)，常见的三方模块使用，例如 log、unsafe 等。

### 语法相关

* [GoLang 语法简介](/post/golang-basic-syntax-introduce.html)，简单介绍常见的语法，例如 import、异常处理、反射等。
* [GoLang 并发控制](/post/golang-concurrenct-control-introduce.html)，常见的并发控制方法，例如 WaitGroup、Context 等。
* [GoLang 异常处理](/post/golang-basic-error-panic-introduce.html)，这是一个经常被讨论的话题，简单介绍其使用方法。
* [GoLang 语法之接口](/post/golang-syntax-interface-introduce.html)，相比来说接口要复杂很多，这里拆出来单独讨论。
* [GoLang 包管理](/post/golang-basic-package-introduce.html)，Go 的包管理简单明了，不过也有一定的规范。
* [GoLang 闭包简介](/post/golang-basic-closure-introduce.html)，与其它语言中的闭包概念相同，介绍下 GoLang 中的使用。

### 三方模块

* [GoLang gRPC 简介](/post/golang-grpc-introduce.html)，使用方式，包括了拦截器、负载均衡、REST 转换等。
* [GoLang HTTP 使用简介](/post/golang-net-http-webserver-introduce.html)，最常用的 HTTP 模块，介绍在 GoLang 的实现。
* [GoLang HTTP2 使用](/post/golang-net-http2-introduce.html)
* [GoLang DB 操作简介 gorm](/post/golang-db-introduce.html) 介绍关系型数据库相关的操作，包括基本操作以及 ORM 使用。
* [GoLang XORM 简介](/post/golang-db-orm-xorm-package-introduce.html) 一个非常简单的 ORM 工具包。
* [GoLang JSON 编码解码](/post/golang-json-encode-decode-introduce.html) 简单记录使用 JSON 的常用技巧。
* [GoLang Echo 简介](/post/golang-http-structure-echo-introduce.html) 一个 HTTP 的 Echo 框架，也可以使用 FastHTTP 作为底层。

### 其它

* [GoLang 如何编码？](/post/golang-how-to-write-go-code.html)，一篇官方文章的翻译，介绍如何进行编写代码。

## InfluxDB

一个开源分布式时序、事件和指标数据库。

* [InfluxDB 简介](/post/influxdata-influxdb-introduce.html)，简单介绍常见概念，如何安装，常用操作等。

## 参考

#### 文档

* [Network programming with Go](https://jan.newmarch.name/go/)，关于网络编程相关。
* [Go Resources](http://www.golang-book.com/)，有两本不错的书 An Introduction to Programming in Go、Introducing Go。
* [Go Bootcamp](http://www.golangbootcamp.com/)、[The Little Go Book](http://openmymind.net/The-Little-Go-Book/)、[Learning Go](https://www.miek.nl/go/)、[Building Web Apps With Go](https://www.gitbook.com/book/codegangsta/building-web-apps-with-go/details) 。

#### Awosome

* [Golang Standard Library by Example](https://github.com/polaris1119/The-Golang-Standard-Library-by-Example) Go 语言标准库的一些常见示例。
* [How to Build A Web with GoLang](https://github.com/astaxie/build-web-application-with-golang) 介绍如何通过 Go 语言构建 Web 应用。

#### 工具

* [Golang Security Checker](https://github.com/securego/gosec) Go 语言的静态语法检查。

#### 其它

由于官方网站被墙，可以访问如下网站。

* [Go 语言中文网](https://studygolang.com/topics) 包含了最新的安装包。

<!--
https://golang.org/ref/spec#Program_initialization_and_execution

Console progress bar for Golang
https://github.com/cheggaaa/pb

ASCII table in golang
https://github.com/olekukonko/tablewriter

JWT-GoLang
https://github.com/dgrijalva/jwt-go
-->

{% highlight text %}
{% endhighlight %}
