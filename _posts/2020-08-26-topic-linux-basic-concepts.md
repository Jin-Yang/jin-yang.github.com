---
title: 【专题】Linux 基本入门操作
layout: post
language: chinese
tag: [Topic, Linux]
keywords: Linux
description: 整理总结与 Linux 相关的基础知识，覆盖了从准备环境、文件系统、用户管理、进程管理等各个方面的基础知识，会分别进行详细介绍。
---

整理总结与 Linux 相关的基础知识，覆盖了从准备环境、文件系统、用户管理、进程管理等各个方面的基础知识，会分别进行详细介绍。

<!-- more -->

* [前言](/post/linux-basic-concepts-preface.html) 当时为什么准备整理 Linux 的相关资料。

## 准备环境

## 文件系统

## 使用终端

## 用户管理

* [用户管理](/post/linux-user-account-management.html) 介绍 Linux 中与用户相关的基本概念，以及如何对用户进行管理。
* [PAM 机制详解](/post/linux-pam-security-introduce.html) 将鉴权、授权、密码管理等功能抽象成一个框架，方便用户使用。

### 相关链接

* [使用 C API 执行 Linux 用户相关操作](/post/linux-user-manage-c-api.html) 通过 glibc 提供的 API 可以获取用户信息、切换用户等操作。
* [Capabilites 机制详细介绍](/post/linux-capabilities-introduce.html) 通过该机制，可以允许普通用户也可以具有部分 root 的权限，从而可以保证权限的最小化。
* [用户管理的最佳实践](/post/linux-user-best-practise.html) 通过 DevOps 中的一个常见场景，介绍如何使用用户、主用户组、辅助用户组控制权限。

## 进程介绍

{% highlight text %}
{% endhighlight %}
