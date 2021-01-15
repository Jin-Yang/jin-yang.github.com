---
title: CSS 概念介绍
layout: post
comments: true
language: chinese
tag: [FrontEnd, Program]
keywords: 前端,SASS,CSS
description:
---


<!-- more -->

## 简介

Cascading Style Sheets, CSS 层叠样式表，定义了如何显示 HTML 中的元素，为了方便管理，通常会将样式表单独保存在 CSS 文件中，利用 CSS 可以很方便控制网页的样式和布局，会大大提升网页开发效率。

### 示例

以一个很简单的示例开始。

``` html
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>Hello CSS</title>
    <style>
    body {
      background-color:#d0e4fe;
    }
    h1 {
      color:orange;
      text-align:center;
    }
    p {
      font-family:"Times New Roman";
      font-size:20px;
    }
    </style>
  </head>
  <body>
  <h1>CSS 实例!</h1>
  <p>这是一个段落。</p>
  </body>
</html>
```

## 基本语法

CSS 规则由选择器以及声明组成，声明可以有多条，通过 `;` 分割；每条声明由一个属性和一个值组成。

![css format](/{{ site.imgdir }}/network/css/format.jpg "css format")

另外，可以通过 `/*` 和 `*/` 进行注释。

### 样式种类

插入样式表方法有三种：A) 外部样式表；B) 内部样式表；C) 内联样式。

#### 外部样式表

在需要多个页面时使用，可以通过改变一个文件来改变整个站点，需要在每个页面使用 `<link>` 标签链接到样式表文件。

``` html
<head>
  <link rel="stylesheet" type="text/css" href="style.css">
</head>
```

在浏览器渲染页面时，会从文件 `style.css` 中读到样式声明，例如如下的示例。

``` css
p {margin-left:20px;}
body {background-image:url("/images/backgroud.gif");}
```

注意，在属性值和单位之间不要留空格，例如 `margin-left: 20 px` 是有问题的。

#### 内部样式表

对单个页面需要特殊样式时，就可以使用内部样式表，也就是在头部使用 `<style>` 标签进行声明。

``` html
<head>
  <style>
    p {margin-left:20px;}
    body {background-image:url("images/backgroud.gif");}
  </style>
</head>
```

#### 内联样式

在具体的元素上添加，不过会损失灵活性，所以，不建议使用。

``` html
<p style="margin-left:20px;color:red">Hello World</p>
```

### 优先级

如果样式被多次定义，那么属性会从多个样式中继承过来；当存在重复时，会按照内联样式、内部样式、外部样式、浏览器默认样式的优先级覆盖。


``` css
h3 { /* 外部样式 */
    color:red;
    font-size:8pt;
}

h3 { /* 内部样式 */
    font-size:20pt;
}
```

那么最终得到的样式为。

``` css
color:red;
font-size:20pt;
```

也就是说，颜色继承于外部样式表，而字体大小会被内部样式表中的规则覆盖。

另外，注意，在上述的继承顺序中，需要保证外部样式在内部样式之前引入，否则，外部样式的优先级要高于内部样式。

``` html
<head>
  <link rel="stylesheet" type="text/css" href="style.css"/>
  <style type="text/css">
    h3{color:green;}
  </style>
</head>
```

## 选择器

最常见的就是元素选择器，也就是 DOM 中所使用的最原始元素。

``` css
html {color:black;}
h1 {color:blue;}
```

如果样式表中有很多样式的元素，那么可以使用分组选择器，每个选择器用逗号分隔。

``` css
h1, h2, p {color: green;}
```

还有一个比较特殊的通配符选择器，会直接匹配所有元素，例如 `*{color: red}` 会使每个元素都为红色。

### 类选择器

类选择器可以独立于文档元素存在，只要在元素中通过 `class="XXX"` 进行了标识，那么就可以通过类选择器修饰，当然，也可以在元素中指定多个类选择器，例如如下。

``` html
<html>
  <head>
    <style type="text/css">
      .important {font-weight:bold;}
      .warning {font-style:italic;}
      .important.warning {background:silver;}
    </style>
  </head>
  <body>
    <p class="important">This paragraph is very important.</p>
    <p class="warning">This is a warning.</p>
    <p class="warning important ">This paragraph is a very important warning.</p>
  </body>
</html>
```

其中元素中类的顺序没有关系，上述 `important` 类是粗体，`warning` 为斜体，而同时包含 `important` 和 `warning` 的所有元素添加一个红色背景。注意，除了这两个类之外，还可以有其它类，也是可以匹配的。

另外，通过 `p.marked { }` 为所有 `class="marked"` 的 `p` 元素指定一个样式。

### ID 选择器

在元素中通过 `id="XXX"` 使用，样式表中则使用 `#` 标识。

``` html
<html>
  <head>
    <style type="text/css">
      #intro{font-weight:bold;}
    </style>
  </head>
  <body>
  <p id="intro">This is a paragraph of introduction.</p>
  </body>
</html>
```

虽然在使用方法上两者基本类似，但是有如下区别：

* ID 只能使用一次，而且是仅一次，而类只要元素中存在，那么就会被渲染。
* ID 无法使用列表，因为属性不允许以空格分割的词列表。

所以，通常来说，ID 具有特殊的含义，而且于无需直到具体元素，例如标识一个 `important` ID ，不需要关心其对应的是 `p` `span` `h1` 等等。

### 属性选择器

根据 HTML 的属性名称以及值进行匹配。

