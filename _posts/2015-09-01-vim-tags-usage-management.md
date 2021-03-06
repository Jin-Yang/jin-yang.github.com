---
title: VIM Tags 相关
layout: post
comments: true
language: chinese
category: [misc]
keywords: vim,编辑器
description:
---

符号索引是个重要功能，不论阅读新项目，还是开发复杂点的大项目，符号索引都能帮你迅速掌握项目脉络，加快开发进度。

在 VIM 中，大部分的代码浏览都要依赖 tags 的使用。

<!-- more -->

## 简介

传统 ctags 系统虽和 vim 结合紧密，但只能查定义无法查引用，而 cscope 能查引用，但只支持 C 语言，甚至连 C++ 都不支持，况且常年不更新。

<!--
ctags 由于使用文本格式存储数据，虽用了二分查找，但打开 Linux Kernel 这样的大项目时，查询会有卡顿的感觉。
-->

建议使用 gtags 替换，在 Vim8 支持了异步模式之后，自动创建索引变得十分方便。

相比来说，gtags 由如下的几个特点：

* 不但能查定义，还能查引用；
* 原生支持 6 种语言 C C++ Java PHP4 ASM Yacc，扩展支持 50+ 种语言；
* 使用性能更好的本地数据库存储符号，而非文本；
* 支持增量更新，每次只索引改变过的文件；
* 多种输出格式，能更好的同编辑器相集成。



### Language Server Protocol, LSP

很多的 tags 工具都是静态的，例如 ctags、gtags ；另外，还有类似 Deoplete 这类插件，将前后端分离，前端用作展示，例如 VIM、VS、Emacs 等，后端则作补全、校验等。

<!--
CTRL-]   在当前窗口里跳转到定义
CTRL-W ] 新窗口打开并查看光标下符号的定义
-->

<!--
## tags

tags 记录了关于一个标识符在哪里被定义的信息，比如 C/C++ 程序中的一个函数定义。vim 默认是支持 tags 的，那么可以让 vim 从任何位置跳转到相应的标示位置。

### 安装更新

除了 ctags 之外，还有 gtags etags cquery 等工具，相比来说后面几个更新，但是 ctags 支持 50+ 语言，而且和 VIM 配合使用时的依赖最少。

老的 Exuberant Ctags 已经停止更新快十年，可以通过 `yum install ctags` 安装，也可以用 [Universal CTags](https://ctags.io/) 替换之，该工具是在原有的 Exuberant Ctags 继续迭代更新。

下载源码后通过如下方式编译安装。

{% highlight text %}
$ ./autogen.sh                # 依赖automake autoconf
$ ./configure --prefix=/usr
$ make
# make install
{% endhighlight %}

编译后只生成了一个 ctags 可执行文件，会安装到 `/usr/bin` 目录下。

注意，需要添加 `--output-format=e-ctags` 参数与老的 ctags 格式兼容，否则 Windows 下会有部分的兼容性问题。

{% highlight text %}
ctags -R --c++-kinds=+px --fields=+iaS --extra=+q .

常用参数：
-R
   遍历循环子目录生成tags；
--fields=+iaS
  将可用扩展域添加到tags中，
    i) 如有继承，则标识出父类；
    a) 标明类成员的权限，如public、private等；
    S) 函数的信息，如原型、参数列表等；
-I identifier-list
   通常用于处理一些宏，如果只列出了那么则会忽略；
--c++-kinds=+px
   记录c++文件中的函数声明和各种外部和前向声明，使用p时同时也会添加extern的声明；
--extra=+q
  是否将特定信息添加到tags中，q) 类成员信息，包括结构体；

其它常用命令：
----- 列举出当前支持的语言，也可以自定义，具体没研究过
$ ctags --list-languages
----- 查看扩展名跟语言的映射关系，从而可以使用正确的分析器
$ ctags --list-maps
----- 可识别的语法元素，默认打印所有，在生成时可以通过--c-kinds指定
$ ctags --list-kinds=c

生成的文件格式如下：
{tagname} {TAB} {tagfile} {TAB} {tagaddress} {term} {field} ..
   {tagname}     标识符名字，例如函数名、类名、结构名、宏等，不能含TAB符。
   {tagfile}     包含 {tagname} 的文件。
   {tagaddress}  可以定位到 {tagname} 光标位置的 Ex 命令，通常只包含行号或搜索命令。
                 出于安全的考虑，会限制其中某些命令的执行。
   {term}        设为 ;” ，主要是为了兼容vi编辑器，使vi忽略后面的{field}字段。
   {field} ..    也就是扩展字段，可选，用于表示此 {tagname} 的类型是函数、类、宏或是其它。

