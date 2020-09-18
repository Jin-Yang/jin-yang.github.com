---
title: Jekyll Blog 的 SEO 优化技巧
layout: post
language: chinese
category: [misc]
keywords: jekyll,blog,seo,优化
description: 针对 Jekyll 搭建的 Blog ，详细介绍如何通过一些常用的技巧进行优化，以优化搜索引擎对网站的收录以及排名，从而有效提高曝光率。
---

所谓的 SEO 也就是搜索引擎优化，指遵循搜索引擎的搜索原理，对网站结构、网页文字语言和站点间互动外交略等进行合理规划部署，以改善网站在搜索引擎的搜索表现，进而增加客户发现并访问网站的可能性。

这里针对通过 Jekyll 搭建的 Blog 查看如何进行优化。

<!-- more -->

## 简介

搜索引擎会通过 Robots 尽量扫描所有的网站，然后将数据保存下来，当用户搜索的时候会从保存的数据中查询相关的记录，同时根据一些参数对结果进行排名。

SEO 的全称是 Search Engine Optimization 也就是根据搜索引擎的排名策略，对网站进行优化，大部分的优化 Tag 都需要添加到网站的 `<head>` 字段内。

如上，那么 SEO 的关键是，如何被各个搜索网站有效抓取，同时提高搜索排名。以 Google 为例，可以使用 `site: yourdomain.com` 查看是否被 Google 抓取；不过其排名的参数有两百多个，很难全部覆盖，如下介绍常见的优化技巧。

## 标题和描述

搜索结果中会显示对应标题和描述内容，如果没有指定，一般会自动选择第一段内容 (当然不一定严格准确)。所以，**标题尽量简单吸引人，而描述可以简要介绍这篇文章的概要内容、要解决的问题等**。

### 标题

建议在 `_config.yml` 中添加 `title` 配置项，这样就可以通过 `site.title` 直接引用，同时方便修改；而在文章的头部，通过 `title` 指定文章相关标题，那么就可以设置如下内容。

{% raw %}
```
<title>
   {%if page.title %}
       {{ page.title }}
   {% else %}
       {{ site.title }}
   {% endif %}
</title>
```
{% endraw %}

在头部中，尽量包含与文章相关的关键词，这样更容易被选中。

### 描述

与上面类似，在 `_config.yml` 中添加主页相关的描述信息，在每篇文章的头部添加与文章相关的描述信息。长度尽量小于 160 字节，否则可能会被识别为垃圾信息或者关键字。

{% raw %}
```
<meta itemprop="description" name="description" content="{% if page.description %}{{ page.description | truncate: 160 }}{% else %}{{ site.description | truncate: 160  }}{% endif %}" />
```
{% endraw %}

描述只会用来在搜索结果中展示，并不会参与排名，但是仍然建议在开始几句话中包含相关的关键字，这样可以方便用户查阅，从而增加用户的点击率。

