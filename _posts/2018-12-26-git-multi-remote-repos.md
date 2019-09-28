---
title: Git 多个远程库
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: linux,auto,completion
description: 将一个本地仓库同步到不同的远端仓库中。
---

将一个本地仓库同步到不同的远端仓库中。

<!-- more -->

最常见的是新增一个远端仓库地址，并将本地中的一个分支推送到新增的远端服务器。

{% highlight text %}
----- 查看当前所有的远端仓库
$ git remote -v
origin git@git.coding.net:user/project.git (fetch)
origin git@git.coding.net:user/project.git (push)

----- 添加一个名为 github 的远端仓库
$ git remote add github git@github.com:user/repos.git
github git@github.com:user/repos.git (fetch)
github git@github.com:user/repos.git (push)

----- 将本地的 develop 分支推送到 github 端的 master 分支上
$ git push github develop:master
{% endhighlight %}

也可以从 origin 的某个远端分支同步过来，然后再同步到 github 的一个远端分支。

{% highlight text %}
----- 将 origin 中的 develop 分支拉取到本地，作为本地的 master 分支
$ git pull origin develop:master

----- 再将本地的 master 推到 github 的远端 test 分支
$ git push github master:test
{% endhighlight %}

<!--
此时，在仓库本地的配置文件为。

[remote "origin"]
        url = git@gogs.cargo.com:cargo/MonitorAgent.git
        fetch = +refs/heads/*:refs/remotes/origin/*
[branch "master"]
        remote = origin
        merge = refs/heads/master
[remote "github"]
        url = https://github.com/Jin-Yang/MonitorAgent.git
        fetch = +refs/heads/*:refs/remotes/github/*

$git remote set-url --add --push origin git@git.coding.net:user/project.git
# 为其添加 push 到 Coding 的 SSH 地址
$git remote set-url --add --push github git@github.com:user/repo.git
# 为其添加 push 到 GitHub 的 SSH 地址
-->

{% highlight text %}
{% endhighlight %}
