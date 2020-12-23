---
title: Flex 简介
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:
---

一种词法分析器，可以通过正则表达式识别文本中的词汇，然后自动生成对应的 C 代码，最后编译链接 C 代码就可以了。

Flex 采用的是状态机，通过分析输入流 (字符流)，只要发现一段字符能够匹配一个关键字 (正则表达式)，就会采取对应的动作。

<!-- more -->

## 简介

Flex 文件被 `%%` 分成了上中下三个部分：

1. 第一部分中要写入 C/C++ 代码必须用 `%{` 和 `%}` 括起来，将原封不动放到生成源码中。
2. 第二部分是规则段，包括了模式 (正则表达式) 和动作，由空白分开，当匹配到模式时，就会执行后面的动作。
3. 第三部分可以直接写入 C/C++ 代码。

`yylex()` 是扫描程序的入口，调用该函数启动或重新开始，该函数会初始化一些全局变量，然后开始扫描。如果定义的 flex 动作是将数值传递给调用程序，那么对 `yylex()` 的下次调用就从它停止的地方继续扫描。

{% highlight text %}
%{
#include <stdio.h>
%}
%%
is |
are                     { printf("%s: VERB ", yytext); }
island                  printf("LAND ");
[0-9]+                  printf("NUMBER ");
[ \t]+                  /* ignore whitespace */;
[a-zA-Z][a-zA-Z0-9]*    printf("COMMON WORD ");
.|\n                    { ECHO; }
%%
int main(void) {
    yylex();
}
{% endhighlight %}

特殊字符 `'.'` 表示匹配换行符以外的任意单个字符，`'\n'` 匹配换行符；`ECHO` 表示输出匹配的模式，默认行为为，也就是将输入原样输出：

{% highlight c %}
#define ECHO fwrite(yytext, yyleng, 1, yyout)
{% endhighlight %}

默认将 stdin 作为输入，可以通过如下命令测试。

{% highlight text %}
----- 首先按照规则生成C源码lex.yy.c
$ flex example.l

----- 然后进行编译
$ cc lex.yy.c -o example -lfl

----- 直接执行测试，Ctrl-D退出
$ ./example
is
is: VERB
are
are: VERB
island
LAND
89
NUMBER
foobar
COMMON WORD
^*&
^*&
{% endhighlight %}

在处理时，flex 采用两个原则：A) 只匹配一次；B) 执行当前输入的最长可能匹配值。也就是对与 island 不会匹配 is 和 land 。当然，我们可以使用一个文件作为关键字列表，而非每次都需要编译。

解析时会通过 yyin 读取，如果需要在 yacc 或者其它文件中设置，那么可以通过如下方式修改。

{% highlight c %}
extern FILE *yyin;
yyin = fopen("filename","r");
{% endhighlight %}

通过 flex 处理文件后，会将匹配转化为指定的符号，然后供 yacc 处理。

例如 C 语言的 Lex 配置信息  [ANSI C grammar, Lex specification](https://www.lysator.liu.se/c/ANSI-C-grammar-l.html) ，可以作为参考。

### 示例

{% highlight text %}
%option nounput
%option noinput
%option noyywrap
%{
        int yylineno;
%}
%%

^(.*)\n printf("%4d\t%s", yylineno++, yytext);
%%

int main(void)
{
        const char *file = "test.txt";

        yyin = fopen(file, "r");
        if (yyin == NULL) {
                fprintf(stderr, "open file '%s' failed, %d:%s.\n",
                        file, errno, strerror(errno));
                return -1;
        }

        yylex();
        fclose(yyin);

        return 0;
}
{% endhighlight %}

如上是一个最简单的示例，会读取 `test.txt` 文件中的每行数据，并打印，可以通过如下方式编译。

{% highlight text %}
$ flex --outfile=test.yy.c test.l
$ gcc test.yy.c -Wall -o test
{% endhighlight %}

## 常用功能

简单列举常用函数。

### 正则表达式

flex 常用正则表达式：

1. 格式与 grep 相似；
2. `<<EOF>>` 标示文件结束；
3. 常用字符集，如 `[:alpha:]`, `[:digit:]`, `[:alnum:]`, `[:space:]` 等；
4. `{name}` 使用预定义的 name 。

简单示例，计算平均值。

{% highlight text %}
%{
	#include <stdio.h>
	#include <stdlib.h>
%}
dgt    [0-9] // 通过name方式定义
%%
{dgt}+   return atoi(yytext);
%%
void main(void)
{
	int val, total = 0, n = 0;
	while ( (val = yylex()) > 0 ) { // 到文件结束时返回0
		total += val;
		n++;
	}
	if (n > 0)
		printf(“ave = %d\n”, total/n);
}
{% endhighlight %}

如上如果在编译时使用 `-Wall` 参数，会报 `warning: `yyunput’ defined but not used` 之类的异常，如下介绍可以通过如下选项关闭。

{% highlight text %}
%option nounput
%option noinput
{% endhighlight %}

如下是常见的示例。

{% highlight text %}
[ \t\n] ;                      // 忽略空格 Tab 换行字符
[a-Z]+  printf("%s", yytext);  // 打印字符串输出
[a-Z]+  ECHO;                  // 功能与上相同
{% endhighlight %}

### 多规则匹配

如果有多个值匹配，那么 flex 会按照如下的规则选取。

1. 贪婪匹配，选择最大的匹配值；
2. 多个规则匹配，选择第一个；
3. 没有规则匹配则会选择默认规则。

例如，有如下的规则。

{% highlight text %}
integer ...; // 关键字
[a-z]++ ...; // 标识符
{% endhighlight %}

当输入了 integers 后，会被识别为标识符，因为 `[a-z]++` 匹配的字符串更长，分别为 8 个和 7 个。当输入了 integer 之后，两个规则都匹配了 7 个字符，此时会选择第一个，也就是 `integer` 。

<!--
最长匹配原则可能会导致部分规则异常，例如 `'.*'` 本意是匹配单引号字符串，但是可能会导致匹配异常。

当输入为 `'first' quoted string here, 'second' here` 时，会匹配到 `'second'` 位置，最好修改为 `'[^'\n]*'` 。
-->

