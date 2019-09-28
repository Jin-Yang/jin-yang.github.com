---
title: Git 简明教程
layout: post
comments: true
language: chinese
category: [misc]
keywords: flask,示例
description: 记录 Flask 常见的示例，可以用来作为参考使用。
---


<!-- more -->


## 简介

![git code review openstack]({{ site.url }}/images/misc/git-code-review-openstack.png "git code review openstack"){: .pull-center width="80%" }

<!--
对OpenStack提交代码更改的流程主要如下：
    配置Git与Gerrit环境
    克隆目标项目的代码并在新branch上进行更改
    commit到本地的repo后推送给远端Gerrit，然后由reviewer给出意见
    根据reviewer的修改意见不断更新patch
其中OpenStack使用Gerrit作为代码review系统，使用Jenkins系统对代码进行自动测试，使用tox工具可以在本地进行相同的代码测试。

基本前提
    创建一个Launchpad账号（Gerrit使用Launchpad进行SSO）
    登录Gerrit，完成基本配置
    加入OpenStack基金会
    签署Contributor License Agreement
    安装 Git 和 git review
-->

git-review 工具是一组 git 子命令，主要用于 OpenStack 代码与 gerrit (review系统) 交互，可以在后面添加 -v 参数打印所有运行的 git 命令。

### 配置GIT

{% highlight text %}
----- 设置好全局参数，简化以后操作
$ git config --global user.name 'YourName'
$ git config --global user.email example@example.com
$ git config --global gitreview.username YourName

----- 确认下配置项
$ gir config --list

----- 克隆源码
$ git clone https://github.com/openstack/FOOBAR.git

----- 切换到源码目录下
$ cd FOOBAR

----- 建立git-review环境
$ git review -s
{% endhighlight %}

check out 到master分支，更新远端并将其pull到本地的master分支
$ git checkout master; git remote update; git pull origin master



6. 在Launchpad上report 一个新的bug， 或者找一个尚未被解决的bug然后将它assign给自己，将bug的状态改为In progress， OpenStack使用Launchpad记录Blueprints和报告bugs。
7. 想要fix某个bug，就必须新建一个分支，然后在这个分支里对源代码进行修改，例如：
$ git checkout -b fix-bug-#123456
上述命令创建并切换到新分支“fix-bug-#123456”，接下来所有的本地工作在这个分支里进行，直到所有fixation都完成后再commit，
$ git commit -a

提交时会要求输入commit message，commit message可以有下面的字段：
Implements: blueprint BLUEPRINT
Closes-Bug: #123456
Partial-Bug: #123456
Related-Bug: #123456
通过这些字段来标识自己工作相关的bug或者blueprint，一旦注明，CI系统会自动将你的commit和相同ID的bug对应起来。

上面的命令提交到本地repo后接下来就是push到Gerrit了。
$ git review -v

Gerrit是OpenStack远端Git仓库的一道大门，所有的submission都要在这里经过review后才能被merge到master分支中，因此之前的工作一定不能在master分支进行，这样会产生一个merge commit，Gerrit默认是不接受merge commit的。

如果提交成功，Gerrit将返回一个显示你此次提交内容的URL，打开它就可以查看commit以及reviewer的评价了：http://review.openstack.org/nnnnnn

如果需要修改commit怎么办？

此时需要到http://review.openstack.org上查找自己的patch记录，然后记下这一个patch的review number，就是review.openstack.org对应patch页面的后几位数字：https://review.openstack.org/#/c/nnnnnn/

$ cd ourTargetProjectName    #切换到项目源码目录
$ git review -d nnnnnn        #把patch给check out，然后就可以编辑了


接着根据reviewer们的意见重新编辑patch，然后提交
$ git commit -a --amend    #在本地commit
$ git review

对上一次的commit进行了修改，或者commit message没有写标准，都可以重新提交commit，但是一定要切换到自己上次提交commit的分支执行上面的命令。如果希望查看完整的git命令流，可以在git review命令后添加 -v选项。









{% highlight text %}
----- 如果用户名和邮箱开始配置有问题，可以通过如下方式修改
$ git commit --amend --author='foobar  <foobar@example.com>'
{% endhighlight %}

其它的一些常见的情况包括了：代码审核未通过，返回修改；代码有冲突，不能合入代码库。这些情况，其解决方法都类似，都可以通过 amend 解决。
如果代码审核未通过，现在本地git log查看一下。最近的一条log是不是就是你要修改的那一个，是的话，OK，不
是的话，git reset --soft  commit_id到你需要修改的那一个commit记录。
继续修改你要改的文件
git add
git commit --amend
repo upload
三步，ok！注意如果你提交了3个文件，其中一个不过关，只需要修改、add 那一个文件就行。如果少提交了一个文件，也是add这个文件就ok了。
如果你多提交了一个文件，处理方法：
mv  filename  newfilename         #先把文件重命名，此时git status查看，可以看到多余commit的文件处于工作区delete状态。
git commit  -a  --amend
然后git log --name-status -1 查看多余提交的文件已被撤销，此时可将之前重命名的文件再改回来重新upload后会生成一个patch set 2。









注意：当审核未通过打回时，我们再修改完成之后，执行：
git add 文件名
git commit --amend ##注意会保留上次的 change-id ，不会生成新的评审任务编号，重用原有的任务编号，将该提交转换为老评审任务的新补丁集
git review











http://blog.csdn.net/agileclipse/article/details/38980419



## 参考

[OpenStack git-review](http://docs.openstack.org/infra/git-review/index.html)


<!--
OpenStack的Commit Message风格，
    这里对于每一次提交commit时commit message的风格有一定的介绍：https://wiki.openstack.org/wiki/GitCommitMessages

OpenStack的bug分流，
    这里有OpenStack对不同程度的bug进行分流的介绍：https://wiki.openstack.org/wiki/BugTriage
-->

{% highlight text %}
{% endhighlight %}
