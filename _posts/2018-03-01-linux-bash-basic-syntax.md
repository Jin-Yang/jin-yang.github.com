---
title: Bash 基本语法
layout: post
comments: true
language: chinese
category: [misc]
keywords: bash
description:
---


<!-- more -->

## 判断

Shell 中使用条件判断时，有几种方式：A) `if ... fi`；B) `if ... else ... fi`；C) `if ... elif ... else ... fi` 。

在 Shell 编程中，如果要判断条件是否满足，可以使用 `[ ]` `test` `[[ ]]` `let` `(( ))` 这几种方式，其中 `[ ]` `test` 是 Shell 的内部命令，其功能基本相同，而 `[[ ]]` 是 Shell 的关键字。

`[ ]` 的三个基本功能是判断文件、字符串、整数，需要注意：

* 比较运算符只支持 `==` `!=`，两者用于字符串比较，不可用于整数比较，整数只能使用类似 `-eq` `-gt` 这种形式。注意，如果要使用 `<` `>` 需要加转义，否则会被作为重定向。
* `let` `(( ))` 功能基本相同，用于算术运算以及整数比较，可以直接使用 `<` `>` 等比较运算符。而且可以直接使用变量名不需要 `$var` 这种形式。

### [] VS. [[]]

可以看下两者的区别。

#### 1. 关键字 VS. 内部命令

{% highlight text %}
$ [ 2 < 1 ] && echo True || echo False
True
$ [[ 2 < 1 ]] && echo True || echo False
False
{% endhighlight %}

因为是命令，那么它就会和参数组合为一体被 Shell 解析，例如上述的 `>` `<` 就被 Shell 解释为重定向符号了，导致第一个认为是 `True` ；而关键字会认为这就是一个条件判断，所以其返回的结果符合预期。

#### 2. 逻辑判断

