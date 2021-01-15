---
title: SASS 使用介绍
layout: post
comments: true
language: chinese
tag: [FrontEnd, CSS]
keywords: 前端,SASS,CSS
description:
---


<!-- more -->

## 搭建环境

SASS 基于 Ruby 语言开发，所以在安装 SASS 前需要安装 Ruby ，一般 Mac 和 Linux 会默认安装，Windows 可以搜下教程。

### 配置 Ruby 环境

需要确认 Ruby 的版本号是否正确。

```
----- 确认Ruby已经安装，及其版本号
$ ruby -v
```

Ruby 会使用 Gems 进行库的管理，建议使用 `2.6.X` 以上版本，可以通过 `gem -v` 查看版本号，如果不满足，可以通过 `gem update --system` 升级，需要管理员账户。

注意，在使用境外的仓库时，可能会下载比较慢，甚至失败，可以通过如下方式更新仓库地址。

```
----- 替换原有的Gems库地址
$ gem sources --add https://gems.ruby-china.com/ --remove https://rubygems.org/

----- 确认只有一个库地址
$ gem sources -l
https://gems.ruby-china.com
```

注意，这里的设置与用户相关，不同的用户需要分别设置。其中，与 gem 相关的命令常见如下。

```
----- 安装、更新、卸载、查看本地
$ gem install PACKAGE
$ gem update PACKAGE
$ gem uninstall PACKAGE
$ gem list --local

----- 也可以只下载但不安装
$ gem fetch PACKAGE

----- 查看所有或者搜索包
$ gem list --remote
$ gem search STRING --remote
```

在安装时，二进制文件会添加到 `~/bin` 目录下，而包则保存在 `~/.gem` 目录下，也就意味着，需要将 `~/bin` 目录添加到 `PATH` 环境变量中。

### SASS 安装

可以直接通过 RubyGems 这个工具来安装所需的 `Sass` 和 `Compass` 两个包。

```
----- 安装
$ gem install sass compass

----- 确认版本号
$ sass -v
Ruby Sass 3.7.4

----- 查看帮助信息
$ sass -h
```

### 编译 SASS

编译有多种方式，很多编辑器都提供了插件，这里介绍命令行的使用方式，可以设置编译后 CSS 排版、生成调试 map、开启 debug 信息等。

```
----- 单文件转换命令
$ sass input.scss output.css

----- 单文件监听命令
$ sass --watch input.scss:output.css

----- 编译格式
$ sass --watch input.scss:output.css --style compact

----- 编译时添加调试map
$ sass --watch input.scss:output.css --sourcemap

----- 开启debug信息
sass --watch input.scss:output.css --debug-info
```

通过 `--style` 解析时，其排版格式有 `nested` `expanded` `compact` `compressed` 四种。

```
// 未编译样式
.box {
  width: 300px;
  .box-title {
    height: 30px;
  }
}

// nested
.box {
  width: 300px; }
  .box .box-title {
    height: 30px; }


// expanded
.box {
  width: 300px;
  height: 400px;
}
.box-title {
  height: 30px;
  line-height: 30px;
}

// compact
.box { width: 300px; height: 400px; }
.box-title { height: 30px; line-height: 30px; }

// compressed
.box{width:300px;height:400px}.box-title{height:30px;line-height:30px}
```

<!--
----- 监听整个目录
sass -watch app/sass:public/stylesheets
-->

## 语法简介

### 变量

其中 SASS 一个重要的特性是引入了变量，可以重复使用变量，老版本使用 `!` 引用变量，新版本采用 `$` 来标识变量。

```
// 声明变量
$highlight-color: #F90;
$basic-border: 1px solid black;
```
如上，可以指定单个或者多个属性值，而且允许在 CSS 规则之外定义，当变量定义在 CSS 规则块内时，那么该变量只能在此规则块内使用。

```
$highlight-color: #F90;
$highlight-border: 1px solid $highlight-color;
.selected {
  border: $highlight-border;
}

// 编译后
.selected {
  border: 1px solid #F90;
}
```

