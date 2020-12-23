---
title: Linux 常用命令 AWK
layout: post
comments: true
language: chinese
category: [linux]
keywords: linux,awk
description:
---

AWK 是一种程序语言，对文档资料的处理具有很强的功能，擅长从格式化报文或一个大的文本文件中抽取数据，会将一行文字按分隔符分为多个域，然后进行处理。

这里简单介绍其常用的方法。

<!-- more -->


## awk

{% highlight text %}
----- 通常使用的是单引号，如果是双引号，那么示例如下
$ ls -l $dir | awk "/$base\$/ { print \$1 \$3 \$4 }"
$ p=`ls -l $dir | awk "/$base\$/ { print \\\$1 \\\$3 \\\$4 }"`

----- 分割passwd文件
$ awk -F : '$1=="root" {print $0}' /etc/passwd
{% endhighlight %}

### 常用语法

#### if else

{% highlight text %}
awk '{if ($1==1) print "A"; else if ($1==2) print "B"; else print "C"}'
{% endhighlight %}

注意，每个语句后面需要使用 `;` 进行分割。

## 与 Shell 交互

在写 Shell 脚本时，经常会使用到 awk 程序，此时可能会需要两者之间相互调用。

### Awk 使用 Shell 变量

主要有如下的几种方法。

#### 方法 <1>

{% highlight text %}
var="abc"
awk 'BEGIN{print "'$var'"}'
{% endhighlight %}

这种写法其实际是双括号变为单括号的常量，然后传递给了 awk 。

#### 方法 <2>

{% highlight text %}
var="this a test"
awk 'BEGIN{print "'"$var"'"}'
{% endhighlight %}

如果变量的值中包含空格，为了使 shell 不把空格作为分隔符，则应使用这一方法。

#### 方法 <3>

{% highlight text %}
var="this a test"; export var;
awk 'BEGIN{print ENVIRON["var"]}'
{% endhighlight %}

也就是使用环境变量进行传递。

#### 方法 <4>

{% highlight text %}
var="this a test"
awk -v awkVar="$var" 'BEGIN{print awkVar}'
{% endhighlight %}

也就是使用 `-v` 选项，如果变量不多时，建议使用这一方式。

### Shell 使用 Awk 变量

其方法无非是用 awk (sed perl 也一样) 输出若干条 shell 命令，然后再用 shell 去执行这些命令。

{% highlight text %}
eval $(awk 'BEGIN{print "var1='str1';var2='str2'"}')
eval $(awk '{printf("var1=%s; var2=%s; var3=%s;",$1,$2,$3)}' abc.txt)
echo "var1=$var1 ----- var2=$var2"
{% endhighlight %}

### 示例

<!--
#### 磁盘使用率

#!/bin/sh

FILE_SYSTEM_NAME="rootfs"
MOUNTED_ON="/"

eval $(df -P | awk '$1=="'"$FILE_SYSTEM_NAME"'" && $6=="'$MOUNTED_ON'" {printf("spaceSize=%s;",$5)}')

spaceSize=`echo $spaceSize | cut -d% -f1`
if [ aa$spaceSize = "aa" ]; then
	spaceSize=-1
fi

if [ $spaceSize -le 85 ]; then
    echo '主磁盘的使用空间充足'
elif [ $spaceSize -eq -1 ]; then
    echo '没有找到主磁盘使用空间，请检查脚本'
else
    echo '主磁盘的使用空间超过阈值'
fi
-->

#### 计算进程 CPU 使用率

{% highlight bash %}
#!/bin/bash

BOOTIME=`awk '{ print $1 }' /proc/uptime`
HERTZ=`getconf CLK_TCK`

if [[ ! "${BOOTIME}" =~ ^[0-9.]*$ ]]; then
        return 1
fi

if [[ ! "${HERTZ}" =~ ^[0-9]*$ ]]; then
        return 1
fi

CPUSAGE=`awk -v bootime="${BOOTIME}" -v hz="${HERTZ}" 'BEGIN{percent=0}; {  \
        tmp = bootime * hz - $22;                                           \
        if (tmp <=0) percent = 0; else percent = ($14 + $15) * 100 / tmp    \
        } END{print percent}' /proc/14945/stat`

echo $CPUSAGE
{% endhighlight %}


## 常用示例

### 按列求和

可以使用 awk 命令计算文件中某一列的总和。

{% highlight text %}
----- 对第二列求和
$ awk 'BEGIN{sum=0}{sum+=$2}END{print sum}' data.txt

----- 对满足开始为/foobar/的行求和
$ awk 'BEGIN{sum=0};/foobar/{sum+=$2};END{print sum}' data.txt

----- 另外比较完整的一个例子
$ awk -F ',' 'BEGIN{sum=0;count=0}{if ($(NF-11)==2 && $NF==0 && $3=="11" && $6~/TIME|ESTABLISHED/) \
     {sum +=$5; count++;} } END {print "sum="sum" count="count " avg="sum/count}'
{% endhighlight %}

`$N` 表示第 N 列，从 0 开始计算；`$0` 表示所有列；`NF` 表示 Number of Fields，也就是字段数，`$NF` 表示最后一个字段，`$(NF-1)` 表示倒数第二个字段。



<!--
https://www.cnblogs.com/xudong-bupt/p/3721210.html
-->

{% highlight text %}
{% endhighlight %}
