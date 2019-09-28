---
title: jquery 常用组件
layout: post
comments: true
language: chinese
category: [webserver]
keywords: hello world,示例,sample,markdown
description: 简单记录一下一些与 Markdown 相关的内容，包括了一些使用模版。
---



<!-- more -->

## NPM

这是 JavaScript 的一个包管理工具，在 CentOS 中，可以通过 yum 进行安装。

{% highlight text %}
$ yum --enablerepo=epel install npm
{% endhighlight %}

<!-- https://github.com/npm/npm -->

## dygraphs

![dygraphs example]({{ site.url }}/images/webserver/dygraphs_example.png "dygraphs example"){: .pull-center width="80%"}


<!--
http://dygraphs.com/
http://dygraphs.com/options.html
-->

## DataTables

一个很强大的表格解决方案，而且文档非常全。

<!--
http://www.cnblogs.com/jobs2/p/3431567.html
http://blog.5ibc.net/p/6097.html
-->




## 参考

DataTables 很多不错的示例，可以参考 [Examples index](https://datatables.net/examples/index) 。

在一行中添加默认值 [Generated content for a column](https://datatables.net/examples/ajax/null_data_source.html) 。

介绍了每一列中如何设置一些属性，如可以排序、可以搜索、渲染等，可以参考 [columnDefs](https://datatables.net/reference/option/columnDefs) 。

通过 AJAX 重新加载数据 [ajax.reload()](https://datatables.net/reference/api/ajax.reload%28%29) 。

自带的 buttons 示例，很不错 [Buttons for DataTables](https://datatables.net/extensions/buttons/examples/) 。



{% highlight python %}
{% endhighlight %}
