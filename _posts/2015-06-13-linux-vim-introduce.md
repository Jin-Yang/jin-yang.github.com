---
title: VIM 基本功能
layout: post
comments: true
language: chinese
category: [misc]
keywords: vim,编辑器
description: Vim 是一个功能强大、高度可定制的文本编辑器，在 Vi 的基础上改进和增加了很多特性。与其相匹敌的是 Emacs ，这两个都是不错的编辑器，在此不再比较两者的优劣，仅介绍 Vim 相关的内容。
---

Vim 是一个功能强大、高度可定制的文本编辑器，在 Vi 的基础上改进和增加了很多特性。与其相匹敌的是 Emacs ，这两个都是不错的编辑器，在此不再比较两者的优劣，仅介绍 Vim 相关的内容。

<!-- more -->

![vim logo]({{ site.url }}/images/misc/vim_logo.png "vim logo"){: .pull-center width="25%" }

## 简介

除了 Vim 之外，还可以使用 NeoVim ，其中后者是较新的一个版本，据说是一个刚烈的汉子对 Vim 不满而做的兼容开发，核心代码基本没变，但是有很多有用的特性。

在安装配置时使用的是 CentOS ，其它发行版本可能会有些出入。

有些插件，例如 vim-go 就会对 vim 的版本有要求，一般是大于 `Vim 7.4.1689` 或者 `Neovim` 才可以；`YouCompleteMe` 要求 `Vim 7.4.1578` 以上版本，所以有时需要从源码开始编译安装。

### 源码安装

