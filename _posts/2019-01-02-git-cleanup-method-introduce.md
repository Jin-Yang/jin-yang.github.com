---
title: Git 清理
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: git
description: 在项目开发时，可能误传了一些敏感信息，例如账号、密码，如果后面要将项目开源，为了避免信息泄露，那么就必须清除 git 的提交历史记录。 还有一种场景是误传了很多二进制文件。
---

在项目开发时，可能误传了一些敏感信息，例如账号、密码，如果后面要将项目开源，为了避免信息泄露，那么就必须清除 git 的提交历史记录。

还有一种场景是误传了很多二进制文件。

<!-- more -->

## 清理版本文件

大多数版本控制系统 (例如 SVN) 存储的是一组初始文件，以及每个文件随着时间的演进而逐步积累起来的差异，而 git 则会把文件的每一个差异化版本都记录下来。

这也就意味着，即使你只改动了某个文件的一行内容，git 也会生成一个全新的对象来存储新的文件内容，如果频繁改动一个大文件，那么就可能导致库增加。

### 示例

生成一个大文件，大约有 54M 。

{% highlight text %}
$ for i in {1..1000}; do head -c 40960 /dev/urandom | base64 >> largefile; done
$ ls largefile -alh
-rw-r--r-- 1 foobar users 53M Apr 25 12:57 largefile
$ git add largefile
$ find .git/objects -type f
.git/objects/e8/e1a5d81918f4ef96e0c87021d9a75d5f6496a4   # 41M
$ ls .git/objects/e8/e1a5d81918f4ef96e0c87021d9a75d5f6496a4 -alh
-r--r--r-- 1 foobar users 41M Apr 25 13:02 .git/objects/e8/e1a5d81918f4ef96e0c87021d9a75d5f6496a4
$ echo "add last" >> largefile
$ git add largefile
$ find .git/objects -type f
.git/objects/30/293c04c935023fc2ab2902d569fdd7d6f83b07   # 41M
.git/objects/e8/e1a5d81918f4ef96e0c87021d9a75d5f6496a4   # 41M
{% endhighlight %}

git 最初向磁盘中存储对象时所使用的格式被称为 "松散(loose)" 对象格式，为了节省空间以及提高效率，会将这些对象打包成 "包文件(packfile)" 和索引文件。

可以手动执行 `git gc --prune=now` 完成，git 在提交的时候也会执行压缩。

{% highlight text %}
$ git commit -m "fisrt commit"  # 如果是第一次需要提交之后才可以
$ git gc --prune=now
Counting objects: 3, done.
Delta compression using up to 8 threads.
Compressing objects: 100% (2/2), done.
Writing objects: 100% (3/3), done.
Total 3 (delta 0), reused 0 (delta 0)
$ find .git/objects -type f | xargs ls -lh
-r--r--r-- 1 foobar users  41M Apr 25 13:05 .git/objects/30/293c04c935023fc2ab2902d569fdd7d6f83b07
-rw-r--r-- 1 foobar users   54 Apr 25 14:23 .git/objects/info/packs
-r--r--r-- 1 foobar users 1.2K Apr 25 14:23 .git/objects/pack/pack-95d4f509aa9862c232396ec27ba892b33fa89fd8.idx
-r--r--r-- 1 foobar users  41M Apr 25 14:23 .git/objects/pack/pack-95d4f509aa9862c232396ec27ba892b33fa89fd8.pack
{% endhighlight %}

除了之前的 blob 文件之外，会新建包文件和索引文件，其中包文件包含了刚才从文件系统中移除的所有对象的内容，索引文件包含了包文件的偏移信息，可以通过索引文件就可以快速定位任意一个指定对象。

可以通过 `git verify-pack` 查看已经打包的内容。

{% highlight text %}
$ git verify-pack -v .git/objects/pack/pack-95d4f509aa9862c232396ec27ba892b33fa89fd8.idx
67291d3a67880f038bb829df0bbd4f14c7a399af commit 166 116 12
93514499bd6dff6e67a303ff5698269eb507304b tree   37 48 128
7ef75db14469e8935583bf9f672188bff569a8ca blob   55335018 42095055 176
non delta: 3 objects
.git/objects/pack/pack-95d4f509aa9862c232396ec27ba892b33fa89fd8.pack: ok
{% endhighlight %}

## 完全清理

一般是在一个项目打算做开源时，将历史的提交数据完全清理。

{% highlight text %}
----- 新建一个全新的分支，不会包含任何的历史记录
$ git checkout --orphan latest
----- 上述操作会将所有的文件作为最新的提交，也可以再添加、删除其它文件
$ git add -A
----- 将上述的文件提交
$ git commit -am "open source"
----- 然后删除老的master分支
$ git branch --delete master
----- 将当前分支重命名为master
$ git branch -m master
----- 将本地的分支强制提交到远端
$ git push --force origin master
{% endhighlight %}


<!--
清理大文件的方法
https://johng.cn/git-clear-large-files-in-history/
https://blog.csdn.net/yiifaa/article/details/78603410

[git-scm.com Pro Git](https://git-scm.com/book/zh/v2) 关于 git 使用方法以及内部详情的不错介绍。

https://www.jianshu.com/p/7231b509c279
-->


{% highlight text %}
{% endhighlight %}
