---
title: Git Cherry Pick 使用
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: git,cherry pick
description: 在开发时如果采用的是分支策略，也就是不同的特性会使用不同的分支进行开发，那么很容易出现切错了分支，导致尝试合并时会失败。 或者是需要将在某个分支开发的代码合并到另外的分支上去，那么此时就需要使用到 Cherry-Pick 的功能了。
---

在开发时如果采用的是分支策略，也就是不同的特性会使用不同的分支进行开发，那么很容易出现切错了分支，导致尝试合并时会失败。

或者是需要将在某个分支开发的代码合并到另外的分支上去，那么此时就需要使用到 Cherry-Pick 的功能了。

<!-- more -->

## 基本使用

假设我们需要将 v3 分支的一个提交合并到 v2 上去，可以通过如下步骤进行操作。

{% highlight text %}
----- 在v3分支上查看提交的CommitID
$ git log

----- 切换到v2分支
$ git checkout v2.0

----- 将位于v3分支上的7a1edfbd提交合并到v2
$ git cherry-pick 7a1edfbd
{% endhighlight %}

当执行完 Cherry Pick 之后，将会生成一个新的提交，这个提交的哈希值和原来的不同，但标识名、提交时间不会修改。

简单来说，Cherry-Pick 就是挑选一个或者多个其它分支的提交，然后合并到当前的分支。

## 命令集合

如下是经常使用的命令。

{% highlight text %}
----- 单独合并一个提交，时间会保留
$ git cherry-pick <CommitID>

----- 功能同上，但是保留原提交者信息
$ git cherry-pick -x <CommitID>
{% endhighlight %}

从 `1.7.2` 版本开始支持批量 Cherry-Pick，也就是一次可以 Cherry-Pick 一个区间的 Commit 。

{% highlight text %}
----- 将(StartCommitID, EndCommitID]之间的提交合并到当前分支
$ git cherry-pick <StartCommitID>..<EndCommitID>

----- 将[StartCommitID, EndCommitID]之间的提交合并到当前分支
$ git cherry-pick <StartCommitID>^..<EndCommitID>
{% endhighlight %}

同其它的 CommitID 使用方式一样，只需要 CommitID 的前六位即可，显然，还需要要求 StartCommitID 要早于 EndCommitID 。

在本地修改完成后，push 到远端即可。

{% highlight text %}
{% endhighlight %}