另外 `<meta name="keywords" content="XXX" />` 对于 Google 来说已经取消了参考该因子作为排名，主要是因为用户可以任意修改，可能会污染排名结果，详见 [Google does not use the keywords meta tag in web ranking](https://webmasters.googleblog.com/2009/09/google-does-not-use-keywords-meta-tag.html) 中的介绍。

## URL 路径

尽量在 URL 中包含与主题相关的信息，一些搜索引擎可以判断 URL 是否与主题相关，这样可以有更好的搜索排名，所以，不建议使用文章 ID 作为 URL 的内容。

在 Jekyll 中，默认路径会使用到日期，例如 `https://yourname.github.io/2010/08/15/your-post-name.html` ，其实，除非文章真正与时间相关 (例如游记、周更漫画等) 不建议添加日期。

可以在 `_config.yml` 中修改为 `permalink: /:title/` 或者 `permalink: /:title.html` 即可。

### 减少使用停用词

所谓的停用词 (Stop-Words) 常见的有 `and` `or` `but` `of` `the` `a` `etc` ，这些会增加 URL 的长度，而且没有太多实际含义。例如 `optimize-jekyll-blog-seo.md` 和 `how-to-optimize-jekyll-blog-for-seo.md` 基本可以表达相同的意思，而前者更加简练。

另外，不太建议使用中文路径，因为复制之后链接地址会被转义，很难看出具体是什么样的链接。

所以，**URL 应该尽量简短，而且要包含与文章主题相关的关键字**。

## 添加 Sitemap

一般是 xml 格式，通过 `sitemap.xml` 用于告知搜索引擎，在该网站上有哪些可供抓取的网页，以便搜索引擎可以更加智能地抓取网站。

最简单的就是 xml 文件，在其中列出网站中的网址以及关于每个网址的其他元数据，包括了上次更新的时间、更改的频率以及相对于网站上其他网址的重要程度等信息。

可以通过 `jekyll-sitemap` 插件自动生成，也可以手动添加，该文件设置的内容直接参考如下，可以查看 [本站 sitemap.xml](/sitemap.xml) 。

{% raw %}
``` text
---
---
<?xml version="1.0" encoding="UTF-8"?>
<urlset xmlns="http://www.sitemaps.org/schemas/sitemap/0.9">
{% for post in site.posts %}
    <url>
      <loc>{{ site.url }}{{ post.url | remove: 'index.html' }}</loc>
    </url>
{% endfor %}
</urlset>
```
{% endraw %}

生成之后，搜索机器人会定期查找，当然也可以通过 [Google Search Console](https://www.google.com/webmasters/tools/)、[百度搜索资源平台](https://ziyuan.baidu.com/site) 提交。

<!-- 可以通过 https://toolbox.seositecheckup.com/apps/seo-checkup 网站查看 -->

## 增加 Open Graph

所谓的 Open Graph Protocol 开放内容协议，是由 Facebook 发起的，用来更好的理解一个链接，因为在分享链接时，可能会因为各种原因导致一些有效信息丢失，也可能会有异常数据。


{% raw %}
```
<meta property="og:site_name" content="WebJeda Blog">
<meta property="og:type" content="article">
<meta property="og:url" content="{{ site.url }}{{ page.url }}">
<meta property="og:title" content="{%if page.title %}{{ page.title }}{% else %}{{ site.title }}{% endif %}">
<meta property="og:description" content="{% if page.description %}{{ page.description }}{% else %}{{ site.description }}{% endif %}">
```
{% endraw %}

<!--
<meta property="og:locale" content="en_US">
<meta property="og:image" content="{{ site.url }}/thumbs/{{ page.image }}" />
<meta property="article:publisher" content="http://www.facebook.com/webjeda" />
<meta property="article:author" content="https://www.facebook.com/sharu725" />
<meta property="article:published_time" content="{{ page.date }}" />

其实还有类似 Twitter 的卡片分享，不过暂时用不到。

## 添加 Icons

所谓的 favicon 是一个 `.ico` 格式的图片，一般是 `16x16` 大小，可以方便用户记住你的网站，包括了在标签页以及收藏栏中。

另外 `Touch Icons` 通常用在移动设备上，当收藏时通常会使用该图片，不过建议不要使用太多，否则打开页面时可能减慢加载速度。

<link rel="apple-touch-icon-precomposed" href="/img/apple-touch-icon-iphone-60x60.png">
<link rel="apple-touch-icon-precomposed" sizes="60x60" href="/img/apple-touch-icon-ipad-76x76.png">
<link rel="apple-touch-icon-precomposed" sizes="114x114" href="/img/apple-touch-icon-iphone-retina-120x120.png">
<link rel="apple-touch-icon-precomposed" sizes="144x144" href="/img/apple-touch-icon-ipad-retina-152x152.png">
-->

{% include ads_content01.html %}

## 其它

<!--
### 增加分享链接
https://blog.webjeda.com/share-buttons-jekyll/
https://github.com/RomkeVdMeulen/jekyll-share-links
-->

### 图片增加 alt 标签

所谓的 `alt` 标签，其实是当图片无法加载时显示的文字内容，所以尽可能在里面添加与文章以及图片相关的关键字。

### Jekyll SEO Tag

实际上 GitHub 支持使用 [Jekyll SEO Tag](https://github.com/jekyll/jekyll-seo-tag) 插件，可以支持很多搜索引擎，使用方式如下。

1. 本地通过 `gem 'jekyll-seo-tag'` 命令安装；
2. 在 `_config.yml` 配置文件中，添加 `jekyll-seo-tag` 插件；
3. 将 {% raw %}`{% seo %}`{% endraw %} 添加到 `</head>` 之前；

注意，如果 Jekyll 的版本小于 `3.5.0` ，那么添加插件时应该使用：

```
gems: [jekyll-seo-tag]
```

而不是

```
plugins:
  - jekyll-seo-tag
```
### 站长工具

通常有 [Google](http://www.google.com/webmasters/tools/)、[Baidu](http://zhanzhang.baidu.com/site/index) 等搜索引擎的站长工具，通常需要在网站的 ```<head> </head>``` 头部添加类似如下的特定元标记，用于标记确实是你网站，也就是用于认证。

{% highlight text %}
<meta name="baidu-site-verification" content="B786jeR0MV" />
{% endhighlight %}

这样就有利于搜索引擎的收录。


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

## 参考

* [10 Must do Jekyll SEO optimizations](https://blog.webjeda.com/optimize-jekyll-seo/) 介绍的内容很不错，这篇文章有很多内容都是参考的这篇文章。

{% highlight text %}
{% endhighlight %}
