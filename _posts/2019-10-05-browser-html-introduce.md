---
title: HTML 概念介绍
layout: post
comments: true
language: chinese
tag: [FrontEnd, Program]
keywords:
description:
---

<!-- more -->

## 简介

Hyper Text Markup Language, HTML 超文本标记语言，不是编程语言，而是一种标记语言，通过一堆的标签来描述网页。标签通过尖括号包裹，例如 `<h1>`、`<br/>` 等，而且一般是成对出现的 `<p>...</p>` ，包括了开始以及结束标签。

### 元素

HTML 文件是由 HTML 元素组成，所谓的元素，包括了从开始标签到结束标签的所有内容，例如如下的最简单页面。

``` html
<html>
  <body>
    <p>This is my first paragraph.</p>
  </body>
</html>
```

其中包括了三个元素，分别为：

* `p` 元素，也就是 `<p>This is my first paragraph.</p>` 。
* `body` 元素，对应了 `<body><p>This is my first paragraph.</p></body>` 。
* `html` 元素，所有的内容。

一般标签中也可以包含属性。

### 属性

HTML 标签可以拥有属性，提供与 HTML 元素相关更多的信息，必须以 Key/Value 的方式出现，而且，是定义在开始标签。

``` html
<h1 align="center">This is a heading</h1>
<a href="http://www.google.com">This is a link</a>
```

最常用的属性有如下几种：

* `class` 元素的类名。
* `id` 元素的唯一标识。
* `style` 元素的行内样式。
* `title` 元素的额外信息，通常可以在工具提示中显示。



