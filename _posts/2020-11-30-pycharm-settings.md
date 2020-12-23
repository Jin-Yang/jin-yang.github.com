---
title: PyCharm 常用设置
layout: post
comments: true
language: chinese
tag: [SoftWare, Python]
keywords: PyCharm,常用设置
description: PyCharm 是 JetBrains 打造的一款 Python IDE 跨平台的 IDE ，当然，还有其它语言的 IDE ，快捷键十分相似。这里简单介绍下 PyCharm 相关的一些常用配置。
---

PyCharm 是 JetBrains 打造的一款 Python IDE 跨平台的 IDE ，当然，还有其它语言的 IDE ，快捷键十分相似。

这里简单介绍下 PyCharm 相关的一些常用配置。

<!-- more -->

## 远程操作

比较常见的一个场景是，需要在 Windows 上开发，而代码实际上是跑在 Linux 上的，因为 OS 的差异，会导致一些库在 Windows 上不存在，例如 `grp` 包，所以，可以通过远程的方式对代码进行调试。

### 远程解析器

也就是使用服务器 (例如 Linux) 上安装的 Python 解析器，包括所使用的库。

通过 `File -> Settings` 打开设置窗口，选择 `Project -> Project Interpreter` 右边的小齿轮，新增一个远程解析器。

### 远程上传文件

在本地开发代码的同时，会将本地代码自动同步到远端服务器上，这里会使用到 SSH 提供的 FTP 功能，需要确保 SSH 的 FTP 功能已经开启。

* 开始代码同步功能，打开 `Tools -> Deployment -> Configuration` 并点击左边的 `+` 添加一个部署配置，选择 `SFTP` 模式，输入配置名；如果之前没有创建 SSH 相关的配置，那么就新建一个。
* 增加 SSH 配置，主要就是用户名密码设置，如果需要，也可以使用 Key Pair 模式。
* 在 `Connection` 标签页 `Root path` 是远程服务器中的跟目录，例如 `/home/YourName/WorkSpace` ，需要确保有上述登录用户的权限。
* 在 `Mappings` 标签页，将 `Local Path` 设置为 Windows 下的工程目录，例如 `D:\Projects\MachineLearning`；将 `Deployment path` 设置为远程服务器中的项目目录，例如 `/MachineLearning`，注意要以 `/` 开头。
* 在 `Excluded Paths` 可以设置在同步时忽略哪些目录，包括了本地和服务端的。

另外，在 `Tools -> Deployment -> Options` 中可以设置一些相关的选项，例如 `Create Empty directories` 。

当保存文件后，会自动触发上传，可以查看 `Tools -> Deployment -> Automatic Upload`，也可以通过 `Tools -> Deployment -> Upload to sftp` 上传当前文件。

### 其它

#### FTP 模式区别

在配置远端服务器的时候有几种可选的协议，下面简单介绍下：

* File Transfer Protocol, FTP 文件传输协议，这是一个很简单的通讯协议，可以用来上传、下载文件，以及一些常见的操作，例如创建目录、切换目录等等，不过使用的是明文传输。
* FTPS 是一种加密版的 FTP ，通过 TLS/SSL 加密从客户机到服务器的 FTP 连接。
* Secure File Transfer Protocol, SFTP 安全文件传送协议，与 FTP 有着几乎一样的语法和功能，基于的是 SSH 的协议，而且 SSH 一般都会包含一个 SFTP 的实现。

注意，有些 Linux 的发行版本默认是不会打开 SFTP 功能的，这样，就需要修改配置文件。

另外，SFTP 本身没有单独的守护进程，需要通过 SSHD 守护进程来完成相应的操作。

