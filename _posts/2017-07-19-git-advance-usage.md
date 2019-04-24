---
title: Git 高级应用
layout: post
comments: true
language: chinese
category: [misc]
keywords: git
description:
---

简单介绍一些常见的高级操作。

<!-- more -->

## PreCommit

在提交之前可以执行一些操作检查，比如代码打包、代码检测，称之为钩子；如果执行成功继续提交，否则失败就阻止提交。

{% highlight text %}
----- 直接复制示例代码
$ cp .git/hooks/pre-commit.sample .git/hooks/pre-commit
{% endhighlight %}

<!--
core.whitespace 配置参数控制默认钩子在如下情况时，阻止提交并报错：

行尾空格（ blank-at-eol ）
行首的 Tab 字符前有空格（ space-before-tab ）
文件尾空行（ blank-at-eof ）
添加自定义选项

禁止 Tab 字符行首缩进，将如下选项添加到 core.whitespace
-->



{% highlight text %}
{% endhighlight %}
