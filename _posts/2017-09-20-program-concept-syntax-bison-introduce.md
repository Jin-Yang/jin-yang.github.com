---
title: Bison 简介
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:

---


<!-- more -->

## 简介

## 词法语法解析

不错的图片介绍
https://www.jianshu.com/p/728e4011a61e

### 文件格式

两个文件的格式基本类似，分成三部分，都是通过 `%%` 进行分割，如下所示。

... definitions ...
%%
... rules ...
%%
... subroutines ...

其中：

1.  `definitions` 定义模式、C语言变量、以及包含c头文件等。 rules 部分用户定义模式对应的动作。 subroutines 部分用于定义c函数等。

## Lex

一种词法分析器，可以通过正则表达式识别文本中的词汇，然后自动生成对应的 C 代码，最后编译链接 C 代码就可以了。

例如 C 语言的 Lex 配置信息  [ANSI C grammar, Lex specification](https://www.lysator.liu.se/c/ANSI-C-grammar-l.html) ，可以作为参考。

### 示例

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

如上是一个最简单的示例，会读取 `test.txt` 文件中的每行数据，并打印，可以通过如下方式编译。

$ flex --outfile=test.yy.c test.l
$ gcc test.yy.c -Wall -o test

### 使用语法

当匹配到字符串之后，会直接执行后面的动作，也可以为空。

[ \t\n] ;                      // 忽略空格 Tab 换行字符
[a-Z]+  printf("%s", yytext);  // 打印字符串输出
[a-Z]+  ECHO;                  // 功能与上相同

#### 匹配字符串

在匹配到字符串之后，会保存到 `yytext` 变量中，同时将长度保存在 `yyleng` 变量中，那么最后一个字符可通过 `yytext[yyleng - 1]` 访问。

<!-- yyless() yymore() -->

#### 读取结束

当读取完成后会调用 `yywrap()` 函数，默认会返回 1 标示结束，也可以重新打开一个 `yyin` 并返回 0 ，此时会继续解析。

#### 歧义源规则

如果有多个表达式可以匹配当前的输入，那么 lex 会按照如下的方式来选择。

1. 首选最长匹配。
2. 在匹配相同数目字符的规则中，首选最先给出的规则。

例如，有如下的规则。

integer ...; // 关键字
[a-z]++ ...; // 标识符

当输入了 integers 后，会被识别为标识符，因为 `[a-z]++` 匹配的字符串更长，分别为 8 个和 7 个。当输入了 integer 之后，两个规则都匹配了 7 个字符，此时会选择第一个，也就是 `integer` 。

##### Tips #1

最长匹配原则可能会导致部分规则异常，例如 `'.*'` 本意是匹配单引号字符串，但是可能会导致匹配异常。

当输入为 `'first' quoted string here, 'second' here` 时，会匹配到 `'second'` 位置，最好修改为 `'[^'\n]*'` 。

## 其它

一般会有个默认的规则，例如 `. {}` ，否则，如果有不匹配的字符，那么会直接输出到终端。


## Yacc

Yacc 的 GNU 版叫做 Bison ，可以针对上述 Flex 生成的标记做一些动作，也就是描述如何处理这些标记(Token)，这就是所谓的语法分析，。

其中 Yacc 的文法采用 Backus-Naur Form, BNF 的变量规则描述，能够表达上下文无关的语言。

### 示例

/* hello.l */
%{
        #include "hello.tab.h"
        int yyerror(char *errormsg);
        int yyparse(void);
%}

%%

("hi"|"Hi")"\n"       { return HI;  }
("Bye"|"bye")"\n"     { return BYE; }
[-[]+.,><]            { return yytext[0]; }
.                     { yyerror("Unknow char");  }

%%
int main(void)
{
        yyparse();
        return 0;
}

int yywrap(void)
{
        return 0;
}

int yyerror(char *errormsg)
{
        fprintf(stderr, "%s\n", errormsg);
        exit(1);
}


/* hello.y */
%{
        #include <stdio.h>
        #include <stdlib.h>
        int yylex(void);
        int yyerror(const char *s);
%}


%token HI BYE

%%
program:
        hi bye
        ;

hi:
        HI     { printf("Hello World\n");   }
        ;
bye:
        BYE    { printf("Bye Now\n"); exit(0); }
        ;

# Makefile
all:
        flex --outfile=hello.yy.c hello.l
        bison -d --output=hello.tab.c hello.y
        gcc hello.tab.c hello.yy.c -o hello

clean:
        rm *.tab.c *.tab.h *.yy.c -f hello

可以直接执行 hello ，然后输入 `Hi` `Bye` 加上回车即可。

### 规则

定了语法的产生以及语义的动作，一般规则为 `Result: Components {...};`，其中 `Result` 为非终结符，`Components` 可以是终结符、非终结符、语义动作。

在 `{...}` 中，通过 C 语言实现语义的动作，`$$` 表示 Result 的


value:
	VARIABLE
        | NUMBER
expression:
          value '+' value
        | value '-' value

这意味着表达式可以是几种格式中的任意一种；例如，一个变量、一个加号或者一个数字都 可以是一个表达式。管道字符（ | ）表明可供选择。


#### 终结符 VS. 非终结符

通过 Flex 生成的符号称为终结符 (Terminals) 或者标记 (Tokens)，从它们装配而来的内容称为非终结符 (Non-Terminals)。

所以，在上述的例子中，`NUMBER` 是一个终结符；value 是一个非终结符，它通过装配终结符而创建出来的。

#### %start

默认语法规则中出现的第一个非终结符是开始符号，也可以通过 `%start symbol` 来明确指定。

#### %token

通常用来指定从 flex 解析获取的符号类型，如上，可以是终结符或者标记。


## Token VS. Type

Bison 会调用 Flex 的 `yylex()` 来获得标志 (Token)，其中与标志对应的值由 Lex 放在变量 yylval 中，而 yylval 的类型由 `YYSTYPE` 决定，默认是 int 。

也就是说，在 Bison 调用 `yylex()` 返回类型后，同时需要将对应的值通过 `yylval` 将值从 Flex 传递到 Bison 中。

### %union

如果有多个值类型，则需要通过 `%union` 列举出所有的类型，此时 `yylval` 的类型就是上述的 `union` 结构体。

然后，需要为每个符号定义相对的类型，其中终结符使用 `%token`，非终结符使用 `%type` 来定义。

%union {
	long value;
}
%token <value>  NUMBER
%type <value>   expression

这样，当 Bison 解析器得到 Flex 返回的 NUMBER 记号时，它可以变量 yylval 中名为 value 的成员已经被赋值；这只是一个约定，同时需要在 Flex 中添加如下内容。

[0-9]+  { yylval.value = atol(yytext); return NUMBER; }

上述通过 `%type` 声明了 `expression` 为非终结符，同时还使用了 `union yylval` 中的 `value` 成员；另外，`NUMBER` 为终结符。

注意，在不同的 Token 返回时，不要将值赋值到错误的对象。

### 使用

在 Bison 的规则中，可以通过符号名引用表达式的组成部分，其中返回值为 `$$` ，其它分别为 `$1` 到 `$N` 。

expression:
	NUMBER '+' NUMBER { $$ = $1 + $3; }

注意，其中 `$2` 对应的是 `+` 号，只作为一个占位符，实际上没有任何意义。

在介绍原理的时候，会保存一个栈分析栈，实际上还通过 `yyvsp` 保存了对应的值，这也就是通过 `$N` 转换保存的值。

#### 其它

* `%error-verbose` 显示详细的错误信息。
https://www.ibm.com/support/knowledgecenter/zh/ssw_aix_61/com.ibm.aix.genprogc/ie_prog_4lex_yacc.htm

flex 通过 yylval 将数据传递给 yacc；如果在 yacc 中使用了 `%union` ，那么各个条件的目的变量使用 yyval 。

https://my.oschina.net/zhoukuo/blog/330089

编译器粗略分为词法分析、语法分析、类型检查、中间代码生成、代码优化、目标代码生成、目标代码优化。

把中间代码生成及之前阶段划分问编译器的前端，从而做到后端与前端独立。后端只需要一种中间代码表示，可以是三地址代码或四元式等，而这些都与前端生成的方式无关。

## 计算器

只进行整数的加减乘除。

/* FILE: calc.l */
%option noyywrap
%{
        #include <stdlib.h>
        #include "calc.tab.h"
%}
DIGIT   [0-9]
PUNCT   [-+*/\n]
%%
{DIGIT}+    { yylval = atoi(yytext); return TOK_NUMBER; }
{PUNCT}     { return yytext[0]; }
.           {}
%%


/* FILE: calc.y */
%{
        #include <stdio.h>
        int yylex(void);
        void yyerror(char *message);
%}
%token  TOK_NUMBER
%left '+' '-'
%left '*' '/'
%start  statement
%%
statement:
        statement expression '\n' {printf("Result: %d\n", $2);}
        | ;
expression:
        TOK_NUMBER                  {$$ = $1;}
        | expression '+' expression {$$ = $1 + $3;}
        | expression '-' expression {$$ = $1 - $3;}
        | expression '*' expression {$$ = $1 * $3;}
        | expression '/' expression {$$ = $1 / $3;};
%%
int main(void)
{
        yyparse();
}

void yyerror(char *errmsg)
{
        fprintf(stderr, "ERROR: %s\n", errmsg);
}

# FILE: Makefile
all:
        flex --outfile=calc.yy.c calc.l
        bison -d --output=calc.tab.c calc.y
        gcc calc.tab.c calc.yy.c -o calc

clean:
        rm *.tab.c *.tab.h *.yy.c -f calc


## Yacc 原理

在解析语法的过程中，意味着可以选择多个分支，yacc 会根据规则选择默认的分支，但是，冲突意味着某些地方可能会出现异常。

Yacc 属于 LR 解析器，类似于一个有限的状态机，有所不同的是，Yacc 同时会有一个栈保存终止符，可以 pop 也可以 push ，这也就是为什么能支持上下文无关的语法。

### 递进 规约

简单来说，Yacc 就是不断的执行递进和规约，每次都是不断地匹配语法右侧规则。

所谓的递进 (Shitf) 就是不断地读取符号，并添加到栈中；当最上层的栈满足某个语法规则时 (如 `A->x y z`) ，那么就会执行规约 (Reduce) 操作，也就是将 `x y z` 弹出，将 `A` 压入。

也就是说，递进是在等待满足条件的语法；而规约则是已经找到了。

### 冲突

总共有两种类型的冲突：A) Shift-Reduce；B) Reduce-Reduce；如果要查看具体的冲突，可以通过 `-v` 参数输出调试信息，也就是 `y.output` 文件。

