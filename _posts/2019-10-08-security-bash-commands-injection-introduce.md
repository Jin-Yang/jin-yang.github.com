---
title: 命令注入
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: command injection,命令注入,bash,shell
description:
---

所谓的命令注入，简单来说，就是利用任何会调用系统命令的应用，通过构造特殊命令字符串的方式，执行恶意代码。常见的例如 SQL 命令注入、页面命令注入等。

<!-- more -->

## 简介

以 Linux 中的 Bash 为例，其提供的不只是一个命令执行的通道，有些独立的语法规则，然后将各种命令执行结果进行拼接。

如下是一些常见的技巧。

### 命令拼接

在 Shell 执行完一条命令之后，会将返回值保存在 `$?` 变量中，当 `$?==0` 时表示执行成功，否则失败。

如下是一个常见的命令拼接方式：

* `A;B` 会执行两个命令，无制约关系；
* `A&&B` 只有当 A 执行成功后，才会执行 B ；
* `A||B` 只有当 A 执行失败后，才会执行 B ；
* `A|B` 通过管道将 A 的输出作为 B 的输入；
* `""` 将双引号中的内容视作字符串，允许替换变量；
* `''` 单引号中的同样作为字符串，但不允许变量替换；

### 命令替换

通过两个反引号以及 `$()` 都可以用作命令替换，也就是将一个命令的输出作为另外一个的输入，例如 `echo "Now is $(date)"` 。

其中反引号移植性比较高，可以绝大部分 Shell 中使用，只是不太容易阅读；而 `$()` 并不是所有 Shell 都支持。

### 命令合并

通过 `(cmd1;cmd2...)` 或者 `{ cmd1;cmd2...}` 可以执行一组命令，两个命令之间都需要通过 `;` 分割，但是略有区别：

* `()` 命令会在子 Shell 中执行，而 `{}` 会在当前 Shell 中执行；
* `()` 最后一个命令可以不用分号，而 `{}` 的最后一个命令必须要用分号；
* `()` 左括号和第一个命令之间不必有空格，而 `{}` 需要有个空格；

另外，如果在括号里有重定向，那么只会应该改命令；如果在括号之外，那么就会影响到所有命令。

### 自动展开

可以通过 `[]` 和 `{}` 用来展开，不过如果文件不存在，那么 `[]` 将会失去模式匹配功能，而变成一个单纯的字符串；但是 `{}` 始终会展开。

{% highlight text %}
$ ls foo[a-e]ar.txt
foobar.txt
$ ls foo[e-g]ar.txt
ls: cannot access foo[e-g]ar.txt: No such file or directory

$ ls foo{a,b}ar.txt
ls: cannot access fooaar.txt: No such file or directory
foobar.txt
{% endhighlight %}

实际上，`[]` 使用的是正则表达式，而 `{}` 是 Shell 的文件扩展。

## 规则绕过

也就是如何绕过一些常见的防范规则。

### 空格绕过

有些会检查是否存在空格，当存在时会执行失败，那么可以使用 `<` `>` `${IFS}` 绕过，其中 `IFS` 默认值为空白，包括了空格、Tab、新行等。

例如 `cat<>./foobar.txt` `cat<./foobar.txt` `cat${IFS}./foobar.txt` 。

### 关键字绕过

防止因为对关键字匹配，导致某些命令无法执行，有如下的几种方式。

### 空白参数

也就是使用 `$*` `$@` `$num` 方式，在没有传参的时候，这些特殊变量都是空的。

{% highlight text %}
$ ca$*t foobar.txt
Hello World
$ ca$@t foobar.txt
Hello World
$ ca$9t foobar.txt
Hello World
$ ca${10}t foobar.txt
Hello World
{% endhighlight %}

### 转义字符

如果使用元字符的时候，需要通过 `\` 进行转义，而对于通用字符，使用转义是无效的。

{% highlight text %}
$ ca\t foo\bar.txt
Hello World
{% endhighlight %}

### 变量替换

可以使用定义的变量，包括了变量中的部分。

{% highlight text %}
$ a=ca;b=t;c=foobar.txt
$ $a$b $c
Hello World

$ a="acat";b=${a:1:3};$b foobar.txt
Hello World
{% endhighlight %}

### 编码方式

可以使用 Base64、十六进制、八进制等方式。

{% highlight text %}
$ echo "cat foobar.txt" | base64
Y2F0IGZvb2Jhci50eHQK
$ echo "Y2F0IGZvb2Jhci50eHQK" | base64 -d | bash
Hello World

$ echo "cat foobar.txt" | xxd
00000000: 6361 7420 666f 6f62 6172 2e74 7874 0a    cat foobar.txt.
$ echo "0x63617420666f6f6261722e747874" | xxd -r -p | bash
Hello World
$ $(printf "\x63\x61\x74\x20\x66\x6f\x6f\x62\x61\x72\x2e\x74\x78\x74")
Hello World
{% endhighlight %}

#### 其它

{% highlight text %}
----- 可以使用单引号或者双引号
$ ca"t" foobar.txt
Hello World
$ ca't' foobar.txt
Hello World

----- 使用花括号扩展
$ {cat,foobar.txt}
Hello World
{% endhighlight %}

<!--
https://blog.zeddyu.info/2019/01/17/%E5%91%BD%E4%BB%A4%E6%89%A7%E8%A1%8C/
-->

### 预防措施

* 选择不调用系统命令的实现方法，同时可以提高性能，但可能降低灵活性。
* 不将外界输入直接作为参数传递给命令行。
* 使用安全的函数对参数进行转义，例如转义所有元字符 `#&;,\|*?~<>^()[]{}$` ，还包括了反引号。


{% highlight text %}
{% endhighlight %}