常见快捷键如下：
Ctrl+]       跳转到定义处；
Ctrl+T       跳转到上次tags处；
Ctrl+i       (in)跳转下一个；
Ctrl+o       (out)退回原来的地方；
gd           转到当前光标所指的局部变量的定义；
gf           打开头文件；
:ju          显示所有可以跳转的地方；
:set tags    查看加载的tags；
:tag name    调转到name处；
:stag name   等价于split+tag name
:ta XXX      跳转到符号XXX定义处，如果有多个符号，直接跳转到第一处；
:ts XXX      列出符号XXX的定义；
:tj XXX      可看做上面两个命令的合并，如果只找到一个符号定义，那么直接跳转，有多个，则让用户自行选择；
:ptag name   预览窗口显示name标签，光标跳到标签处；
Ctrl-W + }   预览窗口显示当前光标下单词的标签，光标跳到标签处；
:pclose      关闭预览窗口；
:pedit file.h 在预览窗口中编辑文件file.h，在编辑头文件时很有用；
:psearch atoi 查找当前文件和头文件中的单词并在预览窗口中显示匹配，在使用没有标签文件的库函数时十分有用。
{% endhighlight %}

如果有多个可以使用 `tfirst` `tlast` `tprevious` `tnext` `tselect` 选择，也可以 `:tag name_<TAB>` 自动补全，或者使用 `tselect /^write` 正则表达式。

### 生成系统tags

{% highlight text %}
----- 添加系统的tags
$ ctags --fields=+iaS --extra=+q -R -f ~/.vim/systags /usr/include /usr/local/include
:set tags+=~/.vim/systags
{% endhighlight %}

此时，基本可以跳转到系统函数，不过仍有部分函数未添加到tags中，常见的有如下的示例。

{% highlight c %}
extern int listen (int __fd, int __n) __THROW;
extern int strcmp (__const char *__s1, __const char *__s2)
     __THROW __attribute_pure__ __nonnull ((1, 2));
{% endhighlight %}

也就是因为存在 `__THROW` `attribute_pure` `nonull` 等属性，导致认为上述的声明不是函数，都需要忽略。如果需要 `#if 0` 里面的定义，可以使用 `-if0=yes` 来忽略 `#if 0` 这样的定义。

{% highlight text %}
$ ctags -I __THROW -I __attribute_pure__ -I __nonnull -I __attribute__ \   忽略这里的定义
    --file-scope=yes              \     例如对于static声明只在一个文件中可见
    --langmap=c:+.h               \     定义扩展名和语言的映射关系，可以通过--list-maps查看
    --languages=c,c++             \     使能哪些语言
    --links=yes                   \     是否跟踪符号链接指向的文件
    --c-kinds=+p --c++-kinds=+p   \     指定生成哪些C语言的tag信息
    --fields=+iaS --extra=+q -R -f ~/.vim/systags /usr/include /usr/local/include
{% endhighlight %}

可以在配置文件中添加如下的内容，然后在源码目录内可以通过 `Ctrl-F12` 生成 tags 文件。

{% highlight text %}
map <C-F12> :!ctags -R --c-kinds=+px --fields=+iaS --extra=+q <CR>
{% endhighlight %}

### 文件配置

{% highlight text %}
set tags=~/.vim/systags,./tags;,tags
{% endhighlight %}

分别表示使用系统 tag ，当前目录下的 `tags` 文件，后面的配置表示从当前目录一直向上查找。
-->

## gtags

实际上，gtags 是 GNU Global 软件包中的一个程序，也是最为常用的一个，类似于 ctags 的代码导航工具，可以为 vim emacs 等添加代码调转功能；另外还有 gtags-cscope 代替 cscope 。

GNU Global 内置的解析器支持 C C++ Java PHP ASM YACC 六种语言，通过 exuberant-ctags 扩展可以支持 ctags 支持的所有语言。

在 CentOS 中可以通过如下命令安装。

{% highlight text %}
# yum install --enablerepo=epel global
{% endhighlight %}

