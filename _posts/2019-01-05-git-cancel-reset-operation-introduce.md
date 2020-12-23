---
title: Git 撤销操作
layout: post
comments: true
language: chinese
category: [misc,linux]
keywords: git,reset
description:
---

在代码开发过程中进程会遇到各种各样的场景需要进行回退，例如：

1. 刚把不想要的代码 commit 到了本地仓库中了，不过还没有 push 到远端！
2. 刚 push 的代码有 bug ，现在想要干掉它！
3. 完了，刚线上更新的代码出了 bug ，需要回退这次提交的代码！

OK，这里看看针对这些场景应该如何进行处理。

<!-- more -->

## 撤销

也即是上述的场景一，在未执行 `push` 前的所有操作，都是在 `本地仓库` 中执行的，这里将所有 `本地仓库` 相关的代码还原操作叫做 `撤销` 。

### 修改未 add

文件已经被修改，但未执行 `git add` 操作，此时保存在了 Working Tree 中，还未添加到版本管理中，如果撤销那么就不能恢复。

{% highlight text %}
$ git checkout YourFile    # 恢复单个文件
$ git checkout .           # 恢复整个目录
{% endhighlight %}

如果只想从缓存中删除，可以使用 `git rm --cached "/Your/File/Path"` 或者 `git rm --cached "/Your/Path" -r` ，此时不会删除物理文件。

注意，如果不加 `--cached` 参数，实际上会直接删除物理文件。

### add 本地未 push

此时已经添加到了 Stage 中，但是还没有提交到本地，可以通过 `reset` 从 Stage 恢复到 Working Tree ，如果需要撤销可以参考上一场景。

{% highlight text %}
$ git reset HEAD YourFile  # 撤销暂存
$ git checkout YourFile    # 撤销本次修改
{% endhighlight %}

### commit 本地

已经提交到了本地，想修复最近一次提交，不需要修改提交的内容。

{% highlight text %}
$ git commit --amend -m "Your Commit"
{% endhighlight %}

此时，提交的 CommitID、提交信息会被修改，但是提交的日期、内容不会被修改。

如果想撤销到之前的几个版本，可以通过如下命令处理。

{% highlight text %}
$ git reset [--hard|soft|mixed|merge|keep] [CommitID|HEAD]
  --hard 完全回退，上次的提交都会被删除，如果要恢复会比较麻烦
{% endhighlight %}

## 回滚

如果已经 `push` 到远端的仓库，将该场景称为回滚。注意，对远程仓库做回滚操作是有风险的，需提前做好备份和通知其他团队成员！

### 回滚指定文件

撤销指定文件到指定版本。

{% highlight text %}
----- 查看指定文件的历史版本
$ git log YourFile
----- 回滚到指定CommitID
$ git checkout CommitID YourFile
{% endhighlight %}

### 删除最后一次远程提交

可以使用 revert 或者 reset 命令。

{% highlight text %}
----- 使用revert，此时会保留之前的历史
$ git revert HEAD
$ git push origin master
----- 使用reset，不会保留上次的提交历史
$ git reset --hard HEAD^
$ git push origin master -f
{% endhighlight %}

两者的区别是：

* revert 是放弃指定提交的修改，但是会生成一次新的提交，需要填写提交注释，以前的历史记录都在；是一个新提交。
* reset 是指将 HEAD 指针指到指定提交，历史记录中不会出现放弃的提交记录；需要强制提交。

上述的 revert 也可以通过指定 CommitID 回退到某个版本，建议使用 revert 而非 reset ，否则所有使用该分支的人都需要在本地执行 reset 。

注意，如果当前版本到回退版本之间有对相同文件的修改，就需要解决冲突。

## 删除某次提交

这个比较复杂，没有太好的验证过，注意风险。

{% highlight text %}
$ git log --oneline
129deb2 Add 3
c2243ba Add 2
8bef0e7 Add 1
2ba2fe1 Your Commit
053849c init
{% endhighlight %}

假设有如上的提交历史，然后想删除其中的 `Add 1` 的那次提交，需要执行如下命令。

{% highlight text %}
$ git rebase -i "c2243ba"^
{% endhighlight %}

等价于。

{% highlight text %}
$ git rebase -i "c2243ba"^1
{% endhighlight %}

也就是回退到 `Your Commit` 处。

此时会有一个窗口选择提交的内容，默认是

## Rebase

在使用 git 时，一般都会使用合并 (Merge) 来整合改动，不过这不是唯一的方法，还可以使用 Rebase 操作。实际上有些团队会使用 Rebase 而非 Merge ，但是相比来说 Rebase 要复杂了很多。

### 合并操作

当 git 执行一个合并时，它实际上会查找三个提交：

(1)共同的原始提交
如果你在项目中查看两个分支的历史，它们总是会出自于一次共同的提交，那么在当时的时间点上，这两个分支还是拥有相同的内容。之后它们就开始有了差别。
(2) + (3) 两个分支的最终点
合并操作的目的就是把两个分支的最新状态结合起来。因此他们各自的最新版本是有特殊含义的。
结合这三个提交后得到的结果就是我们整合的目标。

## 其它

### reset \-\-hard 恢复

如果是误操作，那么可以通过 reflog 来解决这一问题。

它会记录所有 HEAD 操作历史，也就是说当你做 reset、checkout 操作的时候，这些操作会被记录在 reflog 中。

{% highlight text %}
$ git reflog
053849c HEAD@{0}: reset: moving to HEAD^1
2ba2fe1 HEAD@{1}: commit (amend): Your Commit
317e36f HEAD@{2}: commit: second
{% endhighlight %}

如果要找回第二 Commit，只需要做如下操作。

所以，如果因为 reset 等操作丢失一个提交的时候，总是可以把它找回来。除非你的操作已经被 git 自动清理了，一般是 30 天以后。

{% highlight text %}
{% endhighlight %}
