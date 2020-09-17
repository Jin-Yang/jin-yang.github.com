---
title: Jekyll Blog 的 SEO 优化技巧
layout: post
language: chinese
category: [misc]
keywords: jekyll,github,搭建,免费,代码高亮,评论
description:
---

所谓的 SEO 也就是搜索引擎优化，指遵循搜索引擎的搜索原理，对网站结构、网页文字语言和站点间互动外交略等进行合理规划部署，以改善网站在搜索引擎的搜索表现，进而增加客户发现并访问网站的可能性。

这里针对通过 Jekyll 搭建的 Blog 查看如何进行优化。

<!-- more -->

## 简介

SEO 的全称是 Search Engine Optimization 


在 Github 中，有一个 [jekyll-seo-tag](https://github.com/jekyll/jekyll-seo-tag) 插件可以参考，不过暂时没有采用，防止 Github 不兼容。

### 1. 标签优化

title 通常是标题名+网站名称；另外增加适当的 keywords、description、robots 等标签。

{% highlight html %}
<title>{% if page.title %}{{ page.title }} | {% endif %}JinYang's Blog</title>
<meta name="keywords" content="{{ page.keywords }}">
<meta name="description" content="{{ page.description }}">
{% endhighlight %}

### 2. 站长工具

通常有 [Google](http://www.google.com/webmasters/tools/)、[Baidu](http://zhanzhang.baidu.com/site/index) 等搜索引擎的站长工具，通常需要在网站的 ```<head> </head>``` 头部添加类似如下的特定元标记，用于标记确实是你网站，也就是用于认证。

{% highlight text %}
<meta name="baidu-site-verification" content="B786jeR0MV" />
{% endhighlight %}

这样就有利于搜索引擎的收录。

### 3. sitemap

sitemap 用于告知搜索引擎，在该网站上有哪些可供抓取的网页，以便搜索引擎可以更加智能地抓取网站。

最简单的就是 xml 文件，在其中列出网站中的网址以及关于每个网址的其他元数据，包括了上次更新的时间、更改的频率以及相对于网站上其他网址的重要程度等信息。

该文件设置的内容可以直接参考源码 sitemap.xml 。


<!--
1：博客要经常保持更新，文字始终紧紧围绕关键字，围绕网站主题。
2：SEO优化重点以文章内容页优化为主。
4：在新文章中适当增加一些老文章的链接，文章的内部链接一定要自然。
6：栏目分类尽量使用目录形式。如：http://lusongsong.com/youhua/而不是http://lusongsong.com/youhua.html
7：可以在网页底部加上站点地图sitemap，HTML格式是给用户看的，XML格式是给搜索引擎看的。
8：比较重要文章的URL可以使用拼音、英文和分割线等网址结构。
9：外部链接建设很重要，新博客推荐使用工具http://tool.lusongsong.com/seo/
10：垃圾留言一定要删除，非常影响排名。
11：记得给每个博文插图加上和文章内容相关的alt信息，并在图片周围添加相关信息。
12：文章中相同的关键词不能过多，如果非要堆砌关键词，可适当使用长尾关键词。
13：每篇文章的结尾处可加入“原创文章如转载，请注明出处”“本文首发于XXX网站”等信息，对SEO有一定帮助。
14：通过添加TAG标签可以增强主题的相关性和被搜索的概率。
16：文章标题最好能出现一次关键字。
17：关键字最好在第一段或最后一段能够出现。
-->


<!--
## 遗留问题
1. 图片如果设置固定的百分比宽度，可能会导致不同的拉伸，导致图片模糊。
2. 如果要在代码中不想进行渲染，应该如何处理。
-->



{% highlight text %}
{% endhighlight %}