#### Shift-Reduce

此时解析器需要判断是执行 Shift 还是执行 Reduce ，默认是前者。

例如，有如下的语法规则。

S -> 0 S 0
S -> e     /* epsilon */

对应到 Bison 的代码为

%%
S : '0' S '0'
  | /* epsilon */
  ;

然后，通过如下方式编译。

$ bison -d -v hello.y
hello.y: warning: 1 shift/reduce conflict [-Wconflicts-sr]

#### Reduce-Reduce

当执行规约时面临着两个 Reduce 的选择，默认选择第一个。

关于冲突的介绍，以及如何解决冲突
https://www2.cs.arizona.edu/~debray/Teaching/CSc453/DOCS/conflicts.pdf

## 参考

从入门到精通
http://read.pudn.com/downloads139/doc/fileformat/597500/Lex%E5%92%8CYacc%E4%BB%8E%E5%85%A5%E9%97%A8%E5%88%B0%E7%B2%BE%E9%80%9A.pdf
* [Guido van Rossum](https://medium.com/@gvanrossum_83706) Python 作者的 Blog ，初篇介绍了很多关于解析器的内容。
* [Lex and YACC HOWTO](http://tldp.org/HOWTO/Lex-YACC-HOWTO.html) 关于 Lex 和 Yacc 的详细介绍，可以作为入门。
* [龙书](https://www.epaperpress.com/lexandyacc/index.html) 中的相关介绍，很多不错的资源，包括了 PDF 以及源码。
http://www.calvinneo.com/2016/07/29/flex%E5%92%8Cbison%E4%BD%BF%E7%94%A8/
http://www.calvinneo.com/2016/07/22/%E8%AF%AD%E6%B3%95%E5%88%86%E6%9E%90/



{% highlight text %}
{% endhighlight %}