{% highlight text %}
$ x=1, y=1
$ [ $x == 1 && $y == 1 ] && echo True || echo False
-bash: [: missing `]'
False
$[ $x == 1 -a $y == 1 ] && echo True || echo False
True
$ [[ $x == 1 && $y == 1 ]] && echo True || echo False
True
{% endhighlight %}

如果使用与、或等判断条件时，需要注意：A) 在 `[[` 中可以使用 `&&` 和 `||`；B) 在 `[` 中需要使用 `-a` 和 `-o` 。

#### 3. 字符匹配

{% highlight text %}
$ [[ 'abcd' == a*d ]] && echo True || echo False
True
$ [ 'abcd' == a*d ] && echo True || echo False
False
{% endhighlight %}

`[[` 支持字符串的模式匹配，右侧会被作为一个模式，而不仅仅是一个字符串，而且不需要使用引号。另外，果使用 `=~` 操作符时还可以支持 Shell 的正则表达式。

<!--
#### 4. 表达式

$[ !(rpm -qa | grep gcc) ] && echo True || echo False
-bash: [: too many arguments
False

$[[ !(rpm -qa | grep gcc) ]] && echo True || echo False
True
https://www.cnblogs.com/zeweiwu/p/5485711.html
-->

### 总结

建议使用 `[[ ]]` ，相比来说，使用 `[[ ]]` 会更加简单，而且符合判断逻辑，可以防止出现逻辑错误，另外还支持字符串、命令等较高级的模式。

使用方法总结如下。

* `[[ ]]` 左右都要有空格分隔；
* 内部操作符与操作变量之间要有空格 `[[ "a" == "b" ]]` ；
* 字符串比较可以直接使用 `>` `<` 无需转义，也可以使用 `&&` `||` 逻辑判断；
* 字符串或者 `${}` 变量默认会进行模式和元字符匹配，也可以使用 `""` 进行严格匹配；

## 数组

数组有如下的几种处理方式。

{% highlight bash %}
#----- 使用[]操作符
foobar[0]="Hello"
foobar[1]="World"
foobar[2]="!!!"

#----- 使用()直接赋值，如下两种方式相同
foobar=("Hello" "World" "!!!")
foobar=([0]="Hello" [1]="World" [2]="!!!")

#----- declare -a 定义一个空的数组，如果已经定义则不会清零
declare -a foobar

#----- 也可以从文件中读取，每行作为其一个元素，注意可能会失败报错
foobar=(`cat 'foobar.txt'`)

# 读取数组
foobar=("Hello" "World" "!!!")
#----- 可以直接通过下标读取
echo ${foobar[0]}

#----- 获取数组长度
echo ${#foobar[@]}

#----- 未定义的下标不会占用个数
foobar=([0]="Hello" [1]="World" [5]="!!!")
echo ${#foobar[@]}

#----- 获取数组的一部分，会使用上述的下标
echo ${foobar[@]:4:5}

#----- 合并两个数组
hello=("Hi" "Foobar")
all=(${foobar[@]} ${hello[@]})
echo ${#all[@]}
{% endhighlight %}

使用 `@` 这个特殊的下标，可以将数组扩展成列表，然后就可以使用 bash 中的获取变量长度的操作符 `#` 来获取数组中元素的个数。

注意，没有定义的数组下标，并不会占用数组中元素的个数，如上所述。

模式操作符对数组也是有效的，可以使用它来替换数组中的元素

{% highlight bash %}
#----- 修改数组，这里是临时替换，也可以重新保存一个
echo ${foobar[@]/World/Foobar}
nfoobar=(${foobar[@]/World/Foobar})
{% endhighlight %}

数组遍厉如下。

{% highlight text %}
for foo in ${foobar[@]}; do
       echo ${foo}
done

len=${#adobe[@]}
for ((i = 0; i < $len; i++));do
	echo ${adobe[$i]}
done
{% endhighlight %}

<!--
http://blog.zengrong.net/post/1518.html
-->

## for 循环

使用 `for` 循环时，基本有如下的几种方法。

{% highlight bash %}
#----- 普通枚举类型，列表通过空格或者回车分隔
for fruit in 'apple' 'meat' 'sleep' 'woman'; do
	echo "I like ${fruit}"
done

#----- 也可以使用`` ()调用子命令
ans=0
for i in `seq 0 2 100`; do
    let ans+=${i}
done
echo $ans

#----- 使用花括号标示枚举，例如{1..100} {Z..A}
ans=0
for i in {1..100}; do
    let ans+=${i}
done
echo $ans
{% endhighlight %}

如果使用 `{}` 也可以指定增量，例如 `{0..100..2}` 对应的步长是 2 ，而 `seq` 命令为 `seq 首数 [增量] 末数` 。

也可以使用如下的方式，在一行内运行 `for 变量 in 取值列表; do 各种操作; done` 。

另外，还支持 C 语言风格的 for 循环，示例如下，也就是计算一下 `1~100` 的和。

{% highlight bash %}
ans=0
for ((i = 1; i <= 100; i++)); do
        let ans+=$i
done
echo $ans
{% endhighlight %}

注意，数值比较只支持整形，如果要使用浮点数，则需要通过 `bc` 进行转换。

{% highlight bash %}
a=7.2; b=8.3
if [[ `echo "$a < $b" | bc` -eq 1 ]] ; then
        echo  "$a < $b "
fi
{% endhighlight %}

### 示例

#### 参数遍厉

Bash 中在遍历参数时，尽量采用 `$@` 而非使用 `$*` 参数，其中前者可以正确处理一些带有空格的参数，例如如下的示例。

{% highlight bash %}
foobar() {
        for var in "$@"; do
                echo "$var"
        done
}
foobar 1 2 "3 4"
{% endhighlight %}

## 函数

其中 `$#` 表示所有入参的数量，位置参数 `$1` `$2` ... `$N` 代表了各个参数，而 `$0` 代表了第一个参数，也就是脚本的名字。

函数的返回值需要小于 `255` ，可以通过 `$?` 获取。

{% highlight bash %}
function foobar() {
        if [[ $1 == "foo" ]]; then
                return 1
        fi
        return 0
}

foobar foo
echo $?
foobar
echo $?
{% endhighlight %}

也可以通过 `echo` 命令返回一个字符串。

{% highlight bash %}
function foobar() {
        if [[ $1 == "foo" ]]; then
                echo "bar"
                return
        fi
        echo "foo"
}

ret=$(foobar foo)
echo $ret
{% endhighlight %}

这里采集的是标准输出，需要特别注意标准错误输出的处理方式。

## 括号使用

在编写 Shell 脚本时，可以使用多种括号，其中使用括号主要有如下的几种方式：

* `${var}` 引用变量；
* `$(cmd)` ```` 调用命令并获取标准输出，注意不会收集标准错误输出；
* `()` `{}` 用于执行子命令，使用方式略有区别；
* `$((expr))` 执行数学运算，其语法类似于 C 语言；
* `${var: }` `${var% }` `${var# }` 用于字符串的替换、匹配。

### () VS. {}

二者都用于执行一串命令，指令之间用 `;` 分开，其区别为：

* `()` 需要重新开启一个 Shell 运行命令，`{}` 会在当前 Shell 运行；
* `()` 最后一条命令不需要 `;` ，而 `{}` 最后一条命令需要 `;` ；
* `{}` 第一条指令和左括号之间需要有空格，而 `()` 不需要。

{% highlight text %}
$ var=test
$ (var=notest; echo $var)   # 只影响到子Shell
$ echo $var

$ { var=notest; echo $var; }
$ echo $var                 # 同时也会被修改
{% endhighlight %}

### $((expr))

用来执行数学计算，简单介绍其使用方法。

{% highlight text %}
$ echo $((3 + 2))
5
$ echo $((3 > 2))
1
$ echo $((25 < 3 ? 2 : 3))
3
$ echo $var

$ echo $((var = 2 + 3))
5
$ echo $var
5
$ echo $((var++))
5
$ echo $var
6
$ echo $((++var))
5
{% endhighlight %}

<!--
### 字符串操作

${var:-string},${var:+string},${var:=string},${var:?string}
$(var%pattern),$(var%%pattern),$(var#pattern),$(var##pattern)



${var: } ,${var%}用于变量的替换和匹配替换结构
替换：

${var:-string} 若var为空，${var:-string}值为string

${var:=string} 与${var:-string}功能类型，不过${var:=string} 将string值赋给var变量

${var:=string} 判断某个变量是否赋值，没有的话则给它赋上一个默认值

${var:+string}的替换规则和${var:=string}相反，即只有当var不是空的时候才替换成string，若var为空时则不替换或者说是替换成变量 var的值，即空值

匹配：

${var%pattern}和${var%%pattern}表示从最右边(即结尾)匹配的

${var#pattern} 和${var##pattern}从最左边(即开头)匹配的

中括号[] [[]]
比较不错的介绍
https://www.cnblogs.com/include/archive/2011/12/09/2307905.html

-->



{% highlight text %}
{% endhighlight %}
