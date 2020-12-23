---
title: Bison 简介
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:
---

bison 读入一个 CFG 文法的文件，在程序内经过计算，输出一个 parser generator 的 c 文件；也就是说 Bison 适合上下文无关文法，采用 LALR Parser (LALR语法分析器)。


<!-- more -->

## 简介

在实现时，bison 会创建一组状态，每个状态用来表示规则中的一个可能位置，同时还会维护一个堆栈，这个堆栈叫做分析器堆栈 (parser stack)。每次读入一个终结符 (token)，它会将该终结符及其语意值一起压入堆栈，把一个 token 压入堆栈通常叫做移进 (shifting)。

当已经移进的后 n 个终结符可以与一个左侧的文法规则相匹配时，这个 n 各终结符会被根据那个规则结合起来，同时将这 n 个终结符出栈，左侧的符号如栈，这叫做归约 (reduction)。

如果可以将 bison+flex 混合使用，当语法分析需要输入标记 (token) 时，就会调用 `yylex()` ，然后匹配规则，如果找到则返回。

<!--
GNU Bison是主要采用 `LALR(1)` 的解析器生成器，支持显式指定规则的优先级以及结合性。 


<br><br><h2>预读</h2><p>
Bison 分析器并不总是当 N 个终结符与组匹配某一规则时立即进行归约，这种策略对于大部分语言来说并不合适。相反，当可以进行归约时，分析器有时会“预读” (looks ahead) 下一个终结符来决定做什么。<br><br>

当一个终结符被读进来后，并不会立即移进堆栈，而是首先作为一个预读终结符 (look-ahead token)。此后，分析器开始对栈上的终结符和组执行一个或多个归约，而预读终结符仍然放在一边。当没有归约可做时，这个预读终结符才会被移进堆栈。这并不表示所有可能的归约都已经做了，这要取决于预读终结符的类型，一些规则可能选择推迟它们的使用。<br><br>

 主程序分为如下几个步骤进行:
    1. 读取/解析命令行选项 Main.getargs(), 打开文件 Files.openfiles()
       这些是程序基本的准备工作, 一般不属于算法范畴, 我们略去不述.
    2. 读入文法文件 Reader.reader(), 文法文件一般以 .y 为后缀.
       读取时建立内存的一些中间数据结构(主要是单链结构), 后面详述.
    3. 将第2步读入的数据进行检查, 消除无用的,错误的产生式, 建立/转换为适合
       计算 LR 的数据结构(主要是单链=>数组结构). 预先计算一些辅助数据.
    4. 计算 LR0 状态集, 结果可能是一个非确定的(有冲突的)有限状态机.
    5. 转变第 4 步的状态机为确定的 LALR 状态机.
    6. 如果第 5 步中有 s/r, r/r 冲突, 则解决冲突.
    7. 输出及别的收尾工作. 一般略去不细述了.
-->

### 文件格式

两个文件的格式基本类似，分成三部分，都是通过 `%%` 进行分割，如下所示。

{% highlight text %}
... definitions ...
%%
... rules ...
%%
... subroutines ...
{% endhighlight %}

其中：A) `definitions` 定义模式、C语言变量、以及包含c头文件等；B) `rules` 部分用户定义模式对应的动作；C) `subroutines` 部分用于定义 C 函数等。

### 示例

读取 `Hi` `Bye` 并输出相关的内容。

{% highlight text %}
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
{% endhighlight %}

{% highlight text %}
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
{% endhighlight %}

{% highlight text %}
# Makefile
all:
        flex --outfile=hello.yy.c hello.l
        bison -d --output=hello.tab.c hello.y
        gcc hello.tab.c hello.yy.c -o hello

clean:
        rm *.tab.c *.tab.h *.yy.c -f hello
{% endhighlight %}

可以直接执行 hello ，然后输入 `Hi` `Bye` 加上回车即可。

