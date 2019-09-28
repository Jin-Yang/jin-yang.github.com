---
Date: October 19, 2013
title: CLucene
layout: post
comments: true
language: chinese
category: [linux]
---





Lucene 是 JAVA 版的全文检索引擎，而且据说性能相当出色，而 CLucene 是 C++ 版的全文检索引擎，完全移植于 Lucene，不过其缺点是不支持中文。



<!-- more -->





## 编译安装

可以直接从 [clucene.sourceforge.net](http://clucene.sourceforge.net/download.shtml) 上下载相应的源码，然后通过如下方式编译。

{% highlight text %}
----- 新建一个单独的目录进行编译
$ mkdir build && cd build

----- 配置生成Makefile
$ cmake ../clucene-code

----- 直接编译，生成的文件保存在bin目录下
$ make

----- 编译生成测试程序cl_test
$ make cl_test

----- 以及示例程序
$ make cl_demo
{% endhighlight %}

<!--
很不错的文章
http://www.cnblogs.com/forfuture1978/archive/2010/06/13/1757479.html

SCWS
-->
