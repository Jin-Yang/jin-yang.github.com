---
title: JavaScript 语法简介
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---

简单介绍 JavaScript 的核心语法，作为后续的参考手册。

<!-- more -->

## 简介

JavaScript 的核心语法部分相当精简，包括了：A) 基本的语法构造，也就算语言中标准的操作符、控制结构、语句等；B) 标准库，也就是对数据结构、算法的封装；C) 宿主环境提供的 API 接口，例如操作浏览器、DOM、WEB 等功能。

### 变量

{% highlight text %}
{% endhighlight %}

### 函数

() 单参数可以省略
{} 单返回可以省略

`...` 可以变参(参数收集)，数组展开(除了赋值)

## prototype


## this

在 Javascript 中 this 总是指向调用它所在方法的对象，this 是在函数运行时，自动生成的一个内部对象，只能在函数内部使用。

#### 1. 全局函数调用

{% highlight javascript %}
var name = "global this";
function globalTest() {
	this.name = "global this";
	console.log(this.name);
}
globalTest(); //global this
{% endhighlight %}

其中 `globalTest()` 是全局性的方法，属于全局性调用，因此 this 就代表全局对象 window。

#### 2. 对象方法调用

如果函数作为对象的方法调用，this 指向的是这个上级对象，即调用方法的对象。

{% highlight javascript %}
function showName() {
	console.log(this.name);
}
var obj = {};
obj.name = "object name";
obj.show = showName;
obj.show(); // object name
{% endhighlight %}

#### 3. 构造函数调用

构造函数中的 this 指向新创建的对象本身。

{% highlight javascript %}
function showName() {
	this.name = "showName function";
}
var obj = new showName();
console.log(obj.name); //showName function
{% endhighlight %}

通过 new 关键字创建一个对象的实例，将 this 指向对象 obj 。

#### 4. Call Apply Bind

三者在功能上没有区别，都是改变 this 的指向，其区别主要在于方法的实现形式和参数传递：

{% highlight text %}
function.call(object, arg1, arg2, ....)
function.apply(object，[arg1, arg2, ...])
var ss=function.bind(object, arg1, arg2, ....)
{% endhighlight %}

使用方式如下：

{% highlight javascript %}
function show(sex){
	console.log("function" + sex);
}
var person = {
	name: "your name",
	age: 14
};
show.call(person, "mail");
show.apply(person, ["femail"]);
var func = show.bind(person,"unknown");
func();
{% endhighlight %}


<!--
http://crockford.com/javascript/

https://github.com/getify/You-Dont-Know-JS


兼容性测试
http://kangax.github.io/compat-table/es6/
http://kangax.github.io/compat-table/es5/
-->

{% highlight text %}
{% endhighlight %}