<!--
同 flex 相似，仍然通过 ```%%``` 将文件分为三部分：
1. 第一部分将原封不动放到生成源码中，如果要写入 C/C++ 代码，则必须用 ```%{``` 和 ```%}``` 括起来。
2. 第二部分是规则段，包括了模式 (正则表达式) 和动作，由空白分开，当匹配到模式时，就会执行后面的动作。每条规则都是由 ```':'``` 操作符左侧的一个名字、右侧的符号列表、动作代码、规则结束符 ```(;)``` 组成。
3. 第三部分可以直接写入 C/C++ 代码。

如下，是一个简单示例，分别是 frame.l 和 frame.y 文件。

{% highlight text %}
%{
int yywrap(void);
%}
%%
%%
int yywrap(void)
{
    return 1;
}
{% endhighlight %}

{% highlight text %}
%{
void yyerror(const char *s);
%}
%%
program:
   ;
%%
void yyerror(const char *s)
{
}
int main()
{
    yyparse();
    return 0;
}
{% endhighlight %}

然后，通过如下命令进行测试。

{% highlight text %}
----- 编译生成lex.yy.c文件
$ flex frame.l

----- 产生frame.tab.c和frame.tab.h文件
$ bison -d frame.y

----- 编译生成二进制文件
$ gcc frame.tab.c lex.yy.c
{% endhighlight %}
-->

## 规则

定了语法的产生以及语义的动作，一般规则为 `Result: Components {...};`，其中 `Result` 为非终结符，`Components` 可以是终结符、非终结符、语义动作。

在 `{...}` 中，通过 C 语言实现语义的动作，`$$` 表示 Result 的


{% highlight text %}
value:
	VARIABLE
        | NUMBER
expression:
          value '+' value
        | value '-' value
{% endhighlight %}

这意味着表达式可以是几种格式中的任意一种；例如，一个变量、一个加号或者一个数字都 可以是一个表达式。

