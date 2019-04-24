---
title: GoLang XORM 简介
layout: post
comments: true
language: chinese
category: [program,golang,linux]
keywords: mysql,sql,golang,orm,xorm
description:
---


<!-- more -->

## 名称映射

用于结构体名称到表名、结构体 field 到表字段的名称映射，xorm 内置了三种实现 `core.SnakeMapper` `core.SameMapper` `core.GonicMapper` ，默认是 `SnakeMapper` 可以通过如下方式修改：

{% highlight text %}
engine.SetMapper(core.SameMapper{})
{% endhighlight %}




http://www.xorm.io/

{% highlight text %}
{% endhighlight %}
