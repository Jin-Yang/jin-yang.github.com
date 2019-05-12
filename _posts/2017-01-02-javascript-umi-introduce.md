---
title: UMI 使用简介
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---


<!-- more -->

## 简介

这里介绍如何使用 UMI 搭建前端界面，主要是与框架相关的东西。

{% highlight text %}
# npm install -g umi serve
# npm install -g cross-env
{% endhighlight %}

其中 serve 提供一个简单的服务端，`cross-env` 用于跨平台使用环境变量。

## 使用简介

如下是创建一个项目，并完成发布的流程。

{% highlight text %}
$ mkdir myapp && cd myapp

----- 自动生成部分页面
$ umi g page index
$ umi g page users

----- 启动页面，页面修改后会自动刷新
$ umi dev

----- 测试没有问题，然后准备构建发布
$ umi build

----- 启动一个简单的HTTP Server进行测试，此时应该与umi dev相同
$ serve ./dist
$ python -m SimpleHTTPServer 8080
{% endhighlight %}

然后安装一个简单的 ant design pro 模版。

{% highlight text %}
----- 本地添加依赖
$ yarn add umi-plugin-react

----- 可以选择ant-design-pro模版，实际上就是复制https://github.com/umijs/ant-design-pro
$ yarn create umi

----- 启动本地开发
$ yarn start
{% endhighlight %}

## 路由

个人比较倾向于 `配置式路由`，也就是通过 `config/config.js` 中的 `routes` 配置项进行修改，此时则不会对 `src/pages` 目录做约定式的解析。

## 本地化

默认的配置在 `config/config.js` 中设置，而本地化的相关文件在 `src/locales` 目录下。

{% highlight text %}
$ cd src/locales
$ tree
.
├── en-US
│   ├── globalHeader.js
│   ├── menu.js
│   └── settings.js
├── en-US.js
├── zh-CN
│   ├── globalHeader.js
│   ├── menu.js
│   └── settings.js
└── zh-CN.js

2 directories, 8 files
{% endhighlight %}

以中文为例，也就是 `zh-CN.js` 文件，包含了一些常用翻译，以及关于不同模块的扩展。

{% highlight javascript %}
import menu from './zh-CN/menu';
import settings from './zh-CN/settings';

export default {
  'navBar.lang': '语言',
  'layout.user.link.help': '帮助',
  'layout.user.link.privacy': '隐私',
  'layout.user.link.terms': '条款',
  'app.home.introduce': '介绍',
  ...menu,
  ...settings,
};
{% endhighlight %}

实际上，就是通过 ES6 的语法 `...` 展开运算符，将子模块扩展到本文件中。

<!--
# npm install create-umi -g
----- 创建ant-design-pro，实际上就是复制https://github.com/umijs/ant-design-pro
$ npm create umi

----- 测试
umi test
----- 配置检查
umi inspect
-->

## 参考

<!--
https://www.jianshu.com/p/20f4f39d0461
-->


{% highlight text %}
{% endhighlight %}
