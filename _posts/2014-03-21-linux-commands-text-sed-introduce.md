---
title: Linux 常用命令 SED
layout: post
comments: true
language: chinese
category: [linux]
keywords: linux,sed
description: sed 是一个精简的、非交互式的编辑器，可以提供与编辑器 VIM 或者 EMACS 相同的编辑任务，但 sed 不提供交互使用方式，只能在命令行下输入编辑命令。这里简单介绍其使用方法。
---

sed 是一个精简的、非交互式的编辑器，可以提供与编辑器 VIM 或者 EMACS 相同的编辑任务，但 sed 不提供交互使用方式，只能在命令行下输入编辑命令。

这里简单介绍其使用方法。

<!-- more -->

## sed

常用操作如下。

{% highlight text %}
----- 文件头部添加一行，命令1表示第一行，i表示插入，之后是内容
sed -i "1ifoobar" filename
  i 行前插入 insert
  a 行后插入 append
  c 行替换   change
----- 规则匹配行前插入
sed -i "/foobar/ifoo" filename

----- 删除指定行，$表示最后一行
$ sed -i '1d' filename
$ sed -i '$d' filename
$ sed -i '/foobar/d' filename
{% endhighlight %}

<!--
sed '0,/^520/{//d;b};0,/131$/d'
sed '/^520/{:a;N;/131$/!ba;d}'
-->

可以通过如下命令删除两个标记行之间的内容，相关帮助可以通过 `info sed` 查看。

{% highlight text %}
----- 查看输出，没有问题直接替换
$ sed '0,/^#YOUR-MARK-BEGIN$/{//d;b};0,/^#YOUR-MARK-END$/d'    # 只删除第一个匹配
$ sed '/^#YOUR-MARK-BEGIN$/, /^#YOUR-MARK-END$/d'              # 删除所有匹配
$ sed -i -e '0,/^#YOUR-MARK-BEGIN$/{//d;b};0,/^#YOUR-MARK-END$/d'
{% endhighlight %}

### 多行替换

sed 经常用来替换文件的内容，通常是处理单行的，但通过它的一些内建功能，也能实现多行替换。假设有如下的文本：

{% highlight text %}
  hello <<<comment part 1
  comment part 2>>>
  foobar
{% endhighlight %}

现在需要把 ```<<< ... >>>``` 这一段替换为 "COMMENT"，那么 sed 语法应当是：

{% highlight text %}
:begin
/<<</,/>>>/ {
    />>>/! {
        $! {
            N;
             b begin
        }
    }
    s/<<<.*>>>/COMMENT/;
}
{% endhighlight %}

上述语句存储在 test.sed 中，那么执行的方式和结果就是：

{% highlight text %}
$ sed -f foobar.sed foobar
  hello COMMENT
  foobar
{% endhighlight %}

把正则直接写到命令里面也可以，用 ```";"``` 来分隔命令即可，注意右花括号之后也要加上分号 ```";"```，如果再加上 -i 参数就可以直接把改动写到原文件中去了：

{% highlight text %}
$ sed -e ":begin; /<<</,/>>>/ { />>>/! { $! { N; b begin }; }; s/<<<.*>>>/COMMENT/; };" test
  hello COMMENT
  foobar
{% endhighlight %}

各个步骤介绍如下：

1. 花括号 ```{}``` 代表命令块的开始，类似 C 语法；
1. ```:begin``` 是一个标号 (label)，用于跳转，供 b、t、T 等命令使用，这里使用了 b 命令；
1. ```/<<</,/>>>/``` 通过逗号分隔 (开始+结束位置) 用于标示地址范围，后面 {} 中的命令只对地址范围之间的内容使用。
1. ```/>>>/!``` ```$!``` 其中叹号表示取反，而 $ 在 sed 中表示为最后一行，也就意味着 "如果在本行没有发现结束标记，而且本行不是文件的最后一行" 那么执行下面的操作；
1. ```N;``` 用于把下一行的内容追加到缓冲区 (pattern)，也就是相当于合并为一行；
1. ```b begin``` 由于仍然没有找到结束标记跳回到 begin，重新执行追加命令；
1. ```s/<<<.*>>>/COMMENT/;``` 匹配完成之后，可以通过该命令替换。

<!--
http://man.linuxde.net/sed

Update @ 2007-12-14

在和bxy讨论的过程中，又发现sed的另外一种用途，从html或xml中按照tag对应关系，筛选打印出指定的tag内容，使用了正则中的p命令，好像默认就没有“不能处理多行内容”以及“贪婪性”的问题，很好用，很强大：

    $ sed -n -e '/<title>/p' -e '/<text /,/<\/text>/p' from.xml

注意//不在同一行的时候才好用，不然会匹配到下一个实例出现的位置作为结束边界。

sed -e ":begin; /\/\*/,/>>>/ { />>>/! { $! { N; b begin }; }; s/<<<.*>>>/COMMENT/; };"

----- 文件头以/** **/标示，可以通过如下方式打印或者删除
sed -n -e '/\/\*\*/,/^ \*\*\//p' plugin.c
sed -i -e ':begin; /\/\*\*/,/^ \*\*\//d' plugin.c

----- 只显示匹配行
sed -n '/This/p' plugin.c


## 多行合并

将某个目录下的文件合并成一行，中间用冒号 ```:``` 作为分隔符：

ls /tmp | paste --serial --delimiters=":"
ls /tmp | tr "\n" ":"

http://www.361way.com/awk-sed-convert-oneline/5127.html

另外，在 sed 中包含了 ```@``` 符号，类似于 ```/``` 用于分割正则表达式，尤其是在正则表达式中有 ```/``` 时比较实用。

默认只替换第一个就结束，通过 ```g``` 表示会检查所有的行。

----- 行首/行尾添加字符串，
sed 's/^/HEAD&/g' test.file
sed 's/$/&TAIL/g' test.file
sed '/./{s/^/HEAD&/;s/$/&TAIL/}' test.file
-->

{% highlight text %}
{% endhighlight %}
