---
title: 词法语法解析
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: bnf
description: 介绍与词法语法分析相关的概念。
---

介绍与词法语法分析相关的概念。

<!-- more -->

## 简介

可以通过 `yum install flex flex-devel bison` 安装 Linux 中的 flex 以及 bison 包。

![workflow]({{ site.url }}/images/programs/flex-bison-workflow.png "workflow"){: .pull-center }

## BNF

也就是 Backus-Naur Form 巴科斯范式，由 John Backus 和 Peter Naur 首先引入的，用来描述计算机语言语法的符号集，现在，几乎每一位新编程语言书籍的作者都使用巴科斯范式来定义编程语言的语法规则。

其推到规则通过 ```::=``` 定义，左侧为非终结符，右侧为一个表达式；表达式由一个符号序列，或用 ```'|'``` 分隔的多个符号序列构成，从未在左端出现的符号叫做终结符。

{% highlight text %}
"..."  : 术语符号，表示字符本身，用double_quote用来代表双引号
< >    : 必选项
[ ]    : 可选项，最多出现一次
{ }    : 可重复0至无数次的项
|      : 左右两边任选一项，相当于OR
::=    : 被定义为
{% endhighlight %}

如下是 Java 中的 For 语句实例：

{% highlight text %}
FOR_STATEMENT ::=
"for" "(" ( variable_declaration |
( expression ";" ) | ";" )
[ expression ] ";"
[ expression ] ";"
")" statement
{% endhighlight %}

其中 RFC2234 定义了扩展的巴科斯范式 (ABNF)。

## 上下文无关文法

简单来说就是每个产生式的左边只有一个非终结符；首先，试着用汉语来稍微解释一下。

{% highlight text %}
本来这个进球就是违例的，但你不肯承认也没办法
我有一本来自美国的花花公子杂志
拿我的笔记本来
{% endhighlight %}

如果汉语是上下文无关文法，那么我们任何时候看见 ```"本来"``` 两个字，都可以把它规约为一个词；可惜汉语不是上下文无关文法，所以能否归约为一个词，要看它的上下文是什么。如上的示例中，只有第一句可以规约为一个词。

<!--
上下文无关文法就是说这个文法中所有的产生式左边只有一个非终结符，比如：
S -> aSb
S -> ab
这个文法有两个产生式，每个产生式左边只有一个非终结符S，这就是上下文无关文法，因为你只要找到符合产生式右边的串，就可以把它归约为对应的非终结符。

比如：
aSb -> aaSbb
S -> ab
这就是上下文相关文法，因为它的第一个产生式左边有不止一个符号，所以你在匹配这个产生式中的S的时候必需确保这个S有正确的“上下文”，也就是左边的a和右边的b，所以叫上下文相关文法。


其中，生成式由终结符和非终结符组成，分别用大写、小写表示；如果终结字符是一个单字符，那么可以直接使用该字符，如;、)、[。对于语义来说，如整型 INTEGER 的值可以是 1、34、5555。当匹配一个语义时可以直接执行一个动作。

解析器相对于状态机多了一个栈，从而可以处理移进和规约。简单的说，LR(1) 就是指，只需要预读一个 token 那么就可以知道如何解析字符串中的任意一部分。虽然 bison 可以适用于几乎所有的上下文无关语法，不过其针对 LR(1) 做了专门的优化。

在一些确定性的 LR(1) 语法中，仍然会存在这歧义，可能不知道对那个语法规则执行规约，或者不知道是执行规约还是移进，分别被称为 规约/规约冲突 或者 移进/规约冲突。

关于上下文相关、无关语法可以参考 WiKi <a href="https://en.wikipedia.org/wiki/Context-free_grammar">Context-free grammar</a>、<a href="https://en.wikipedia.org/wiki/Context-sensitive_grammar">Context-sensitive grammar</a>，以及 <a href="http://cs.union.edu/~striegnk/courses/nlp-with-prolog/html/node37.html">Context Free Grammars</a>，也可以参考本地文档。
-->

## 参考

关于最原始的论文，可以参考 [Lex - A Lexical Analyzer Generator](http://www.cs.utexas.edu/users/novak/lexpaper.htm) ([本地](/reference/databases/mysql/LEX_A_Lexical_Analyzer_Generator.html))，以及 [Yacc: Yet Another Compiler-Compiler](http://www.cs.utexas.edu/users/novak/yaccpaper.htm) ([本地](/reference/databases/mysql/YACC_Yet_Another_Compiler_Compiler.html))。

对于 Lex 和 Yacc 来说，比较经典的入门可以参考 [Lex & Yacc Tutorial](http://epaperpress.com/lexandyacc/index.html)，其中包括了如何编写一个计算器，以及相关的调试等信息；也可以参考 [本地文档](/reference/databases/mysql/LexAndYaccTutorial.pdf)，以及相关的 [源码](/reference/databases/mysql/LexAndYaccCode.zip) 。

关于总体介绍可以参考 [Lex and YACC primer](http://www.tldp.org/HOWTO/Lex-YACC-HOWTO.html)，或者 [本地文档](/reference/databases/mysql/LEX_YACC_Primer_HOWTO.html)，也可以查看中文翻译 [如何使用 Lex/YACC](http://segmentfault.com/blog/icattlecoder/1190000000396608)，以及 [本地](/reference/databases/mysql/LEX_YACC_Primer_HOWTO_cn.mht)，以及 [示例源码](/reference/databases/mysql/lex-yacc-examples.tar.gz) ；以及 [Bison-Flex 笔记](/reference/databases/mysql/Bison_Flex_Notes.mht)、[Flex/Bison Tutorial](/reference/databases/mysql/Tutorial-Flex_Bison.pdf) 。

关于调试方法可以参考 [Understanding Your Parser](http://www.gnu.org/software/bison/manual/html_node/Understanding.html)，这个是 [Bison Offical Documents](http://www.gnu.org/software/bison/manual/html_node/index.html) 文档的一部分；更多可以参考 [dinosaur.compilertools.net](http://dinosaur.compilertools.net/) 查看相关的资料。


<!--
自己动手写编译器
http://pandolia.net/tinyc/
https://bellard.org/tcc/
计算器
http://good-ed.blogspot.tw/2010/04/lexyacc.html
编写自己的编译器
http://coolshell.cn/articles/1547.html

http://blog.csdn.net/huyansoft/article/details/8860224
http://blog.csdn.net/lidan3959/article/details/8237914
http://www.tuicool.com/articles/3aMVzi
http://blog.csdn.net/sfifei/article/details/9449629



http://www.calvinneo.com/2016/07/29/flex%E5%92%8Cbison%E4%BD%BF%E7%94%A8/
-->

{% highlight text %}
{% endhighlight %}