yacc 中定义了很多的符号，详细的可以查看 [Bison Symbols](http://dinosaur.compilertools.net/bison/bison_13.html) 中的介绍，如下简单介绍常见的符号定义：

{% highlight text %}
%start foobar
  修改默认的开始规则，例如从foobar规则开始解析，默认从第一条规则开始
%token TOKEN1 TOKEN2 TOKEN3 ...
  通常用来指定从 flex 解析获取的符号类型，如上，可以是终结符或者标记。
%left，%right，%nonassoc
  类似于终结符，不过同时具有某种优先级和结核性，分别表示左结合、右结合、不结合 (也就是终结符不能连续出现，
     例如<，此时不允许出现a<b<c这类句子)。
  优先级与其定义的顺序相关，先定义的优先级低，最后定义的优先级最高，同时定义的优先级相同。
     例如，如上程序关于计算器中优先级的定义。
{% endhighlight %}

### 终结符 VS. 非终结符

通过 Flex 生成的符号称为终结符 (Terminals) 或者标记 (Tokens)，从它们装配而来的内容称为非终结符 (Non-Terminals)。

所以，在上述的例子中，`NUMBER` 是一个终结符；value 是一个非终结符，它通过装配终结符而创建出来的。


## Token VS. Type

Bison 会调用 Flex 的 `yylex()` 来获得标志 (Token)，其中与标志对应的值由 Lex 放在变量 yylval 中，而 yylval 的类型由 `YYSTYPE` 决定，默认是 int 。

也就是说，在 Bison 调用 `yylex()` 返回类型后，同时需要将对应的值通过 `yylval` 将值从 Flex 传递到 Bison 中。

### %union

如果有多个值类型，则需要通过 `%union` 列举出所有的类型，此时 `yylval` 的类型就是上述的 `union` 结构体。

需要为每个符号定义相对的类型，其中终结符使用 `%token`，非终结符使用 `%type` 来定义。

{% highlight text %}
%union {
	long value;
}
%token <value>  NUMBER
%type <value>   expression
{% endhighlight %}

这样，当 Bison 解析器得到 Flex 返回的 NUMBER 记号时，它可以变量 yylval 中名为 value 的成员已经被赋值；这只是一个约定，同时需要在 Flex 中添加如下内容。

{% highlight text %}
[0-9]+  { yylval.value = atol(yytext); return NUMBER; }
{% endhighlight %}

上述通过 `%type` 声明了 `expression` 为非终结符，同时还使用了 `union yylval` 中的 `value` 成员；另外，`NUMBER` 为终结符。

注意，在不同的 Token 返回时，不要将值赋值到错误的对象。

### 使用

在 Bison 的规则中，可以通过符号名引用表达式的组成部分，其中返回值为 `$$` ，其它分别为 `$1` 到 `$N` 。

{% highlight text %}
expression:
	NUMBER '+' NUMBER { $$ = $1 + $3; }
{% endhighlight %}

注意，其中 `$2` 对应的是 `+` 号，只作为一个占位符，实际上没有任何意义。

在介绍原理的时候，会保存一个栈分析栈，实际上还通过 `yyvsp` 保存了对应的值，这也就是通过 `$N` 转换保存的值。

### 其它

* `%error-verbose` 显示详细的错误信息。

## 冲突

在解析语法的过程中，意味着可以选择多个分支，yacc 会根据规则选择默认的分支，但是，冲突意味着某些地方可能会出现异常。

Yacc 属于 LR 解析器，类似于一个有限的状态机，有所不同的是，Yacc 同时会有一个栈保存终止符，可以 pop 也可以 push ，这也就是为什么能支持上下文无关的语法。

### 递进 规约

简单来说，Yacc 就是不断的执行递进和规约，每次都是不断地匹配语法右侧规则。

所谓的递进 (Shitf) 就是不断地读取符号，并添加到栈中；当最上层的栈满足某个语法规则时 (如 `A->x y z`) ，那么就会执行规约 (Reduce) 操作，也就是将 `x y z` 弹出，将 `A` 压入。

也就是说，递进是在等待满足条件的语法；而规约则是已经找到了。

总共有两种类型的冲突：A) Shift-Reduce；B) Reduce-Reduce；如果要查看具体的冲突，可以通过 `-v` 参数输出调试信息，也就是 `y.output` 文件。

### Shift-Reduce

此时解析器需要判断是执行 Shift 还是执行 Reduce ，默认是前者。

例如，有如下的语法规则。

{% highlight text %}
S -> 0 S 0
S -> e     /* epsilon */
{% endhighlight %}

对应到 Bison 的代码为

{% highlight text %}
%%
S : '0' S '0'
  | /* epsilon */
  ;
{% endhighlight %}

然后，通过如下方式编译。

{% highlight text %}
$ bison -d -v hello.y
hello.y: warning: 1 shift/reduce conflict [-Wconflicts-sr]
{% endhighlight %}

#### Reduce-Reduce

当执行规约时面临着两个 Reduce 的选择，默认选择第一个。

## 其它

### 字符串解析

通过如下方式可以设置使用内存字符串而非文件：


{% highlight text %}
YY_BUFFER_STATE yy_scan_string(const char *str);
YY_BUFFER_STATE yy_scan_bytes(const char *bytes, int len);
{% endhighlight %}

这里会返回一个 `YY_BUFFER_STATE` 类型，在使用完之后，需要通过 `yy_delete_buffer()` 删除，也就是在通过 `yylex()` 解析前会先复制一份数据，然后解析时会修改缓存。

如果不希望复制，那么可以使用如下函数。

{% highlight text %}
YY_BUFFER_STATE yy_scan_buffer(char *base, yy_size_t size)
{% endhighlight %}

下面是一个简单的示例：

{% highlight c %}
int main() {
	yy_scan_string("a test string");
	yylex();
}
{% endhighlight %}

<!--
这个例子中，我们定义STATE 为一个整数，这点跟前面一样，NUMBER符号用于读取温度值。

不过新的WORD被定义为一个字符串。

分词器文件也有很多改变：

