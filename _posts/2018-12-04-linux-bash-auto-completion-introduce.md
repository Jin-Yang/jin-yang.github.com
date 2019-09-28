---
title: Linux Bash 自动补全
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: linux,auto,completion
description: 在 Linux 命令行中，当输入字符后，按两次 `Tab` 键，Shell 就会列出以这些字符打头的所有可用命令，如果只有一个命令匹配到，按一次 `Tab` 键就自动将这个命令补全。 比如，如果想更改密码，但只记得这个命令前几个字母是 pa，此时按 `Tab` 键 Shell 就会自动补全 passwd 命令，非常方便。当然，除了命令补全，还有路径、文件名补全，这个在通过 cd 命令切换到指定目录时特别好用。
---

在 Linux 命令行中，当输入字符后，按两次 `Tab` 键，Shell 就会列出以这些字符打头的所有可用命令，如果只有一个命令匹配到，按一次 `Tab` 键就自动将这个命令补全。

比如，如果想更改密码，但只记得这个命令前几个字母是 pa，此时按 `Tab` 键 Shell 就会自动补全 passwd 命令，非常方便。

当然，除了命令补全，还有路径、文件名补全，这个在通过 cd 命令切换到指定目录时特别好用。

<!-- more -->

## 简介

大部分的终端都提供了自动补全功能，不同的终端略有区别，例如 zsh、bash 等等，这里仅介绍最常用的 bash 提供的补全功能。实际上，补齐功能可以通过脚本指定命令参数如何补全，默认的补全脚本保存在 `/etc/bash_completion.d` 目录下。

对于 bash 来说，使用的是内置的 `complete` 命令，用于支撑 `tab` 键的自动补齐。 

{% highlight text %}
----- 查看命令的类型
$ type -a complete
{% endhighlight %}

CentOS 默认会安装一个 `bash-completion` 包，这里面包含了常用命令的大部分自动补齐脚本，在编写脚本时可以直接参考这个包里的内容。

### git 补齐

在使用命令行时，可以很方便的进行补齐操作，如果不好使，可以使用如下方式配置。

以 git 为例，不同的环境保存的路径可能会有些区别，一般来说，会有多个 `git-completion.XXXX` 文件，其后缀是终端的名称。

可以将如下的命令添加到 `~/.bashrc` 文件中，在每次启动终端时自动加载。

{% highlight text %}
if [ -f /etc/bash_completion.d/git-completion.bash ]; then
	. /etc/bash_completion.d/git-completion.bash
fi
{% endhighlight %}

### 简单示例

假设有一个命令 foobar ，接下来为该命令添加自动补齐功能。

{% highlight text %}
# cat /etc/bash_completion.d/foobar.bash
_foobar()
{
	local cur=${COMP_WORDS[COMP_CWORD]}
	COMPREPLY=( $(compgen -W "exec help test" -- $cur) )
}
complete -F _foobar foobar
{% endhighlight %}

测试 `foobar` 命令是否可以自动补全。注意，`foobar` 命令自身没有自动补全，需要手动输入。

<!-- chmod +x /etc/bash_completion.d/foo.bash -->

{% highlight text %}
# source /etc/bash_completion.d/foobar.bash
# foobar <Tab><Tab>
exec  help  test  
{% endhighlight %}

如上，source 命令是为了加载 `foobar.bash` 使其能在当前会话生效，为了可以自动生效，可以将上述的 source 命令添加到 bashrc 或者 profile 中。

## 常用命令

在上述的示例中使用到了两个命令 `complete` 和 `compgen` ，下面分别介绍这两个命令。

### complete

补全命令，这是最核心的命令了，先看下这个命令的参数说明，可以通过 `help complete` 查看帮助，这里简单列举一下常用参数。

<!--
{% highlight text %}
# help complete
complete: complete [-abcdefgjksuv] [-pr] [-DE] [-o option] [-A action] [-G globpat] [-W wordlist]  [-F function] [-C command] [-X filterpat] [-P prefix] [-S suffix] [name ...]
{% endhighlight %}
-->

{% highlight text %}
-F function	执行shell 函数，函数中生成COMPREPLY作为候选的补全结果
-C command	将 command 命令的执行结果作为候选的补全 结果
-G pattern	将匹配 pattern的文件名作为候选的补全结果
-W wordlist	分割 wordlist 中的单词，作为候选的补全结果
-p [name]	列出当前所有的补全命令
-r [name]	删除某个补全命令
{% endhighlight %}

示例如下。

{% highlight text %}
# complete -W 'word1 word2 word3 test' foobar
# foobar w<Tab>
# foobar word<Tab>
# complete -p
complete -W 'word1 word2 word3 test' foobar
... ...
# complete -r foobar
# complete -p
... ...
{% endhighlight %}

## compgen

筛选命令，用来筛选生成匹配单词的候选补全结果。

<!--
# help compgen
compgen: compgen [-abcdefgjksuv] [-o option]  [-A action] [-G globpat] [-W wordlist]  [-F function] [-C command] [-X filterpat] [-P prefix] [-S suffix] [word]

重点说明：
-W wordlist	分割 wordlist 中的单词，生成候选补全列表

# compgen -W 'word1 word2 test' 
word1
word2
test
# compgen -W 'word1 word2 test' word 
word1
word2



compopt（修改补全命令设置）
这个命令可以修改补全命令设置，注意了，这个命令必须在补全函数中使用，否则会报错。

# help compopt
compopt: compopt [-o|+o option] [-DE] [name ...]

重点说明：
+o option	启用 option 配置
-o option	弃用 option 配置

例如，设置命令补全后不要多加空格，方法如下：
compopt -o nospace


内置补全变量
除了上面三个命令外，Bash还有几个内置变量来辅助补全功能，如下：
COMP_WORDS	类型为数组，存放当前命令行中输入的所有单词
COMP_CWORD	类型为整数，当前输入的单词在COMP_WORDS中的索引
COMPREPLY	类型为数组，候选的补全结果
COMP_WORDBREAKS	类型为字符串，表示单词之间的分隔符
COMP_LINE	类型为字符串，表示当前的命令行输入字符
COMP_POINT	类型为整数，表示光标在当前命令行的哪个位置
https://blog.csdn.net/mycwq/article/details/52420330
-->


{% highlight text %}
{% endhighlight %}