另外，如果通过 `"/*"(.|\n)*"*/"` 规则匹配 C 语言中的注释，那么如下场景可能出错。

{% highlight c %}
#include <stdio.h>  /* definitions */
int main(int argc, char * argv[ ])
{
	if (argc <= 1) {
		fprintf(stderr, "Error!\n");  /* no arguments */
		return -1;
	}

	printf("%d args given\n", argc);
	return 0;
}
{% endhighlight %}

一般会有个默认的规则，例如 `. {}` ，否则，如果有不匹配的字符，那么默认会直接输出到终端。

贪婪匹配，会从 `/* def` 到 `nts */` 之间的内容都作为注释，此时就需要使用条件 (Condition) 规则。例如，以 `<S>` 开始的规则，只有在条件 S 时才会进行匹配，可以在 definition section 段通过如下方式定义条件。

{% highlight text %}
%x S exclusive start conditions
%s S inclusive start conditions
{% endhighlight %}

然后，通过 `BEGIN(S)` 进入条件，另外，flex 有个初始条件，可以通过 `BEGIN(INITIAL)` 返回；如果使用多个状态，那么实际上可以实现一个状态机，详见 [lex tutorial.ppt](https://www2.cs.arizona.edu/~debray/Teaching/CSc453/DOCS/lex tutorial.ppt) 或者 [本地文档](/reference/databases/mysql/lex_tutorial.ppt) 。

关于上述内容，也可以参考 [Start conditions](http://dinosaur.compilertools.net/flex/flex_11.html) 中的介绍。

### yyterminate()

可在一个动作中代替 return 使用，用于结束扫描并向扫描器的调用者返回 0；可以通过如下方式自定义。

{% highlight c %}
#ifdef yyterminate
# undef yyterminate
#endif
#define yyterminate() \
    do { free (foobar); foobar = NULL; pos = 0; len = 0; \
        return YY_NULL; } while (0)
{% endhighlight %}

### 配置选项

{% highlight text %}
%option yylineno    提供当前的行信息，通常用于后续打印错误行信息
%option noyywrap    不生成yywrap()声明
%option noinput     会生成#define YY_NO_INPUT 1定义
%option nounput     会生成#define YY_NO_UNPUT 1定义
{% endhighlight %}

flex 会声明一个 `int yywarp(void);` 函数，但是不会自动定义，所以通常会在最后的 section 实现该函数。该函数的作用是将多个输入文件打包成一个输入，也就是当 `yylex()` 读取到一个文件结束 (EOF) 时，会调用 `yywrap()` ，如果返回 1 则表示后面没有其它输入文件了，此时 `yylex()` 函数结束；当然，`yywrap()` 也可以打开下一个输入文件，再向 `yylex()` 函数返回 0 ，告诉它后面还有别的输入文件。

如果只有一个文件，那么可以通过 `%option noyywrap` 不声明该函数，也就不需要再实现。

## 其它

### 值传递

在通过 flex 进行扫描时，会将值保存在 yylval 变量中，而 bison 则读取 yylval 中的值，该变量默认是 int 类型，如果要使用字符串类型，那么可以在 .l+.y 的头部第一句加入 `#define YYSTYPE char*` 即可。

{% highlight text %}
// 在.l赋值的时候，要特别注意，需要拷贝字符串
yylval = strdup(yytext);  return WORD;
// 在.y取用的时候，直接强转就可以了
(char*)$1
{% endhighlight %}

关于更优雅的实现方式，当然是用 union 啦，仿照上面，很容易写出来的。

### 标准格式

{% highlight text %}
%{
/* C语言定义，包括了头文件、宏定义、全局变量定义、函数声明等 */
}%
%option noinput       /* 常见的配置选项 */
WHITE_SPACE [\ \t\b]  /* 正则表达式的定义，如下section时可以直接使用这里定义的宏 */
COMMENT #.*
%%
{WHITE_SPACE}           |
{COMMENT}               {/* ignore */}  /* 规则定义处理 */
%%
/* C语言，函数实现等 */
{% endhighlight %}

<!--
### 使用语法

在匹配到字符串之后，会保存到 `yytext` 变量中，同时将长度保存在 `yyleng` 变量中，那么最后一个字符可通过 `yytext[yyleng - 1]` 访问。

yyless() yymore()

当读取完成后会调用 `yywrap()` 函数，默认会返回 1 标示结束，也可以重新打开一个 `yyin` 并返回 0 ，此时会继续解析。
-->


{% highlight text %}
{% endhighlight %}
