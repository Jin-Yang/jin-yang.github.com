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

变量通过 `var name = value` 的方式定义，在 JavaScript 中，是可变类型，也就是说，变量对应的类型可以随时改变。

``` javascript
var foobar;                // undefined
foobar = 5;                // 类型为数字
foobar = "Hello World!";   // 类型为字符串
```

值包含了六种基本类型：字符串 (String)、数字 (Number)、布尔 (Boolean)、空 (Null)、未定义 (Undefined)、Symbol。其中，Symbol 是 ES6 引入的一种新的类型，标识独一无二的值。

另外，还包含了三种引用数据类型：对象 (Object)、数组 (Array)、函数 (Function) 。

``` javascript
//----- 字符串
foobar = "Hello World!";   // 可以使用单引号或者双引号
foobar = "Hey, I'm Andy."; // 使用双引号内部可以用单引号，反之亦然

//----- 数字
foobar = 123;              // 不区分整型、浮点，统一采用64Bits浮点表示
foobar = 3.1415926;        // 浮点数
foobar = 1e2;              // 科学记数法，也就是100 123e-3 --> 0.123

//----- 布尔
foobar = true;             // 只能是真 (true)、假 (false)


//----- 数组
foobar = new Array();      // 可以新建数组然后按照序号赋值
foobar[0] = "Hey";
foobar[1] = 3.1415926;
foobar = new Array("Hey", 3.14);
foobar = ["Hey", 3.14];

//----- 对象
foobar = {name:"Andy", age:10};  // 可以通过 foobar.name 或者 foobar["name"] 访问
```

而 `undefined` 是变量声明但未定义时的值，而 `null` 表示为空值，所以，除了 `undefined` `null` `symbol` 这三种特殊类型，就剩下了五种，可以通过如下方式直接声明。

``` javascript
var foobar = new String;
var foobar = new Number;
var foobar = new Boolean;
var foobar = new Array;
var foobar = new Object;
```

#### 对象

对象也是最为常用的类型，在定义时比较容易出现歧义，例如 `{ foobar: 123 }` 可以将 `foobar` 看作代码块中的标签，而 `123` 作为语句。在定义对象时，最好在大括号外加上圆括号，因为圆括号里只能是表达式，可以确保大括号只能解释为对象，例如 `({ foo: 123 })` 。



### 函数

通过 `function` 关键字声明一个函数，也可以赋值给变量。

``` javascript
function foobar() {
	alert("Hello World");
}
```


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
-->

## 参考

* 一些常见的参考 [kangax.github.io](http://kangax.github.io/) ，例如 [ES6 兼容性表格](http://kangax.github.io/compat-table/es6/) 。

{% highlight text %}
{% endhighlight %}