很多的一些插件依赖一些高版本的特性，所以需要手动编译安装，直接从 [www.vim.org/download](https://www.vim.org/download.php) 或者 [github](https://github.com/vim/vim) 上下载相关的版本。

{% highlight text %}
# yum remove vim-common vim-enhanced vim-filesystem vim-minimal
$ git clone https://github.com/vim/vim.git
$ cd vim
$ ./configure -h
$ ./configure --with-features=huge                                      \
    --enable-pythoninterp                                               \
    --with-python-config-dir=/usr/lib/python2.7/config-x86_64-linux-gnu \
    --enable-multibyte                                                  \   多字节，支持UTF8
    --enable-rubyinterp                                                 \
    --enable-perlinterp                                                 \
    --enable-luainterp                                                  \
    --enable-cscope --prefix=/usr                                       \
    --disable-selinux                                                   \
    --enable-gui=auto --enable-xim  --with-x --enable-fontset               也可以使用gnome或者gtk2
$ make VIMRUNTIMEDIR=/usr/share/vim/vim80
# make install
# checkinstall
$ vim --version
{% endhighlight %}

此时会安装到 `/usr/bin` 目录下，也就是 `/usr/bin/vim` ，安装时基本依赖 `ncurses-devel` 包

#### Python 支持

也即是支持对 Python 编写插件的支持，上述支持的是 Python2 ，如果要支持 Python3 需要添加 `--enable-python3interp` 配置项，同时添加相关的 `--with-python-config-dir` 配置。

需要通过 `yum install python-devel` 安装相关的开发版本。 

#### Perl 支持

在编译时会依赖 `xsubpp` 命令生成 `src/auto/if_perl.c` 文件，在 CentOS 中该命令依赖可以通过 `yum install perl-ExtUtils-Embed` 安装。

### 常用操作

启动 Vim ，默认会进入 Normal 模式；按下键 i 进入 Insert 模式；此时，可以输入文本；按下 ESC 键，返回 Normal 模式。注意：在 Normal 模式下，所有的按键都是功能键。

通过 ```:help <command>``` 可以查看相关命令的帮助文件，或者直接通过 ```:help``` 查看帮助文件。

首先是常见的插入、替换操作。

{% highlight text %}
===== 插入操作 :help inserting
a                   → 在当前光标字符后插入
A                   → 在当前行的末尾插入
i                   → 在当前光标字符前插入
I                   → 在当前行的第一个非空字符前插入
o                   → 在当前行后插入一个新行，也即打开新行
O                   → 在当前行前插入一个新行

===== 替换操作 :help replacing
s                   → substitute替换当前字符，删除字符+进入插入模式
S                   → 替换一行，删除行+进入插入模式
r                   → 替换一个字符，进入替换模式，自动回到normal模式
R                   → 可以替换多个连续字符，Esc退出Replace模式
c                   → change改变选中内容，通常使用Visual Mode，选中+c
cw                  → 替换从光标所在位置后到一个单词结尾的字符；首先删除，然后等待插入
{% endhighlight %}

### 其它

如果有多个配置，则可以通过 ```source ~/.vim/default.vim``` 加载。

#### mapleader

可以通过 ```let mapleader=','``` 设置命令的前缀，然后直接使用即可。

#### 启动时间

可以通过 `vim --startuptime tmp.txt` 命令将启动时间信息保存在 `tmp.txt` 文件中，然后通过 `sort -nrk 2` 排序即可。

另外，可以通过 `vim --noplugin` 取消插件加载。

#### 加载插件

通过 `:scriptnames` 命令查看发现没有加载相应的插件。

## 基本配置

vim 含有多种变量，`$HOME` 表示环境变量；`&options` 表示选项；`@a` 表示寄存器；可以通过 `:help function-list` 查看所支持的函数列表；`has()` 用于判断 vim 是否支持这一特性。

如果不知道 vim 配置文件的位置，可以通过 ```:version``` 查询，不同的平台其配置文件名、配置文件顺序会有所区别；还可以通过 ```:help vimrc``` 来查看配置文件在所有平台上的加载位置。

在 Windows 平台下，通常为 `_vimrc` (也即 `$VIM/_vimrc` )，可以通过 `source` 指令包含其它的配置文件，如与 `_vimrc` 同目录下 `source $VIM/myvim.vim` 。

在配置文件中，可以指定变量，而对应的变量可通过 ```:echo $VIM``` 查看。

### Tab 操作

{% highlight text %}
:help tabpage

:tabnew file :tabe file      → 新建或打开某一文件并开启新标签页
:tab split                   → 用标签页打开当前编辑的文件
:tabf filename_re            → 基于正则表达式递归遍历当前工作目录查找名称匹配的文件并为其建立新标签页

:tabs                        → 显示已打开标签页的列表，&lt;指示当前页，+显示修改未保存
:tabc    :tabnew             → 关闭当前标签页等价与:q，新建
:tabn    :tabp               → 移动到下/上一个标签页
:tablast :tabfirst           → 移动到最后/第一个标签页
gt                           → 切换到下一个Tab
gT                           → 反向切换

:tabmove 0                   → 将当前tab移动到第一个位置，位置编号从0开始
:tabdo %s/aaa/bbb/g          → 在每个打开的Tab上执行操作

:tab help tabpage            → 使用Tab而非Windows打开帮助窗口
:help setting-guitablabel    → 自己配置tab标题
{% endhighlight %}

`:tabr` 跳转第一个标签页 gvim 提供了 remote-tab 的功能。

在标签栏中，各标签页的默认名称是对应文件所在路径全称的简写，如 `/usr/share/doc/test.txt` 文件所对应的标签页名默认是 `/u/s/d/test.txt`，这样的标签页名看上去有些诡异。可以在配置文件中添加如下内容，在标签页栏中去除当前所编辑文件的路径信息，只保留文件名。

{% highlight text %}
function ShortTabLabel ()
    let bufnrlist = tabpagebuflist (v:lnum)
    let label = bufname (bufnrlist[tabpagewinnr (v:lnum) -1])
    let filename = fnamemodify (label, ':t')
    return filename
endfunction

set guitablabel=%{ShortTabLabel()}
{% endhighlight %}

### 缓冲区

{% highlight text %}
:help buffer|buffer-list
{% endhighlight %}

缓冲区是一块内存区域，里面存储着正在编辑的文件，如果没有把缓冲区里的文件存盘，那么原始文件不会被更改。

{% highlight text %}
:e <path/to/file>            → 打开一个文件，保存到缓冲区列表，e是edit缩写
:badd <path/to/file>         → 添加到缓冲区列表中
:saveas <path/to/file>       → 另存为 &lt;path/to/file&gt;
:wq                          → 存盘并退出
:x                           → 表示仅在需要时保存
ZZ                           → 不需要输入冒号并回车
:q!                          → 退出不保存
:qa!                         → 强行退出所有的正在编辑的文件，就算别的文件有更改

:[buffers|ls]                → 列出当前编辑中所有的缓冲区状态
:buffer number|filename      → 切换缓冲区，可通过缓冲区的序号或者打开文件的名字选择缓冲区
:sbuffer number|filename     → 水平分割窗口
:ball                        → 为每个缓冲区打开一个窗口，默认按顺序水平切分
:bnext :bprevious            → 调转到下/上一个缓冲区，简写为:bn/:bp，:n 调转到下个文件
:blast :bfirst               → 调转到最后/第一个缓冲区
:bd(elete) number|filename   → 删除缓冲区
:3 bdelete :1,3 bdelete      → 删除或者指定范围的缓冲区
{% endhighlight %}

查看缓冲区时，在这个状态列表中，前面的数字是缓冲区的数字标记，第二个标记就是缓冲区当前的状态，紧接着是与缓冲区所关联的文件名。有如下几种状态：

{% highlight text %}
    - （非活动的缓冲区）
    a （激活缓冲区）
    h （隐藏的缓冲区）
    % （当前的缓冲区）
    # （交换缓冲区）
    = （只读缓冲区）
    + （已经更改的缓冲区）
{% endhighlight %}

在删除缓冲区时，如果缓冲区被改动过，那么该命令将失败，除非使用 `!` 选项。如果使用了带 `!` 选项的 `:bdelete! filename` 命令，那么在缓冲区中的所有改动都会被放弃。

### 折叠

{% highlight text %}
: help folding
: help usr_28

: set foldenable
{% endhighlight %}

当进入插入模式，将会自动打开折叠，也可通过 ```:nnoremap <space> za``` 定义通过空格展开/关闭。

主要有如下几种方式，当进入非 manual 时，所有的折叠将会删除重建，反之则不会。

#### Marker Fold 标记折叠

{% highlight text %}
:set foldmethod=marker
vim: foldmarker={,} foldlevel=0 foldmethod=marker :
{% endhighlight %}

可以精确地定义折叠，折叠的标记通过 ```foldmaker``` 定义，默认以 \{\{\{ 开始，以 \}\}\} 结束，同时在标记后面可以添加数字表示折叠的层级，通过 ```foldlevel``` 设置级别，超过默认值则会折叠。


<!--
#### Manual Fold 手动折叠

{% highlight text %}
:set foldmethod=manual
{% endhighlight %}

在可视化模式下，通过命令 zf ，将折叠选中的文本；通过组合使用移动命令折叠指定行，如，zf70j(折叠光标之后的70行)、5zF(当前行及随后 4 行折叠)；zf7G(当前行至全文第 7 行折叠)、zfa[(折叠括号(如()、[]、{}、><等)包围的区域)。<br><br>

vim 不会自动记忆手工折叠，可以通过命令 :mkview 来保存当前的折叠状态；下次打开文档时，使用命令 :loadview 来载入记忆的折叠信息；使用命令 :help fold-manual 查看关于手工折叠的帮助信息。

#### Indent Fold 缩进折叠

{% highlight text %}
:set foldmethod=indent
{% endhighlight %}

，相同缩进距离的行构成折叠。<br>

对于已经格式化好的语言，如 Python 比较有用。此时所有文本将按照（选项shiftwidth定义的）缩进层次自动折叠，使用zr打开，zm关闭。zm和zr实际控制的是 set foldlevel=N，zr增加，zm减小，0时关闭所有折叠，1时打开一层折叠。可以设置foldignore='#'来忽略对一些行的折叠，仅用于该选项。可以使用以下命令，查看关于缩进折叠的帮助信息：:help fold-indent</li><br><li>

#### Syntax Fold 语法折叠

:set foldmethod=syntax


所有文本将按照语法结构自动折叠。可以使用以下命令，查看关于语法折叠的帮助信息：:help fold-syntax</li><br><li>

Expression Fold, :set foldmethod=expr，表达式折叠<br>
通过 'foldexpr' 给出每行的折叠级别。如<br>
set foldexpr=strlen(substitute(substitute(getline(v:lnum),'\\s','',\"g\"),'[^>].*','',''))<br>
上面是对>开头（如邮件）的折叠，其中 getline(v:lnum)：获得当前行；substitute(...,'\\s','','g')：删除所有的空白字符；substitute(...,'[^>].*','','')：删除>之后的所有内容；strlen(...)：计算长度，也就是>的个数。


zfap 创建一个折叠。当我们在折叠处左右移动或者跳转(如0)时，折叠会自动打开； set foldopen=all 使光标在当前时自动打开，注意此时光标不能移动到关闭的折叠，因此经常临时使用，可以使用 set foldopen& 恢复默认；使用 set foldclose=all 当光标移动之后自动关闭。<br><br>

注意折叠只对本窗口有效，因此可以对同一个缓冲区打开两个窗口，一个带有折叠，另一个没有。<br><br>

当关闭时折叠不会保存，因此应该在关闭之前使用 mkview 此时会保存包括 folder 在内的影响 view 的所有内容；再次打开时使用 loadview 加载。最多可以保存10个，可以使用 mkview 3 或 loadview 2 。
-->

#### 常用命令

{% highlight text %}
zo  打开当前的折叠，O-pen a fold.
zO  打开当前所有的折叠，O-pen all fold.
zr  打开所有折叠，R-educe the folding.
zR  打开所有折叠及其嵌套的折叠

zc  关闭当前打开的折叠，C-lose a fold.
zC  关闭当前所有打开的折叠，C-lose all fold.
zm  关闭所有折叠，folds M-ore.
zM  关闭所有折叠及其嵌套的折叠

za  关闭、打开相互切换

zd  删除当前折叠，对于标记则会自动删除，D-elete a fold
zD  删除当前所有折叠，D-elete all fold

zj  移动至下一个折叠
zk  移动至上一个折叠

zn  禁用折叠
zN  启用折叠
zi  在上述两者之间切换
{% endhighlight %}

<!--
zf  关闭所有折叠，F-old creation.
zE  删除所有折叠
-->


### 高亮显示

{% highlight text %}
:help syntax
:help usr_44.txt
{% endhighlight %}

Vim 会根据 ```$VIMRUNTIME/syntax/language.vim``` 中的配置内容，自动识别关键字、字符串以及其他语法元素，并以不同的颜色显示出来。

{% highlight text %}
:syntax [enable|clear]          → 启用|关闭语法高亮度，只在当前文件中有效
:syntax [off|on]                → 同上，会对所有缓冲区中的文件立刻生效
:set filetype=c                 → 如果VIM无法识别，则设置文件类型
{% endhighlight %}

高亮显示主要通过两步来实现：A) 首先，确定需要格式化的字符；B) 然后，根据配色方案决定如何显示这些字符。

其中经典的是 [solarized](http://ethanschoonover.com/solarized) ，一个配色方案在 github 就有 4K+ 的 Star ，可通过如下方式配置：

{% highlight text %}
syntax enable
if has('gui_running')
    set background=light
else
    set background=dark
endif
colorscheme solarized
{% endhighlight %}

还有一种很受欢迎的配色方案 [Molokai](https://github.com/tomasr/molokai)，它是 Mac 上 TextMate 编辑器的一种经典配色。

<!--
如可使用以下命令，将所有 FIX 和 ENDFIX 关键字显示为特定颜色。

:syntax match cscFix "FIX\|ENDFIX"              // 创建名为 cscFix 的匹配模式
:highlight cscFix ctermfg=cyan guifg=#00FFFF    // 青色显示匹配的文本

注释的开头到结尾，可用于识别代码注释。

:syntax region myComments start=/\/\*/ end=/\*\//
:syntax keyword myToDo FIXME TODO
:syntax region myComments start=/\/\*/ end=/\*\// contains=myToDo

https://github.com/altercation/vim-colors-solarized
-->

### 自动缩进和对齐

可以在配置文件中添加如下内容。

{% highlight text %}
set autoindent         " 设置自动缩进
set smartindent        " 对autoindent进行了一些改进

set shiftwidth=4       " 自动缩进所使用的空白长度
set tabstop=4          " 定义tab所等同的空格长度
set softtabstop=4      " 详见如下的解释

set expandtab          " 将TAB自动替换为空格

set listchars=tab:▸\ ,trail:-,extends:>,precedes:<,eol:¬    " 设置不可见字符的显示方式
set nolist             " 不显示TAB、空格、回车等不可见字符

filetype indent on     " 可以通过如下的设置，根据文件类型自动进行设置
autocmd FileType python setlocal expandtab smarttab shiftwidth=4 softtabstop=4
{% endhighlight %}

#### autoindent/smartindent/cindent

当在 ```insert``` 状态用回车新增一个新行，或者在 ```normal``` 状态用 ```o/O``` 插入一个新行时，会根据配置的不同模式，进行不同的缩进。

在 ```autoindent``` 模式中，会自动将当前行的缩进拷贝到新行，也就是 "自动对齐"，当然了，如果你在新行没有输入任何字符，那么这个缩进将自动删除。

```smartindent``` 对 ```autoindent``` 进行了一些改进，可以识别一些基本的语法。

```cindent``` 会按照 C 语言的语法，自动地调整缩进的长度，可以与上述配置共存。比如，当输入了半条语句然后回车时，缩进会自动增加一个 ```TABSTOP``` 值，当你键入了一个右花括号时，会自动减少一个 ```TABSTOP``` 值。

另外，可以通过 ```indentexpr``` 设置不同的模式，在此不详述，详见 [vim reference manual](http://man.chinaunix.net/newsoft/vi/doc/indent.html) 。

#### 缩进宽度设置

如上配置的第二部分，就是所使用的缩进模式，如下主要介绍下 ```softtabstop``` 的含义。

当 ```shiftwidth/tabstop``` 不同时，会导致程序对齐很难看，这时可以使用 ```softtabstop```；此时，当按下 ```TAB``` 键，插入的是空格和 ```TAB``` 制表符的混合，具体如何混合取决于你设定的 ```softtabstop``` 。

举个例子，如果设定 ```softtabstop=4``` ，那么按下 ```tab``` 键，插入的就是正常的一个制表符；如果设定 ```softtabstop=8``` ，那么插入的就是两个制表符；如果 ```softtabstop=10``` ，那么插入的就是一个制表符加上 2 个空格；如果 ```softtabstop=2``` 呢？

开始插入的就是 2 个空格，此时一旦你再按下一次 tab ，这次的 2 个空格就会和上次的 2 个空格组合起来变成一个制表符。换句话说，```softtabstop``` 是 “逢 4 空格进 1 制表符” ，前提是 ```tabstop=4``` 。

#### TAB 和空格替换

```:set expandtab/noexpandtab``` 使用空格替换TAB/不进行替换。对于已保存的文件，可以使用下面的方法进行空格和TAB的替换。

{% highlight text %}
----- TAB替换为空格
:set ts=4
:set expandtab
:%retab!

----- 空格替换为TAB
:set ts=4
:set noexpandtab
:%retab!
{% endhighlight %}

加 ```!``` 用于处理非空白字符之后的 ```TAB```，即所有的 ```TAB```，若不加 ```!```，则只处理行首的 ```TAB``` 。

假设配置文件中使用了 ```set expandtab```，如果想要输入 ```TAB```，Linux 下使用 ```Ctrl-V + TAB```，Win 下使用 ```Ctrl-Q + TAB``` 。

#### 设置对齐方式

通过如下方式设置 C 语言的缩进方式，具体配置可查看 [Vim documentation: indent](http://vimdoc.sourceforge.net/htmldoc/indent.html) 。

{% highlight text %}
set cinoptions={0,1s,t0,n-2,p2s,(03s,=.5s,>;1s,=1s,:1s
{% endhighlight %}

#### 常用命令

{% highlight text %}
----- 执行缩进，前面可以加数字进行多节缩进
>>               # Normal模式下，增加当前行的缩进
<<               # Normal模式下，减少当前行的缩进
CTRL+SHIFT+T     # Insert模式下，增加当前行缩进
CTRL+SHIFT+D     # Insert模式下，减小当前行缩进
=                # Visual模式下，对选中的部分进行自动缩进

: set list       # 查看不可见字符，包括TAB、空格、回车等
{% endhighlight %}

<!-- ]p可以实现p的粘贴功能，并自动缩进。 -->

### 文件类型检测

Vim 会根据不同的文件类型，分别设置高亮、缩进等功能。可以通过 ```:filetype``` 查看 Vim 的文件类型检测功能是否已打开，通常有三个选项 ```detection:ON plugin:ON indent:ON``` 。

* detection，是否自动检测文件类型。
* plugin，是否自动加载该类型相关的插件。
* indent，根据文件类型定义缩进方式，通常在安装目录的 indent 目录下定义缩进相关脚本。

当配置文件中添加了 ```filetype on``` 命令时，实际上会调用 ```$VIMRUNTIME/filetype.vim``` 脚本检测文件的类型。

{% highlight text %}
:filetype [detection|plugin|indent] on  → 设置开关
:filetype plugin indent on              → 同上，也可以在一行中设置
:filetype [off|on]                      → 关闭/打开
:set filetype                           → 查看当前文件类型
:set filetype=python                    → 设置类型
{% endhighlight %}

在设置文件类型时，会同时触发 `FileType` 自动命令；从而，可以用下面的命令实现，根据不同文件类型设置不同的配置项。

{% highlight text %}
autocmd FileType c,cpp set shiftwidth=4 | set expandtab
{% endhighlight %}

## 高级进阶

### 区域选择

{% highlight text %}
:help object-motions
:help text-objects
{% endhighlight %}

`Text Objects Commands` 或许这个是最强大的命令了。

{% highlight text %}
<number><command><text object or motion>
  <number>     操作的重复次数；
  <command>    命令，如chang(c), delete(d), yank(y)，默认是选择，如果不指定则只移动；
  <text object or motion>
               可以是指定文本，例如word、sentence、paragraph；或者是动作，如前进一行、行尾等。

Old text                  Command     New text
the t*ext object          daw         the object     删除Word，同时删除空格
the t*ext object          diw         the  object    删除Word，不删除空格
the t*ext object          dw          the tobject    删除Word，从开始位置删除，含空格
<h2>Sa*mple</h2>          dit         <h2></h2>      删除tag之间的内容
<div *id="cont"></div>    di>         <></div>       删除单个tag的内容
{% endhighlight %}

除了上面的 `Word` 外，还可以通过如下方式指定范围。

{% highlight text %}
aw(A Word)        iw(Inner Word)        前者包含了区分子的空格，后者不包含；
as(A Sentence)    is(Inner Sentence)    通过.分割，作为一个句子；
as(A Block)       is(Inner Block)       也就是括号
ap(A Paragraph)   ip(Inner Paragraph)   通过空白行分割；
at(A Tag Block)   it(Inner Tag Block)   使用tag，例如HTML中的tag
a>(A Single Tag)  i>(Inner Single Tag)  在单个的tag内
{% endhighlight %}

除了上述的定义外，还可以通过 `a)` `i)` `a]` `i]` `a}` `i}` `a'` `i'` `a"` `i"` 还有反问号 ( \` ) ，这类操作与上述的 `Word` 操作相同。另外，`%` 可以在括号之间切换，其中 `c%` 和 `ca)` 等价。

需要注意 `Motion Commands` 和 `Text Objects Commands` 的区别，前者如 `cw` 是从当前的位置操作，而像 `ciw` 则处理的是光标所在的整个 `Word` 。

<!--
在 visual 模式下，这些命令很强大，其命令格式为 ```<action>a<object>``` 和 ```<action>i<object>``` ，其中 a 含有空格，i 不含。在 Normal 模式下按 'v' 进入自由选择，'V' 行选择，Ctrl-v 块选择。

{% highlight text %}
Old text                  Command     New text ~
 "Hello *world!"           ds"         Hello world!
 [123+4*56]/2              cs])        (123+456)/2
 "Look ma, I'm *HTML!"     cs"<q>      <q>Look ma, I'm HTML!</q>
 if *x>3 {                 ysW(        if ( x>3 ) {
 my $str = *whee!;         vlllls'     my $str = 'whee!';
 <div>Yo!*</div>           dst         Yo!
 <div>Yo!*</div>           cst<p>      <p>Yo!</p>
{% endhighlight %}
-->

常用操作。


{% highlight text %}
ci[ ci( ci< ci{      删除一对 [], (), <>, 或{} 中的所有字符并进入插入模式
ci" ci' ci`          删除一对引号字符 " ' 或 ` 中所有字符并进入插入模式
cit                  删除一对 HTML/XML 的标签内部的所有字符并进入插入模式

ci                   如 ci(，或者 ci)，将会修改 () 之间的文本；
di                   删除配对符号之间文本；
yi                   复制；
ca                   同ci，但修改内容包括配对符号本身；
da                   同di，但剪切内容包括配对符号本身；
ya                   同yi，但复制内容包括配对符号本身。
{% endhighlight %}

另外，`dib` 等同于 `di(`，`diB` 等同于 `di{` 。

### 宏录制

通过 `q` 进入宏录制模式，操作为 A) 录制，`qa do_something q`；B) 使用 `@a`, `@@`。`qa` 会把操作记录在寄存器 `a`；`@a` 会重做被录制的宏，就好象自己在输入一样；`@@` 是一个快捷键，用来执行最近被执行的宏。

如，在一个只有一行且这一行只有 `"1"` 的文本中，键入如下命令:

{% highlight text %}
qaYp<C-a>     → qa开始录制；Yp复制行；<C-a>增加1；q停止录制
@a            → 在1下面写下2
@@            → 在2正面写下3
100@@         → 会创建新的100行，并把数据增加到103
{% endhighlight %}

### 块操作

在 Normal 模式下，通过 `Ctrl-v` 执行操作，如在指定的行首添加 `"--"` 字符串，其执行命令顺序为 `0 <C-v> <C-d> I-- [ESC]`，解释如下：

{% highlight text %}
0             → 到行头
<C-v>         → 开始块操作
<C-d>         → 向下移动，也可以使用hjkl来移动光标
I-- [ESC]     → I是插入，插入"--"，按ESC键来为每一行生效
{% endhighlight %}

行后添加字符的执行顺序如下。

{% highlight text %}
<C-v>       → 视图模式
<C-d>       → 选中相关的行，也可以使用j，或是/pattern或是%等
$           → 到行最后
A           → 输入字符串，按ESC
{% endhighlight %}

在 Windows 下的 vim ，需要使用 `<C-q>` 而不是 `<C-v>`，`<C-v>` 是拷贝剪贴板。

### 可视化选择

常见的操作包括了 `v`、`V`、`<C-v>` 等视图模式下，在 Windows 下应该是 `<C-q>`，选好之后，可以做下面的事。

{% highlight text %}
J           → 把所有的行连接起来，变成一行
< 或 >      → 左右缩进
=           → 自动缩进
{% endhighlight %}

### 其它

#### 重复执行命令

{% highlight text %}
.                     → 重复上一次的命令
<NUM><command>        → 重复某个命令N次
2dd                   → 删除2行
100idesu [ESC]        → 会写下100个"desu "
{% endhighlight %}

#### 模式行

可以通过 `:help modeline` 查看帮助，在文件首、尾的若干行 (modelines默认5行) 添加的配置内容，可以用来指明这个文件的类型，以及一些其它的相关配置项。

{% highlight text %}
# vim: filetype=python
/* vim: filetype=java */
# vim: foldmarker={,} foldlevel=0 foldmethod=marker :
{% endhighlight %}

#### 其它

{% highlight text %}
:set cc=80         → 高亮显示第80列
: shell            → 切换到shell执行命令，退出后返回到VIM
: messages         → 出现错误时可以通过该命令查看错误信息
{% endhighlight %}

## 杂项

简单记录下杂七杂八。

### 常用技巧

#### 大小些操作

按 `v` 进入 `visual` 模式，`ap` paragraph 选择整段，`aw` word 选择一个词，通过 `~` 大小写取反；通过 `gU` 更改为大些，`gu` 改为小写。

#### 取消备份

Windows 的配置文件在安装目录下，为 `_vimrc` 。可以添加 `set nobackup | backup` ，如果只需要当前文件不保存可以 `:set nobackup` ，或者保存在固定目录下 `set backupdir=dir` 。

默认会将文件保存为 `filename~` ，也可以通过 `:set backupext=.bak` 指定后缀名，当然这样只能保存最后的一个版本。

另外，在 VIM 中，还提供了 patchmode 把第一次的原始文件备份下来，不会改动，可以直接通过 `:set patchmode=.orig` 命令开启。

#### 没有使用 root

如果由于权限不足导致无法保存，可以通过如下方式保存 `:w !sudo tee %` 。

#### 二进制文件

通常使用外部程序 `xxd` 达到这样的效果，通过 `vim -b file` 命令以二进制格式打开, `:%xxd` 将二进制转换为文本，然后通过 `:%xxd -r` 重新保存为二进制，详细参数可以参考 `man xxd` 。

另外，可以通过 `:set display=uhex` 以 uhex 模式显示，用来显示一些无法显示的字符，例如控制字符之类。

#### 外部修改

可以通过 `:e` 或 `:e!` 加载，后者会放弃之前的修改。

#### 输入特殊字符

通过 `:digraphs` 可以查看特殊字符，在插入模式下或输入命令时可以通过 `Ctrl-k XX` ，其中 `XX` 为 digraphs 前面指定的两个字符；也可以通过 `Ctrl-v NUM` 对应的数字输入特殊字符。

#### 替换^M字符

在 Linux 下查看一些 Windows 下创建的文本文件时，经常会发现在行尾有一些 `^M` 字符，有几种方法可以处理。

##### 1. dos2unix

使用 `dos2unix` 命令，发布版本一般都带这个工具，通过 `dos2unix myfile.txt` 修改即可。

##### 2. vim

使用 vim 的替换功能，当启动 vi 后，进入命令模式，输入以下命令:

{% highlight text %}
:%s/^M$//g                      # 去掉行尾的^M
:%s/^M//g                       # 去掉所有的^M
:%s/^M/[ctrl-v]+[enter]/g       # 将^M替换成回车
:%s/^M/\r/g                     # 将^M替换成回车
{% endhighlight %}

##### 3. sed

使用 sed 命令，和 vi 的用法相似 `sed -e 's/^M/\n/g' myfile.txt` 。

注意：这里的 `^M` 要使用 `CTRL-V CTRL-M` 生成，而不是直接键入 `^M` 。

#### 光标闪烁

可以通过 `set gcr=a:block-blinkon0` 设置，其中 `gcr` 是 `guicursor` 的简写，`a` 表示所有模式，冒号后面是对应模式下的行为参数，每个参数用 `-` 分隔，`block` 说明用块状光标，`blinkon` 表示亮起光标时长，时长为零表示禁用闪烁。

<!--
也可以是 blinkoff0 或者 blinkwait0。具体参考 :help gcr。
Applications->System Tools->Settings->Keyboard->[Cursor Blinking]
-->

### 异常处理

如果发现某个插件无法使用，可以通过 `:scriptnames` 命令查看当前已经加载的脚本程序。

### 键盘图

![vim keyboard 1]({{ site.url }}/images/misc/vim-keyboard-1.gif "vim keyboard 1"){: .pull-center width="90%" }

也可以参考英文版的 [vim Keyboard en]({{ site.url }}/images/misc/vim-keyboard-en.gif) ，或者 [参考下图](http://michael.peopleofhonoronly.com/vim/) 。

![vim keyboard 2]({{ site.url }}/images/misc/vim-keyboard-2.png "vim keyboard 2"){: .pull-center width="90%" }

另外，关于按键列表可以参考 [vim keys]({{ site.url }}/reference/linux/vim-keys.pdf) 。

<!-- http://jrmiii.com/attachments/Vim.pdf -->

## 参考

如果要编写 VIM 脚本，可以参考 Trinity 这个简单的插件，常见的语法可以参考 Vim 脚本语言官方文档 [Write a Vim script](http://vimdoc.sourceforge.net/htmldoc/usr_41.html) 。

可以查看中文文档 [VIM 中文帮助文档](http://vimcdoc.sourceforge.net/doc/) ；另外一个很经典的文档 [Learn Vim Progressively](http://yannesposito.com/Scratch/en/blog/Learn-Vim-Progressively/)，以及 [中文版](http://coolshell.cn/articles/5426.html)； 还有就是一个 PDF 教程，也就是 [Practical Vim.pdf](/reference/linux/Practical_Vim.pdf) 。

一个 vim 的游戏，用于熟悉各种操作，可以参考 [vim adventures](http://vim-adventures.com/) 。

关于如何针对新的语言添加 tags 可以参考 [How to Add Support for a New Language to Exuberant Ctags](http://ctags.sourceforge.net/EXTENDING.html) 。

<!--
https://raw.githubusercontent.com/nvie/vimrc/master/vimrc
https://github.com/amix/vimrc
https://github.com/vgod/vimrc
https://github.com/humiaozuzu/dot-vimrc
https://github.com/xolox/vim-easytags
https://github.com/ruchee/vimrc/blob/master/_vimrc
http://www.cnblogs.com/ma6174/archive/2011/12/10/2283393.html
https://www.zhihu.com/question/26713049

vim 配置文件，将 F2 等功能添加到 default 文件中。

http://www.yeolar.com/note/2010/01/27/vim/
https://github.com/vim-airline/vim-airline/wiki/FAQ

<a href="http://python.42qu.com/11180003">使用Vim打造现代化的Python IDE</a>。<br><br>
<a href="http://python.42qu.com/11165602">如何用python写vim插件</a>

http://ju.outofmemory.cn/entry/79671                      经典插件的介绍
http://blog.chinaunix.net/uid-24118190-id-4077308.html    VIM终极配置
/reference/linux/{default.vim, plugin.vim}                本地保存的版本
-->

{% highlight text %}
{% endhighlight %}