%{
#include <stdio.h>
#include <string.h>
#include "y.tab.h"
%}
%%
[0−9]+             yylval.number=atoi(yytext); return NUMBER;
heater             return TOKHEATER;
heat               return TOKHEATER;
on|off             yylval.number=!strcmp(yytext,"on"); return STATE;
target             return TOKTARGET;
temperature        return TOKTEMPERATURE;
[a−z0−9]+          yylval.string=strdup(yytext);return WORD;
\n                 /* ignore end of line */;
[ \t]+             /* ignore whitespace */;
%%

如你所见，我们不再直接获取yylval的值，而是添加一个后缀指示想取得哪个部分的值。不过在YACC语法中，我们无须这样做，因为YACC为我们做了神奇的这些：

heater_select:
        TOKHEATER WORD
        {
            printf("\tSelected heater '%s'\n",$2);
            heater=$2;
        }
        ;

由于上面的%token定义，YACC自动从联合体中挑选string成员。同时也请注意，我们保存了一份$2的副本，它在后面被用于告诉用户是哪一个加热器发出的命令：

target_set:
        TOKTARGET TOKTEMPERATURE NUMBER
        {
            printf("\tHeater '%s' temperature set to %d\n",heater,$3);
        }
        ;
-->

### 变量

```$$ $1 $2 ...``` 定义了默认的参数，示例如下：

{% highlight text %}
exp:
| exp '+' exp     { $$ = $1 + $3; }

exp[result]:
| exp[left] '+' exp[right]  { $result = $left + $right; }
{% endhighlight %}



<!--
上述函数的作用为<ul><li>
    yywrap<br>
    可以在lex或者yacc文件中定义，该函数是必须的，给了这个函数实现之后不再需要依赖flex库，在次只是简单返回1，表示输入已经结束。函数yywrap能够用于是否继续读取其它的文件，当遇到EOF时，你可以打开其它文件并返回0。或者，返回1，意味着真正的结束。</li><br><li>

    program<br>
    这是语法规则里面的第一个非终结符，注意上面的格式哦：“program”后 面紧跟着一个冒号“:”，然后换行之后有一个分号“;”，这表明这个 program是由空串组成的。至于什么是非终结符以及什么是终结符，还有什 么是语法规则都会在后面的章节中进行详细介 绍。</li><br><li>

    yyerror<br>
    错误处理函数，为了保证代码尽可能的简洁，在此什么都不做。</li><br><li>

    yyparse<br>
    这个函数是yacc生成的，在代码里可以直接使用。lex生成的函数为yylex，实际上yyparse还间接调用了yylex函数，可以查看生成的C源文件。
</li></ul>
</p>


* lex/yacc程序组成结构、文件格式。
* 如何在lex/yacc中使用C++和STL库，用extern "C"声明那些lex/yacc生成的、要链接的C函数，如yylex(), yywrap(), yyerror()。
* 重定义YYSTYPE/yylval为复杂类型。
* lex里多状态的定义和使用，用BEGIN宏在初始态和其它状态间切换。
* lex里正则表达式的定义、识别方式。
* lex里用yylval向yacc返回数据。
* yacc里用%token<>方式声明yacc记号。
* yacc里用%type<>方式声明非终结符的类型。
* 在yacc嵌入的C代码动作里，对记号属性($1, $2等)、和非终结符属性($$)的正确引用方法。
* 对yyin/yyout重赋值，以改变yacc默认的输入/输出目标。





<br><br><h2>调试</h2><p>
通常来说 bison 生成 *.tab.{c,h} 两个文件，如果通过 --report=state 或者 --verbose 生成 *.output 输出。
-->




