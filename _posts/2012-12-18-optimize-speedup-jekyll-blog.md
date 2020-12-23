---
title: 优化加快 Jekyll Blog 的访问速度
layout: post
comments: true
language: chinese
tag: [Blog,Others]
keywords: jekyll,github,cdn
description: 如果直接使用 GitHub Pages 提供的 CDN ，在国内访问会很慢，而且因为 Baidu 被 GitHub 屏蔽，所以搜索效果不是很好。这里会介绍如何优化、测试国内的访问性能，同时提供一些国内常用的解决方案。
---

如果直接使用 GitHub Pages 提供的 CDN ，在国内访问会很慢，而且因为 Baidu 被 GitHub 屏蔽，所以搜索效果不是很好。

这里会介绍如何优化、测试国内的访问性能，同时提供一些国内常用的解决方案。

<!-- more -->

## 简介

优化 Jekyll 博客的加载速度有很多方法，

* 优化博客代码，尽量减少需要加载的资源，同时对必须加载的资源进行压缩；
* 将博客部署到国内服务器上，或者使用 CDN 加速；
* 将代码同时部署到国内的代码托管平台，例如 Gitee、Coding 等；

部署国内的太麻烦，所以还是考虑 CDN 加速的方式，包括了一些常用的库，例如 Bootstrap、JQuery 等，自己编写的需要自己去托管到一个 CDN 服务器上。

{% include ads_content01.html %}

## CDN

虽然可以删除不需要的资源，但是国内访问 GitHub Pages 速度实在是太慢，建议使用 CDN 搭建进行加速。可以使用国内的云厂商提供的 CDN 方案，而且大部分每月会有免费流量，但是域名需要进行备案，所以想想，还是免了。

另外 jsDelivr 提供 CDN 服务原生支持 GitHub 中使用资源，不过只对于 50M 以下的仓库是免费的，忽略。

还有一个原因，因为 GitHub 屏蔽了 Baidu 的爬虫，而 Baidu 在国内搜索的占比有比较高，但是无法被其收录，从而影响网站的访问量。

<!--
https://www.marketmechina.com/baidu-search-engine-market-share-in-china-dec-2019/
-->

### Baidu 收录问题

CDN 无法解决文章的收录问题，尤其是对小流量的个人博客来说。

如果 Baidu 爬虫可以直接访问 CDN 节点上的资源，是没有问题的，但是，当第一次请求缓存未建立，或者缓存过期了之后请求会回源到 GitHub ，由于转发请求默认携带了百度爬虫的 UserAgent 以及原始 IP 信息，在 GitHub 发现后会直接拒绝。

所以，如果要使用这种方式，可以单独搭建一个 Nginx 反向代理，将转发的爬虫信息取消掉，但是这样门槛比较高，而且需要单独再部署一个 VPS 。

### 托管/镜像

搜索发现很多人在使用 Netlify ，提供了静态资源网络托管，能够将托管 GitHub、GitLab、Bitbucket 等网站上通过 Jekyll、Hexo、Hugo 等代码自动编译并生成静态网站，不过国内的速度堪忧。

另外一个还不错的是 [Vercel](https://vercel.com/) ，与上述的 Netlify 类似，部署起来也很简单。

{% include ads_content02.html %}

## 其它

有个工具 [jekyll-compress-html](https://github.com/penibelst/jekyll-compress-html) 可以用来压缩 HTML ，不过经常会出现标签被错误移除，而空行却没有被正常处理的情况，所以不建议使用。

## 参考

* 可以通过 [tool.chinaz.com](https://tool.chinaz.com/speedtest/) 进行测速，Google 也提供了一个测试工具 [PageSpeed Insights](https://developers.gogle.com/speed/pagespeed/insights/) 。

<!--
https://zpjiang.me/2020/01/15/let-baidu-index-github-page
-->

{% highlight text %}
{% endhighlight %}