``` html
<html>
  <head>
    <style type="text/css">
    [title] { color:red; }
    </style>
  </head>
  <body>
    <h2 title="Hello world">Hello world</h2>
  </body>
</html>
```

如上，只匹配属性的 Key 为 `title` 的元素，另外，还可以针对属性 Key/Value 分别进行匹配，常见的有如下几种方式。

| 选择器               | 描述                                                      |
|:--------------------:|:----------------------------------------------------------|
| `[attribute]`        | 选取带有指定属性的元素。                                  |
| `[attribute=value]`  | 选取带有指定属性和值的元素。                              |
| `[attribute~=value]` | 选取属性值中包含指定词汇的元素。                          |
| `[attribute|=value]` | 选取带有以指定值开头的属性值的元素，该值必须是整个单词。  |
| `[attribute^=value]` | 匹配属性值以指定值开头的每个元素。                        |
| `[attribute$=value]` | 匹配属性值以指定值结尾的每个元素。                        |
| `[attribute*=value]` | 匹配属性值中包含指定值的每个元素。                        |

### 后代/包含选择器

可以用来选择某元素后代的元素，例如只希望对 `h1` 元素中的 `em` 元素应用样式，那么可以写成 `h1 em {color:red;}` ，中间通过空格分开。

### 子元素选择器

通过大于号 `>` 分割，相比后代选择器，子元素选择器的范围更小，仅严格对某个元素的子元素生效。

``` html
<html>
  <head>
    <style type="text/css">
      h1 > strong {color:red;}
    </style>
  </head>
  <body>
    <h1>This is <strong>very</strong> <strong>very</strong> important.</h1>
    <h1>This is <em>really <strong>very</strong></em> important.</h1>
  </body>
</html>
```

其中只有第一行的内容，在 `<h1>` 之后会紧跟着 `<strong>` ，也就是子元素，那么会变红；而第二行，因为中间隔着 `<em>` ，所以 `<strong>` 实际是孙子元素，也就不会被渲染为红色。

也可以将后代和子选择器结合，例如 `table.company td > p` 。

### 相邻兄弟选择器

用来选择紧接某一元素后的元素，且二者有相同父元素，通过加号 `+` 标识。

``` html
<html>
  <head>
    <style type="text/css">
      h1 + p {color:red;}
    </style>
  </head>
    
  <body>
    <h1>Heading</h1>
    <p>Paragraph A</p>
    <p>Paragraph B</p>
    <p>Paragraph C</p>
  </body>
</html>
```

如上，只有 `Paragraph A` 作为相邻兄弟元素，会变为红色。

### 伪类

用于向某些选择器添加特殊的效果，最常用的就是超链接。

``` css
a:link    {color: #FF0000}   /* 未访问的链接     */
a:visited {color: #00FF00}   /* 已访问的链接     */
a:hover   {color: #FF00FF}   /* 鼠标移动到链接上 */
a:active  {color: #0000FF}   /* 选定的链接       */
```

<!--
### 伪元素

## 常用元素

CSS中，颜色值通常以以下方式定义:
    十六进制 - 如："#ff0000"
    RGB - 如："rgb(255,0,0)"
    颜色名称 - 如："red"

## 登陆

这个实际上是从 [Slick login form with HTML5 & CSS3](http://red-team-design.com/slick-login-form-with-html5-css3/) 参考的示例，该 blog 中有很多不错的前端设计示例，其效果可以直接参考 [login examle](/reference/linux/html_css/login/index.html) 。

{% highlight html %}
<form id="login">
    <h1>Log In</h1>
    <fieldset id="inputs">
        <input id="username" type="text" placeholder="Username" autofocus required>
        <input id="password" type="password" placeholder="Password" required>
    </fieldset>
    <fieldset id="actions">
        <input type="submit" id="submit" value="Log in">
        <a href="">Forgot your password?</a><a href="">Register</a>
    </fieldset>
</form>
{% endhighlight %}



根据最新的 HTML5 文档，其中的一些属性列举如下：

* placeholder 用来提示输入框中所需要输入的内容。

* required 表示当表单提交的时候，该输入框是必须的。

* autofocus 指定当加载之后自动聚焦到该输入框。

* type 指定类型，对于 password 来说，字段中的字符会被遮蔽；而 text 则会直接显示。





## 常见示例

* 弹出联系方式等，可以参考 [示例](/reference/linux/bootstrap/pop_toolbars/index.html) 。

http://www.cyzone.cn/a/20140113/248500.html     10个适合初学者的HTML5入门教程

自由分享jQuery、html5和css3的插件库
http://www.htmleaf.com/index.html
-->

## 其它

### @media

通常用在编写响应式应用的时候，在 `css2` 中已经支持 `@media` 属性，但实现功能比较少，通常仅用做打印时定制化。

``` css
@media screen {
  body{ font-size:12pt; }
}

@media print {
  @import "print.css"
  body{ font-size:8pt; }
}
```

而在 CSS3 中，增加了 media queries 功能，这样就可以根据具体的设备进行适配。

``` css
/* 宽度小于 500px 时显示绿色 */
@media screen and (max-width:500px) {
  body{ background:green; }
}
/* 宽度大于 800px 时显示红色 */
@media screen and (min-width:800px) {
  body{ background:red; }
}
```

详细可以查看 [Using Media Queries](https://developer.mozilla.org/en-US/docs/Web/CSS/Media_Queries/Using_media_queries) 中的介绍。