<!--
<ul><li>
token 标记<br>
在yacc文件中进行定义，通过bison生成头文件，头文件中会定义为宏或者enmu(yacc生成的.c文件直接使用数字，在yytoknum[]中)，通常从258开始，通常1～265表示字符，还有一些内部的定义。如frame.y，则生成frame.tab.h，在frame.l中通常需要包含该头文件。</li><br><li>

    int yyparse(void)/int yylex(void)<br>
    yyparse()为bison的执行入口，程序可以直接通过该函数执行；yyparse()会通过yylex()获得token。
    </li></ul>
    flex frame.l<br>
    bison -d frame.y<br>
    cc frame.tab.c lex.yy.c -ll -o example<br><br>

    如果需要进行调试需要通过如下方式进行编译<br>
    flex frame.l<br>
    bison -y -d -t frame.l<br>
    cc -g frame.tab.c lex.yy.c -ll -o example<br>
    gdb example<br>
    (gdb) set yydebug=1<br>
    (gdb) continue
    </p>
-->

## 示例程序

实现一个简单的计算器程序，能进行加、减、乘、除、幂运算，需要注意优先级。

`calc.l` 文件。

{% highlight text %}
%{
    #include "calc.tab.h"
    #include <stdlib.h>
    void yyerror(char *);
%}

%%

[a-z]       {
                yylval = *yytext - 'a';
                return VARIABLE;
                }

[0-9]+      {
                yylval = atoi(yytext);
                return INTEGER;
            }

[-+()=/*\n]     { return *yytext; }

[ \t]   ;       /* skip whitespace */

.               yyerror("Unknown character");

%%

int yywrap(void)
{
	return 1;
}
{% endhighlight %}

`calc.y` 文件。

{% highlight text %}
%{
	#include <stdio.h>
	void yyerror(char *);
	int yylex(void);
	int sym[26];
%}

%token INTEGER VARIABLE
%left '+' '-'
%left '*' '/'

%%

program:
	program statement '\n'
	| /* NULL */
	;

statement:
	expression                      { printf("%d\n", $1); }
	| VARIABLE '=' expression       { sym[$1] = $3; }
	;

expression:
	INTEGER
	| VARIABLE                      { $$ = sym[$1]; }
	| expression '+' expression     { $$ = $1 + $3; }
	| expression '-' expression     { $$ = $1 - $3; }
	| expression '*' expression     { $$ = $1 * $3; }
	| expression '/' expression     { $$ = $1 / $3; }
	| '(' expression ')'            { $$ = $2; }
	;

%%

void yyerror(char *s)
{
	fprintf(stderr, "%s\n", s);
}

int main(void)
{
	yy_scan_string("1+1\n");
	yyparse();
}
{% endhighlight %}


{% highlight text %}
all:
	bison -d calc.y
	flex -o calc.lex.c calc.l
	gcc calc.lex.c calc.tab.h calc.tab.c -o calc -lm

clean:
	rm -f calc.lex.c calc.tab.c calc.tab.h calc test
{% endhighlight %}


<!--
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
-->

## 参考

* [Guido van Rossum](https://medium.com/@gvanrossum_83706) Python 作者的 Blog ，初篇介绍了很多关于解析器的内容。
* [Lex and YACC HOWTO](http://tldp.org/HOWTO/Lex-YACC-HOWTO.html) 关于 Lex 和 Yacc 的详细介绍，可以作为入门。
* [龙书](https://www.epaperpress.com/lexandyacc/index.html) 中的相关介绍，很多不错的资源，包括了 PDF 以及源码。


<!--
关于冲突的介绍，以及如何解决冲突
https://www2.cs.arizona.edu/~debray/Teaching/CSc453/DOCS/conflicts.pdf

## 参考

从入门到精通
http://read.pudn.com/downloads139/doc/fileformat/597500/Lex%E5%92%8CYacc%E4%BB%8E%E5%85%A5%E9%97%A8%E5%88%B0%E7%B2%BE%E9%80%9A.pdf
http://www.calvinneo.com/2016/07/29/flex%E5%92%8Cbison%E4%BD%BF%E7%94%A8/
http://www.calvinneo.com/2016/07/22/%E8%AF%AD%E6%B3%95%E5%88%86%E6%9E%90/

-->

{% highlight text %}
{% endhighlight %}