也可以从 [www.gnu.org](http://www.gnu.org/software/global/) 上下载，通过如下方式编译安装，例如 6.6.3 版本。

{% highlight text %}
$ ./configure --prefix=/usr
$ make
# make install
{% endhighlight %}

然后可以通过 `gtags --version` 查看版本信息。

### gtags-cscope

一些常用的快捷键：

* `<leader>cg` 查看光标下符号的定义；
* `<leader>cs` 查看光标下符号的引用；
* `<leader>cc` 查看有哪些函数调用了该函数；
* `<leader>cf` 查找光标下的文件；
* `<leader>ci` 查找哪些文件 include 了本文件。

## Gutentags

其中 gutentags 可以用来动态生成对应的 tags 数据库，而 gutentags_plus 则提供了无缝切换 tags 的功能。

相关内容可以参考官网 [bolt80.com](https://bolt80.com/gutentags/) 。

### 问题排查

一般的报错信息为 `gutentags: gutentags: gtags-cscope job failed, returned: 1` ，也就是说在生成 tag 数据时报错。

判断 gtags 为何失败，需进一步打开日志，查看 gtags 的错误输出：

{% highlight text %}
let g:gutentags_define_advanced_commands = 1
{% endhighlight %}

在 vimrc 中添加上面的配置，允许 gutentags 打开一些高级命令和选项，打开出错文件，执行 `:GutentagsToggleTrace` 打开日志信息，保存文件，然后触发 gtags 数据库更新。

也可以通过 `:messages` 命令列出所有消息记录，即可看到 gtags 的错误输出，方便定位。

最简单的方式，如果开始是正常的，可以直接删除掉 cache 信息。

## 其它

在编译的时候可以通过 [AsyncRun](https://github.com/skywind3000/asyncrun.vim) 异步运行，不过还是习惯用 tmux 。

<!--
非常经典的介绍，关于GTags使用介绍
https://zhuanlan.zhihu.com/p/36279445
https://blog.csdn.net/gatieme/article/details/78819740
https://www.zhihu.com/question/47691414/answer/373700711

https://www.jianshu.com/p/110b27f8361b

搭建整体IDE的介绍
https://www.zhihu.com/question/47691414










Pandas concat()介绍
https://blog.csdn.net/stevenkwong/article/details/52528616







Browsing programs with tags
https://vim.fandom.com/wiki/Browsing_programs_with_tags
https://blog.csdn.net/wdscq1234/article/details/51815531
https://xu3352.github.io/linux/2018/12/16/practical-vim-skills-chapter-16



在 VIM 会通过 tag 定位一些符号的地址，例如变量、函数等，tag 中记录了这些符号的文件以及所在行信息，从而可以在不同符号之间跳转。

详细的使用方式可以参考 `:help tagsrch.txt` 中的介绍。

跳转方式

主要介绍的是命令行以及快捷键的方式，除此之外还可以通过鼠标进行操作。

1. 通过 `:tag <tagname>` 输入命令跳转，也可以使用 `:stag <tagname>` 打开新窗口跳转。
2. 使用 `Ctrl+]` 快捷键跳转，或者 `Ctrl+W ]` 在新窗口打开。

如果有多个 tag ，可以使用 `:tselect <tag>`、`:stselect <tag>`、`g]`、`Ctrl+w g]` 几种方式查看。

实际上，在跳转之前很难确定是否存在多个 tag ，所以，可以通过 `:tjump <tag>` 或者 `Ctrl+]` 进行跳转，当有多个时候，会出现一个选择列表。

在跳转时，会将跳转的位置保存在栈中，当从上个 tag 中返回的时候，可以使用 `:pop` `Ctrl+t` 返回，也可以通过 `:tags` `:tagstack` 查看当前的跳转栈信息。

https://ctags.io/
https://zhuanlan.zhihu.com/p/36279445
https://ricostacruz.com/til/navigate-code-with-ctags
https://www.jianshu.com/p/110b27f8361b
https://www.cnblogs.com/standardzero/p/10744648.html


首先是尽量通过异步方式替换现有插件，然后针对不同的语言分别进行定制，以达到最优。如果在一个进程中的不同窗口打开不同语言的源码如何进行处理。

1. 代码浏览、补全、语法检查、模板、格式化。
2. 文件快速搜索、grep 查找。

基本上都是基于 tag 实现，关键是如何进行管理。

https://github.com/amix/vimrc
https://github.com/spf13/spf13-vim

VIM定制
http://liadbiz.github.io/vim-custimization/
tagbar 非异步的，可以使用LeaderFunction替换
https://www.cnblogs.com/wudongwei/p/9129676.html
universal-ctags
https://www.zhihu.com/question/31934850/answer/53915949

### 按键映射

除了一些基本的快捷键之外，还可以通过 vim 的按键映射将一些常见的功能映射为快捷键。

" 可使用 - 删除一行，仍可使用 dd
map - dd

上述的 `map` 实际上是 `nmap` 表示在 Normal 模式下的映射，除此还包括 `nmap` `imap` `vmap` `cmap` 分别对应了四种不同的模式。

例如，可以使用类似 Emacs 的快捷键映射，将 `Ctrl-a` `Ctrl-e` 映射为定位行首和行尾，与 VIM 中的 `I` `A` 效果相同。

nmap <c-a> I
nmap <c-e> A
imap <c-a> <esc>I
imap <c-e> <esc>A

其中 `<c-a>` 表示 `Ctrl-a` 组合按键，而 `<esc>` 也就是 `esc` 按键，还有一个 `<leader>` 指定一个前导快捷键，可以通过 `let mapleader=','` 指定。

通过一系列的组合键，就可以完成很复杂的功能。

#### 避免递归

另外，有一种场景可能会导致递归，如 `nmap dd jddk` ，当执行完 `j` 要执行 `dd` 时，会认为又需要重新映射，从而导致循环递归。这一场景，可以使用 `nnoremap` `inoremap` `vnoremap` `cnoremap` 等指令进行映射。

### Abbreviations

在输入字符的时候，VIM 会检查输入是否为 `non-keyword` 的字符，如果能够匹配到一个字则会自动替换，可以用来替换输入错误，例如。

:iabbrev na an

当输入 `This is na apple.` 时，`na` 会自动修改为 `an` 。其中单词的判断是通过 `iskeyword` 中的字符判定，只有当单词满足 `na` 的时候才会替换，例如 `animal` 则不会替换。

另外，也可以映射一些常见的快捷字符，例如：

:iabbrev @@    hello@foobar.com
:iabbrev ccopy Copyright Your Name Here, all rights reserved.

在输入模式中，输入 `@@` 或者 `ccopy` 就可以快速完成替换。

#### iskeyword

在 normal 模式中，通过 `#` `*` 查找字符时，会听过 iskeyword 的配置项来决定是否一个单词，可以通过 `:set iskeyword?` 命令查看，默认为 `iskeyword=@,48-57,_,192-255` ，通过 `,` 进行分割，对应了：

* `@` 通过 `isalpha()` 确认为真的所有字符；
* `48-57` 实际上就是数字 `0` 到 `9` ；
* 下划线 `_` ；
* ASCII 中的 `192~255` ，已经超过了标准 ASCII 的范围，暂可以忽略。

只要是 `iskeyword` 指定的字符链接在一起，那么都会被算作是一个单词，例如 `abc` `abc123foo` `abc_123_foo` 等。

#### 为什么不用 map

在 Insert 模式中，可以通过如下设置映射。

:inoremap copy Copyright Your Name Here, all rights reserved.

在输入的时候，会等待 `copy` 字符串，而且即使因为输入 `copyright` 时，也会被替换，而之上的方式也可以输入 `ccopyright` 类似字符。

## buffers

    A buffer is the in-memory text of a file.
    A window is a viewport on a buffer.
    A tab page is a collection of windows.

tab 只是合并了很多的 Windows ，如果已经打开了很多 buffer ，那么在这个 Windows 仍然可以打开所有，与经常使用的 tab 会略有区别。

vim-airline 已经支持 buffers 的显示，直接打开对应的选项就可以了。

" Enable the list of buffers
let g:airline#extensions#tabline#enabled = 1

" Show just the filename
let g:airline#extensions#tabline#fnamemod = ':t'

https://github.com/vim-airline/vim-airline/issues/1108
https://github.com/vim-airline/vim-airline/issues/1389
https://github.com/vim-airline/vim-airline#smarter-tab-line

https://joshldavis.com/2014/04/05/vim-tab-madness-buffers-vs-tabs/
-->


{% highlight text %}
{% endhighlight %}
