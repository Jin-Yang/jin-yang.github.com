---
title: VIM 插件使用
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

常用的插件列表可以参考。

{% highlight text %}
插件管理
  1. Vundle        +++ the plug-in manager for Vim
  2. Vim-Plug      --- Minimalist Vim Plugin Manager

自动补全
  1. YouCompleteMe +++ visual assist for vim
  2. UltiSnips     +++ ultimate snippets
  3. Zen Coding    --- hi-speed coding for html/css

导航与搜索
  1. CtrlP         +++ fast file finder
  2. NERDTree      +++ file navigation
  3. Tagbar        +++ tag generation and navigation
  4. Taglist       --- source code browser

其它
  1. Tabularize    +++ align everything
  2. vim-airline   +++ lean & mean status/tabline for vim that's light as air 

  2. Easymotion    - jump anywhere
  3. NERDCommenter - comment++
  4. Surround      - managing all the "'[{}]'" etc
  5. Gundo         - time machine
  6. Sessionman    - session manager
  7. Powerline     --- ultimate statusline utility
{% endhighlight %}

其中 taglist 和 tagbar 类似，不过其关注点有所区别，后者比较适合面向对象。

<!--
Ack 全文搜索插件，可以在当前打开的项目中进行源码的全文搜索，并可以在搜索结果中方便的切换和打开源码文件，十分方便。
NERDTreeCommenter 方便的用来注释代码的插件
AutoPairs 自动补全括号的插件，包括小括号，中括号，以及花括号，可以提升编码效率
Surround 快速给词加环绕符号,例如单引号/双引号/括号/成对标签等的插件
EasyMotion 在当前文件中快速移动光标到指定查找位置的插件，十分方便和高效

语法
  1. Syntastic   - integrated syntax checking
  2. Python-mode - Python in VIM

-->

通常可以使用 vundle 管理所有的插件，通常插件为了防止多次加载，会在开始的时候检测是否已经加载。

一些经典的配置可以参考 [vim.spf13.com](http://vim.spf13.com/)，该 vim 配置，在 [github](https://github.com/spf13/spf13-vim) 中有 1W+ 的 Star，可以通过如下方式进行配置：

{% highlight text %}
----- 直接下载安装，实际上就是github中的bootstrap.sh脚本
$ curl https://j.mp/spf13-vim3 -L -o - | sh
{% endhighlight %}

## 插件管理

比较经典的是 Vundle ，也可以使用 Vim-Plug 。

### Vundle

<!-- 可以使用 vim plug -->

vim 缺少默认的插件管理器，所有插件的文件都散布在 ```~/.vim``` 下的几个文件夹中，无论是配置、更新、删除，都需要手动配置，很容易出错。

vundle 把 git 操作整合进去，用户需要做的只是去 Github 上找到自己想要的插件的名字，安装，更新和卸载都可有 vundle 来完成了，可以查看 [github](https://github.com/gmarik/vundle)，可以通过如下方式安装。

{% highlight text %}
$ git clone https://github.com/gmarik/vundle.git ~/.vim/bundle/vundle
{% endhighlight %}

vim-scripts 在配置时，可以直接打仓库名，相关的插件可以从 [官网](http://vim-scripts.org/index.html) 查看，或查看 [Github](https://github.com/vim-scripts)，此时可以直接使用 ```Bundle 'L9'``` 。在配置文件中，主要是配置安装哪些插件，Vundle 支持如下的格式：

{% highlight text %}
Bundle 'gmarik/vundle'          " 使用Vundle来管理Vundle，这个必须要有
Bundle 'tpope/vim-fugitive'     " Github上其他用户的仓库，非vim-scripts账户里的仓库
Bundle 'file:///path/to/plugin' " 使用自己的插件
Bundle 'git://vim-latex.git.sourceforge.net/gitroot/vim-latex/vim-latex'
{% endhighlight %}

自己写的插件也可以通过如上方式管理，如自己写了一个 test 插件，并放到了 ```.vim/myplugin``` 目录中，那么可以在 vim 的配置文件中加入下面的命令 ```set rtp+=~/.vim/myplugin/``` 。

Vundle 常用的命令如下。

{% highlight text %}
BundleList             列出所有已配置的插件
BundleInstall(!)       下载(更新)插件
BundleUpdate           更新插件
BundleClean            清除不再使用的插件
BundleSearch(!) foo    查找(先刷新cache)foo
{% endhighlight %}

对于 Vundle 插件，如果使用 `call vundle#begin()` 时发现很多插件无法使用，可以使用 `call vundle#rc()` ，暂时不确认为什么。

### Vim-Plug

相比来说，所有的插件更新和安装都是并行的，比 Vundle 的效率要高很多，而且可以按需加载。

## 自动补全

比较经典的是 YCM 。

<!--
https://github.com/Shougo/deoplete.nvim
-->

### YouCompleteMe

当通过 `Plugin 'Valloric/YouCompleteMe'` 安装 YCM 后，经常会出现 `no module named future` 的报错，可以通过如下方式进行安装。

{% highlight text %}
----- 可以通过Vundle安装，或使用如下方式下载，后者用于下载相关的依赖
$ git clone --recursive https://github.com/Valloric/YouCompleteMe.git ~/.vim/bundle/YouCompleteMe
$ git submodule update --init --recursive

----- 通过该脚本安装，支持C、Go，如果要支持所有语言通过--all安装
$ ./install.py --clang-completer --gocode-completer
{% endhighlight %}

在安装时，可以通过 `--system-clang` 指定使用系统的 Clang 。 <!-- --system-libclang -->

另外，UltiSnips 与 YCM 有按键冲突，很多都建议将 UltiSnips 的自动填充快捷键更换，不过仍无效，可以通过如下方式修改，此时 `<C-N>` 和 `<C-P>` 仍然有效。

{% highlight text %}
let g:ycm_key_list_select_completion=[]
let g:ycm_key_list_previous_completion=[]
{% endhighlight %}

常用命令。

{% highlight text %}
:YcmDiags          通过location-list显示诊断信息
{% endhighlight %}

<!--
在 Centos7 中可以通过 `yum install clang` 安装，不过依赖 epel 镜像库
使用 vundle 下载源码，在 vimrc 文件中加入 Bundle 'Valloric/YouCompleteMe'，并通过如下命令安装。
-->

<!--
    " 自动补全配置
    autocmd InsertLeave * if pumvisible() == 0|pclose|endif "离开插入模式后自动关闭预览窗口
    inoremap <expr> <CR>       pumvisible() ? "\<C-y>" : "\<CR>"    "回车即选中当前项
    "上下左右键的行为 会显示其他信息
    inoremap <expr> <Down>     pumvisible() ? "\<C-n>" : "\<Down>"
    inoremap <expr> <Up>       pumvisible() ? "\<C-p>" : "\<Up>"
    inoremap <expr> <PageDown> pumvisible() ? "\<PageDown>\<C-p>\<C-n>" : "\<PageDown>"
    inoremap <expr> <PageUp>   pumvisible() ? "\<PageUp>\<C-p>\<C-n>" : "\<PageUp>"

    "youcompleteme  默认tab  s-tab 和自动补全冲突
    "let g:ycm_key_list_select_completion=['<c-n>']
    let g:ycm_key_list_select_completion = ['<Down>']
    "let g:ycm_key_list_previous_completion=['<c-p>']
    let g:ycm_key_list_previous_completion = ['<Up>']
    let g:ycm_confirm_extra_conf=0 "关闭加载.ycm_extra_conf.py提示

    let g:ycm_collect_identifiers_from_tags_files=1 " 开启 YCM 基于标签引擎
    let g:ycm_min_num_of_chars_for_completion=2 " 从第2个键入字符就开始罗列匹配项
    let g:ycm_cache_omnifunc=0  " 禁止缓存匹配项,每次都重新生成匹配项
    let g:ycm_seed_identifiers_with_syntax=1    " 语法关键字补全
    "nnoremap <leader>lo :lopen<CR> "open locationlist
    "nnoremap <leader>lc :lclose<CR>    "close locationlist
    inoremap <leader><leader> <C-x><C-o>
    let g:ycm_collect_identifiers_from_comments_and_strings = 0

    nnoremap <leader>jd :YcmCompleter GoToDefinitionElseDeclaration<CR> " 跳转到定义处-->

#### 配置文件

YCM 中需要在 flags 中添加编译时的参数定义，例如 `-Wall`、`-Wextra` 等，然后通过 `-isystem` 指定系统的头文件，通过 `-I` 指定自定义的头文件。

注意，在使用相对路径时是 YCM 的安装路径。

#### YCM错误日志及调试信息

在 vim 配置文件中加上下面的选项，打开调试信息。

{% highlight text %}
let g:ycm_server_keep_logfiles = 1
let g:ycm_server_log_level = 'debug'
{% endhighlight %}

几个常用命令:

{% highlight text %}
:YcmToggleLogs   查看到错误日志及调试信息，输出到哪些文件中
:YcmDebugInfo    可以查看ycm相关的信息,包括编译标志,版本等等
:YcmDiags        查看当前文件中的错误信息
{% endhighlight %}

然后调试时就可以通过 print 打印，然后输出到 stdout 指定的文件中。

#### 常见问题

对于 `Your C++ compiler does NOT support C++11` 错误，是由于要求最低 `gcc 4.9` 版本，该功能会在 `third_party/ycmd/cpp/CMakeLists.txt` 文件中进行检查，也就是 `set( CPP11_AVAILABLE false )` 定义。

<!--
配置文件参考示例
https://github.com/rasendubi/dotfiles/blob/master/.vim/.ycm_extra_conf.py
https://github.com/robturtle/newycm_extra_conf.py/blob/master/ycm.cpp.py
-->

### UltiSnips

一个和牛摆的模版，在写代码时经常需要在文件开头加一个版权声明之类的注释，又或者在头文件中要需要 `#ifndef... #def... #endif` 这样的宏，亦或写一个 `for` `switch` 等很固定的代码片段。

该工具和 YouCompleteMe 以及 neocomplete 都很好的整合在一起了，不过需要编写模版，很多模版可以参考 [honza/vim-snippets](https://github.com/honza/vim-snippets)。

<!-- http://vimcasts.org/episodes/meet-ultisnips/ -->

## 导航与搜索

包括了文件、代码的导航。

### CtrlP

![vim CtrlP]({{ site.url }}/images/misc/vim-ctrl-p-buttons.png "vim CtrlP"){: .pull-center width="50%" }

一个强大的搜索插件，可以模糊查询定位，包括了工程下的所有文件、打开的 Buffer、Buffer 内的 tag、最近访问的文件等，极大了方便了大规模工程代码的浏览。

<!-- 据说还有一个 CopyCat ，暂时没有找到怎么使用 -->

### NERDTree

[NERDTree](https://github.com/scrooloose/nerdtree) 的作用就是列出当前路径的目录树，类似与一般 IDE，可以方便的浏览项目的总体的目录结构和创建删除重命名文件或文件名。

另外还有一个 NERDTreeTabs 插件，可以以 tab 形式显示窗口，用于方便浏览目录以及文件。

{% highlight text %}
map <F2> :NERDTreeToggle<CR>          " 使用F2键快速调出和隐藏它
autocmd vimenter * NERDTree           " 打开vim时自动打开NERDTree
{% endhighlight %}

如下是一些常用的命令：

{% highlight text %}
t   以Tab形式打开文件
i   分割成上下两个窗口显示文件
s   分割成左右两个窗口显示文件

C   将该目录设置为根目录
P   把有表移动到该目录的根目录
K   把有表移动到该目录的第一个
J   把有表移动到该目录的最后一个

p   切换到目录的下一层
u   切换到目录的上一层

m   通过NERDTree的选择菜单进行操作
I   是否显示隐藏文件切换
q   关闭NERDTree窗口
?   打开/关闭帮助指令
{% endhighlight %}

另外，比较好用的是提供的标签操作。

<!--
书签操作，以下命令只在在Nerdtree的buffer中有效。

:Bookmark name 将选中结点添加到书签列表中，并命名为name（书签名不可包含空格），如与现有书签重名，则覆盖现有书签。</li><li>
:OpenBookmark name  打开指定的文件。</li><li>
:ClearBookmarks name 清除指定书签；如未指定参数，则清除所有书签。</li><li>
:ClearAllBookmarks 清除所有书签。</li><li>
:ReadBookmarks 重新读入'NERDTreeBookmarksFile'中的所有书签。</li></ul>
:BookmarkToRoot 以指定目录书签或文件书签的父目录作为根结点显示NerdTree
:RevealBookmark
如果指定书签已经存在于当前目录树下，打开它的上层结点并选中该书签

下面总结一些命令，原文地址<a href="http://yang3wei.github.io/blog/2013/01/29/nerdtree-kuai-jie-jian-ji-lu/">http://yang3wei.github.io/blog/2013/01/29/nerdtree-kuai-jie-jian-ji-lu/</a>
<pre style="font-size:0.8em; face:arial;">
o       在已有窗口中打开文件、目录或书签，并跳到该窗口
go      在已有窗口中打开文件、目录或书签，但不跳到该窗口
i       split 一个新窗口打开选中文件，并跳到该窗口
gi      split 一个新窗口打开选中文件，但不跳到该窗口
s       vsplit 一个新窗口打开选中文件，并跳到该窗口
gs      vsplit 一个新 窗口打开选中文件，但不跳到该窗口
O       递归打开选中 结点下的所有目录
x       合拢选中结点的父目录
X       递归 合拢选中结点下的所有目录
t       在新 Tab 中打开选中文件/书签，并跳到新 Tab
T       在新 Tab 中打开选中文件/书签，但不跳到新 Tab
</pre>
Nerdtree当打开多个标签时，会导致新打开的标签没有nerdtree窗口，此时可以使用nerdtree-tabs，将上述定义的快捷键设置为NERDTreeTabsToggle即可。
!       执行当前文件
e       Edit the current dif

双击    相当于 NERDTree-o
中键    对文件相当于 NERDTree-i，对目录相当于 NERDTree-e

D       删除当前书签

P       跳到根结点
p       跳到父结点
K       跳到当前目录下同级的第一个结点
J       跳到当前目录下同级的最后一个结点
k       跳到当前目录下同级的前一个结点
j       跳到当前目录下同级的后一个结点

C       将选中目录或选中文件的父目录设为根结点
u       将当前根结点的父目录设为根目录，并变成合拢原根结点
U       将当前根结点的父目录设为根目录，但保持展开原根结点
r       递归刷新选中目录
R       递归刷新根结点
m       显示文件系统菜单 #！！！然后根据提示进行文件的操作如新建，重命名等
cd      将 CWD 设为选中目录

I       切换是否显示隐藏文件
f       切换是否使用文件过滤器
F       切换是否显示文件
B       切换是否显示书签

:tabnew [++opt选项] ［＋cmd］ 文件      建立对指定文件新的tab
:tabc   关闭当前的 tab
:tabo   关闭所有其他的 tab
:tabs   查看所有打开的 tab
:tabp   前一个 tab
:tabn   后一个 tab

标准模式下：
gT      前一个 tab
gt      后一个 tab

MacVim 还可以借助快捷键来完成 tab 的关闭、切换
cmd+w   关闭当前的 tab
cmd+{   前一个 tab
cmd+}   后一个 tab
-->

### Tagbar

Tagbar 依赖于 ctags 命令，对于 CentOS 来说可以直接通过 `yum install ctags` 来安装。源码可以从 [github tagbar](http://majutsushi.github.io/tagbar/) 中查看，帮助文档可以查看 [doc/tagbar.txt](https://github.com/majutsushi/tagbar/blob/master/doc/tagbar.txt) 。

#### 常见问题

##### E257: cstag: tag not found

也就是说无法找到 tags 文件，此时需要通过如下参数进行设置，指定 tags 的查找路径。

{% highlight text %}
----- 指定多个路径
set tags+=tags,../tags,../../tags
set tags+=$HOME/.vim/systags

----- 或者允许自动切换路径，并使用当前目录下的文件
set tags=tags
set autochdir
{% endhighlight %}


## 杂项

### Tabular

一个不错的对齐用的插件，可以从 [tabular](http://www.vim.org/scripts/script.php?script_id=3464) 下载，然后通过如下的方式安装，详细的资料可以参考 [aligning text with tabular vim](http://vimcasts.org/episodes/aligning-text-with-tabular-vim/) 。

{% highlight text %}
$ git clone https://github.com/godlygeek/tabular.git
$ cd tabular/
$ mv after autoload doc plugin ~/.vim/
{% endhighlight %}

首先，通过 V v Ctrl-v 选取需要对其的内容，然后直接输入如下的命令即可。注意，在 Visual 模式中输入 ```:``` 后，会显示 ```:'<,'>``` 直接输入命令即可。

{% highlight text %}
:Tabularize /=           使用=进行分割
:Tabularize /:\zs        ':'符号不动，只对其':'后面的字符
{% endhighlight %}

其中 Tabularize 可以简写为 Tab ；另外，tabular 可以自动识别，尤其是含有 {}()[] 的，因此可以不选择字符，也就是在某行中直接输入 ```:Tabularize /|``` 即可。

可以通过如下的示例进行测试。

{% highlight text %}
|start|eat|left|
|12|5|7|
|20|5|15|

var video = {
    metadata: {
        title: "Aligning assignments"
        h264Src: "/media/alignment.mov",
        oggSrc: "/media/alignment.ogv"
        posterSrc: "/media/alignment.png"
        duration: 320,
    }
}
{% endhighlight %}

对于第二部分，执行 ```vi}``` 选择区域，执行 ```:Tab /:``` ，执行 ```:Tab /:\zs``` 则会使 : 不变。

{% highlight text %}
$ vim ~/.vimrc                              # 在最后添加如下的内容
let mapleader=','
if exists(":Tabularize")
  nmap <Leader>a= :Tabularize /=<CR>
  vmap <Leader>a= :Tabularize /=<CR>
  nmap <Leader>a: :Tabularize /:\zs<CR>
  vmap <Leader>a: :Tabularize /:\zs<CR>
endif

inoremap <silent> <Bar>   <Bar><Esc>:call <SID>align()<CR>a
function! s:align()
  let p = '^\s*|\s.*\s|\s*$'
  if exists(':Tabularize')&&getline('.') =~# '^\s*|'&&(getline(line('.')-1) =~# p || getline(line('.')+1) =~# p)
    let column = strlen(substitute(getline('.')[0:col('.')],'[^|]','','g'))
    let position = strlen(matchstr(getline('.')[0:col('.')],'.*|\s*\zs.*'))
    Tabularize/|/l1
    normal! 0
    call search(repeat('[^|]*|',column).'\s\{-\}'.repeat('.',position),'ce',line('.'))
  endif
endfunction
{% endhighlight %}

上述前一部分表示绑定的快捷键<!--，if语句使用时出错?????????????-->；后一部分表示在输入时会自动检测，并设置格式。

### Vim-Airline

主题在目录 `autoload/Powerline/Themes` 下，配色在 `autoload/Powerline/Colorschemes`，`vim-airline` 与 `vim-powerline` 类似但是更小，更轻便。

推荐使用后者，可以参考 [vim-airline](https://github.com/vim-airline/vim-airline)，其中主题 themes 已经单独拆出来了。







































### Syntastic

一个语法检查工具，支持多种语言，提供了基本的补全功能、自动提示错误的功能外，还提供了 tags 的功能；采用 C/S 模式，当 vim 关闭时，ycmd 会自动关闭。

不过对于不同的语言需要安装相应的插件，详细内容可以查看 [doc/syntastic-checkers.txt](https://raw.githubusercontent.com/vim-syntastic/syntastic/master/doc/syntastic-checkers.txt)，安装方法可以参考 README.md 文件。

![vim Syntastic]({{ site.url }}/images/misc/vim-syntastic-screenshot.png "vim Syntastic"){: .pull-center width='90%' }

能够实时的进行语法和编码风格的检查，还集成了静态检查工具，支持近百种编程语言，像是一个集大成的实时编译器，出现错误之后，可以非常方便的跳转到出错处。

另外，是一个 [Asynchronous Lint Engine](https://github.com/w0rp/ale) 一个异步的检查引擎。

### NERDCommenter

用于快速，批量注释与反注释，适用于任何你能想到的语言，会根据不同的语言选择不同的注释方式，方便快捷。


### Surround

一个专门用来处理这种配对符号的插件，它可以非常高效快速的修改、删除及增加一个配对符号，对于前端工程师非常有用。

通常可以和 [repeat.vim](http://www.vim.org/scripts/script.php?script_id=2136) 配合使用，如下是常见的操作示例。

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

如下是一个命令列表。

{% highlight text %}
 Normal mode
 ds  - delete a surrounding
 cs  - change a surrounding
 ys  - add a surrounding
 yS  - add a surrounding and place the surrounded text on a new line + indent it
 yss - add a surrounding to the whole line
 ySs - add a surrounding to the whole line, place it on a new line + indent it
 ySS - same as ySs

 Visual mode
 s   - in visual mode, add a surrounding
 S   - in visual mode, add a surrounding but place text on new line + indent it

 Insert mode
 <CTRL-s> - in insert mode, add a surrounding
 <CTRL-s><CTRL-s> - in insert mode, add a new line + surrounding + indent
 <CTRL-g>s - same as <CTRL-s>
 <CTRL-g>S - same as <CTRL-s><CTRL-s>
{% endhighlight %}

### tags

tags 记录了关于一个标识符在哪里被定义的信息，比如 C/C++ 程序中的一个函数定义。vim 默认是支持 tags 的，那么可以让 vim 从任何位置跳转到相应的标示位置。

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

#### 生成系统tags

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

### cscope

{% highlight text %}
:help if_cscop
:cscope show       查看已经加载的数据库
:cscope add        添加数据库
:cscope kill       关闭已加载的数据库
:cscope reset      重新初始化所有连接
{% endhighlight %}

可以将 cscope 看做是 ctags 的升级版本，提供交互式查询语言符号功能，如查询哪些地方使用某个变量或调用某个函数。

和 tags 一样，默认支持，可以通过 `:version` 查看，没有则增加 `--enable-cscope` 编译选项，详细可查看官方网址为 [cscope.sourceforge.net](http://cscope.sourceforge.net/) 。

确定 Vim 已支持 Cscope 后，将文件 
[cscope_maps.vim](http://cscope.sourceforge.net/cscope_maps.vim) 下载到 `~/.vim/plugin` 目录下；也可以将其内容添加到 `~/.vimrc` 配置文件中。

在 CentOS 中，可以通过 `yum install cscope` 安装，在第一次解析时扫描全部文件，以后再调用 cscope 只会扫描那些改动过的文件。

进入项目代码根目录运行命令生成 cscope 数据库文件：

{% highlight text %}
cscope -Rbq -f cscope.out

常用参数：
  -R     表示递归操作，无该参数则进入基于cureses的GUI界面，通过方向键选择查找类
           型、TAB键搜索结果和搜索类型中选择、Ctrl-D退出；
  -b     生成数据库文件后直接退出，否则生成数据库之后会进入 cscope 界面；
  -q     表示生成 cscope.in.out 和 cscope.po.out 文件，加快 cscope 的索引速度；
  -C     在搜索时忽略大小写；
  -Ppath 默认是相对路径，可以在相对路径的文件前加上path。
  -Idir  在指定的目录中查找头文件；
{% endhighlight %}

<!--
-k 在生成数据库时如果在当前项目目录下没有找到头文件，则会自动到/usr/include目录下查找，通过-k选项关闭；
-i 如果不是cscope.files，则通过该选项指定源文件列表，-表示标准输入；
-u 扫描所有文件，重新生成交叉索引文件；
-->

默认只解析 C、lex 和 yacc 文件，如果希望解析 C++ 和 Java 文件，可以通过指定 cscope.files 文件，此时不再需要 -R 选项。

<!--
  然后可以通过 cscope find 进行查找，vim支持8中cscope的查询功能，如下：<ul><li>
     0/s, symbol: 查找C语言符号，即查找函数名、宏、枚举值等出现的地方。</li><li>
     1/g, global: 查找函数、宏、枚举等定义的位置，通常为全局变量，类似ctags所提供的功能。</li><li>
     2/d, called: 查找本函数所调用的函数。</li><li>
     3/c, calls: 查找所有调用本函数的函数。</li><li>
     4/t, text: 查找指定的字符串。</li><li>
     6/e, egrep: 查找egrep模式，相当于egrep功能，但查找速度快多了。</li><li>
     7/f, file: 查找并打开文件，类似vim的find功能。</li><li>
     8/i, includes: 查找包含当前cursor的文件。</li></ul>
 如查找调用do_cscope()函数的函数，可以输入 :cs find c do_cscope 或者使用 :cs find s do_cscope 。</li><br><li>

    </li><br><li>
 Cscope 常用快捷键，此处是通过vimrc配置的<br>
   Ctrl-\ s 查找所有当前光标所在符号出现过位置。<br>
   Ctrl-\ c 查找所有调用当前光标所在函数的函数。<br><br>


find $PROJECT_HOME -name *.c -o -name "*.cpp" -o -name "*.cc" -o -name "*.h" -o -name "*.hpp" > cscope.files
cscope -qbR -i cscope.files


    vim提供了一些选项可以调整它的cscope功能：<ul><li>
        cscopeprg选项用于设置cscope程序的位置。</li><br><li>

        cscopequickfix设定是否使用quickfix窗口来显示cscope的结果，这是一组用逗号分隔的值，每项都包含于csope-find命令（s, g, d, c, t, e, f, 或者i）和旗标（+, -或者0）。<!-- ‘+’预示着显示结果必须追加到quickfix窗口。‘-’隐含着清空先前的的显示结果，’0’或者不设置表示不使用quickfix窗口。查找会从开始直到第一条命令出现。默认的值是””（不使用quickfix窗口）。下面的值似乎会很有用：”s-,c-,d-,i-,t-,e-”。 详情请”:help cscopequickfix“。</li><br><li>

        如果你想vim同时搜索tag文件以及cscope数据库，设置cscopetag选项，此时使用的是cstag而不是tag，默认是关闭的set cst/nocst。</li><br><li>

        cscopetagorder选项决定:cstag是先查找tag文件还是先查找cscope数据库。设置为0则先查找cscope数据库，设置为1先查找tag文件。默认是0，set csto=0/1。</li><br><li>

        cscopeverbose是否显示数据库增加/失败，如 set csverb/nocsverb。</li><br><li>

       ‘cspc’的值决定了一个文件的路径的多少部分被显示。默认值是0，所以整个路径都会被显示。值为1的话，那么就只会显示文件名，不带路径。其他值就会显示不同的部分。例如 :set cspc=3 将会显示文件路径的最后3个部分，包含这个文件名本身。 </li></ul>

    vim的手册中给出了使用cscope的建议方法，使用命令”:help cscope-suggestions“查看。
 </p>
-->

#### 查看Linux Kernel

对于内核中感兴趣的文件通常需要去除文本、非x86源码、非驱动。

{% highlight text %}
$ tar -Jxf linux-kernel.tar.xz
$ LNX=/home/andy/linux-kernel
$ find  $LNX                                                              \
    -path "$LNX/arch/*" ! -path "$LNX/arch/i386*" -prune -o               \
    -path "$LNX/include/asm-*" ! -path "$LNX/include/asm-i386*" -prune -o \
    -path "$LNX/tmp*" -prune -o                                           \
    -path "$LNX/Documentation*" -prune -o                                 \
    -path "$LNX/scripts*" -prune -o                                       \
    -path "$LNX/drivers*" -prune -o                                       \
    -name "*.[chxsS]" -print >/home/andy/cscope/cscope.files
$ cd /home/andy/cscope
$ cscope -b -q -k                             生成Cscope数据库文件
$ add some files and 'cscope -b -q -k'        添加新文件重新生成数据库
{% endhighlight %}

在 find 命令中 `-o` 表示 `or`，`-prune` 表示不包含该目录，如果前面又添加了 `!` 号，则表示只包含该目录。

<!--
duplicate cscope database not added
使用Cscope时两次加载cscope，可以通过 grep -rne "cscope.out" 检查 ~/.vim/ 目录和 /etc/ 目录，通常时由于 ~/.vim/plugin
-->

#### 常用快捷键

{% highlight text %}
F2    替换末尾的空格
F3    Tagbar(左侧)
F4    Nerdtree(右侧)
{% endhighlight %}


<!--
##############################################################################

### 移动光标 :help motion

其中帮助还可以查看 :help word/cursor-motions/various-motions 命令。

在 VIM 中，光标的移动非常重要，很多命令可以和其进行组合，常见的如 &lt;start position&gt; &lt;command&gt; &lt;end position&gt; ，如果没有指定 &lt;start position&gt; 则默认为当前光标所在位置。

如 0y$ 复制从 0 开始到 $ 结尾，也就是行首到行尾的数据；ye 复制从当前光标到词的结尾的数据；y2/foo 复制当前光标到第二次出现 foo 的数据。不仅 y(yank) 其它的操作如 d(delete)、v(visual select)、c(change)、gU (uppercase，变为大写)、gu (lowercase，变为小写) 等。

### 常规方式

{% highlight text %}
[h|j|k|l]           → 等价于光标键(←↓↑→)
35j                 → 向下调转35行

0                   → 数字零，到行头
^/Home              → 到本行第一个不是空字符的位置(所谓空字符就是空格、tab、换行、回车等)
<NUM>|              → 跳转到某一列
$/End               → 到本行行尾
g_                  → 到本行最后一个不是空字符的位置

<NUM>G              → 跳转到第<NUM>行，默认到最后一行
:<NUM>              → 同上
<NUM>gg             → 到第N行，默认第一行

H                   → 跳转到窗口的第一行
M                   → 跳转到窗口的中间
L                   → 跳转到窗口的最后一行
w                   → 跳转到下一个单词的开始处，用标点符号和空格分开
W                   → 跳转到下一个单词的开始处，用空格分开
b                   → 跳转到上一个单词的开始处，用标点符号和空格分开
B                   → 跳转到上一个单词开始处，用空格分开
e                   → 跳转到单词的末尾，使用标点符号和空格分开
E                   → 跳转到单词的末尾，使用空格分开
(                   → 跳转到上一句
)                   → 跳转到下一句
{                   → 调转到上一段
}                   → 调转到下一段
{% endhighlight %}

### 查找调转

{% highlight text %}
%                   → 匹配括号移动，包括 (, {, [，需要在当前行
[*|#]               → 匹配光标当前所在的单词，移动光标到下一个或上一个匹配单词

3fh                 → 调转到第3次出现h字符的地方
t,                  → 到逗号前的第一个字符，逗号可以变成其它字符，T反向
3fa                 → 在当前行查找第三个出现的a，F反向

Ctrl-o              → 跳回到前一个位置
Ctrl-i              → 跳转到后一个位置

/pattern            → 搜索pattern的字符串，通过 n N 来查找下一个或上一个
{% endhighlight %}

![network]({{ site.url }}/images/vim/vim_word_moves.jpg){: .pull-center}
![network]({{ site.url }}/images/vim/vim_line_moves.jpg){: .pull-center}

### Marks :help mark-motions

在 VIM 中可以自己定义标记，用来调转。

{% highlight text %}
''(双单引号)              → 在最近的两个标记的地方相互切换，回到第一个非空字符上
``(双反引号)              → 与上述的功能相同，只是该命令会回到之前的列
:jumps                   → 列出可以跳转的位置列表，当前的用>标记
m[char]                  → 使用26个字符(a-z)标记当前位置，如ma
`[char]                  → 跳转到标记处，如`a
'[char]                  → 同上，跳转到所在行的行首，如'a
:marks                   → 列出所有的标记
:delmarks markname       → 删除标记。
{% endhighlight %}

## 查找 :help pattern

通过* # 可以查找当前光标所在单词位置。

{% highlight text %}
/\<step\>                → 只查找step，其中\<和\>表示一个单词的起始和结尾
/\d                      → 查找一个数字
/\d\+                    → 查找一个或两个数字
/\d\*                    → 查找0个或多个数字
{% endhighlight %}

## 替换，h :substite

vim 中替换命令方式如下。

{% highlight text %}
[ADDR]s/src/dest/[OPTION]
      ADDR: %(1,$)整个文件；1,20第1行到第20行；.,$当前行到行尾
    OPTION: g全局替换；c每次需要确认；i忽略大小写；p不清楚作用
{% endhighlight %}

可以参考如下的示例。

{% highlight text %}
:%s/\(That\) \(this\)/\u\2 \l\1/       将That this 换成This that
:%s/child\([ ,.;!:?]\)/children\1/g    将非句尾的child换成children
:%s/  */ /g                            将多个空格换成一个空格
:%s/\([:.]\)  */\1 /g                  使用空格替换句号或者冒号后面的一个或者多个空格
:g/^$/d                                删除所有空行
:%s/^/  /                              在每行的开始插入两个空白
:%s/$/hello/g                          在每行的莫为插入两个空白
:s/hi/hh/g                             将本行的hi替换为hh
:.,5/$/./                              在接下来的6行末尾加入.
:%s/\r//g                              删除DOS的回车符(^M)
:%s/\r/\r/g                            将DOS回车符，替换为回车
:%s/^\(.*\)\n\1$/\1/g                  删除重复的行
{% endhighlight %}

<!--
http://www.guckes.net/vi/substitute.html<br>
<a href="reference/vim/vi_substitution_guide.html">Vi Pages - Substitution Guide </a>


:set ignorecase     Ignore case in searches
:set smartcase  Ignore case in searches excepted if an uppercase letter is used
:%s/\<./\u&/g   Sets first letter of each word to uppercase
:%s/\<./\l&/g   Sets first letter of each word to lowercase
:%s/.*/\u&  Sets first letter of each line to uppercase
:%s/.*/\l&  Sets first letter of each line to lowercase
Read/Write files
:1,10 w outfile     Saves lines 1 to 10 in outfile
:1,10 w >> outfile  Appends lines 1 to 10 to outfile
:r infile   Insert the content of infile
:23r infile     Insert the content of infile under line 23
File explorer
:e .    Open integrated file explorer
:Sex    Split window and open integrated file explorer
:browse e   Graphical file explorer
:ls     List buffers
:cd ..  Move to parent directory
:args   List files
:args *.php     Open file list
:grep expression *.php  Returns a list of .php files contening expression
gf  Open file name under cursor
Interact with Unix
:!pwd   Execute the pwd unix command, then returns to Vi
!!pwd   Execute the pwd unix command and insert output in file

Alignment
:%!fmt  Align all lines
!}fmt   Align all lines at the current position
5!!fmt  Align the next 5 lines
Tabs
:tabnew     Creates a new tab
gt  Show next tab
:tabfirst   Show first tab
:tablast    Show last tab
:tabm n(position)   Rearrange tabs
:tabdo %s/foo/bar/g     Execute a command in all tabs
:tab ball   Puts all open files in tabs
Window spliting
:e filename     Edit filename in current window
:split filename     Split the window and open filename
ctrl-w up arrow     Puts cursor in top window
ctrl-w ctrl-w   Puts cursor in next window
ctrl-w_     Maximise current window
ctrl-w=     Gives the same size to all windows
10 ctrl-w+  Add 10 lines to current window
:vsplit file    Split window vertically
:sview file     Same as :split in readonly mode
:hide   Close current window
:­nly   Close all windows, excepted current
:b 2    Open #2 in this window
Auto-completion
Ctrl+n Ctrl+p (in insert mode)  Complete word
Ctrl+x Ctrl+l   Complete line
:set dictionary=dict    Define dict as a dictionnary
Ctrl+x Ctrl+k   Complete with dictionnary
Marks
mk  Marks current position as k
˜k  Moves cursor to mark k
d™k     Delete all until mark k
Abbreviations
:ab mail mail@provider.org  Define mail as abbreviation of mail@provider.org
Text indent
:set autoindent     Turn on auto-indent
:set smartindent    Turn on intelligent auto-indent
:set shiftwidth=4   Defines 4 spaces as indent size
ctrl-t, ctrl-d  Indent/un-indent in insert mode

## 撤销/重做 help undo-todo

帮助文档，也可以通过 help usr_32.txt 或者 help undolist 查看。

{% highlight text %}
u                        → 撤销
Ctrl-r                   → 重做
:earlier 4m              → 回到4分钟前
:later 45s               → 前进45s钟
:undo 5                  → 撤销5次
:undolist                → 查看undo列表
{% endhighlight %}

## 多窗口操作 :help split

{% highlight text %}
new filename             → 水平打开新窗口
<NUM>sp(lit) filename    → 水平窗口打开文件，默认当前窗口文件，NUM指定新窗口的大小
Ctrl+W s                 → 同上

vnew filename            → 垂直打开新窗口
<NUM>vsp(lit) filename   → 垂直窗口打开文件，默认当前窗口文件，NUM指定新窗口的大小
Ctrl+W v                 → 同上

close                    → 关闭窗口，内容仍然保存，会保留最后一个窗口
Ctrl+W c                 → 同上
quit/ZZ/Ctrl+W q         → 可以关闭最后一个窗口，当多于一个窗口时上述的命令等效
only                     → 只保留当前窗口，如果其它窗口有未保存的修改，则会提示错误

CTRL-W w                 → 切换窗口
CTRL-W h                 → 到左边的窗口
CTRL-W j                 → 到下面的窗口
CTRL-W k                 → 到上面的窗口
CTRL-W l                 → 到右边的窗口
CTRL-W t                 → 到顶部窗口
CTRL-W r                 → 各个窗口旋转
CTRL-W b                 → 到底部窗口

<NUM>Ctrl-w =|+|-        → 均分/增加/减小窗口的大小(NUM行)
<NUM>Ctrl-w [><]         → 宽度设置
<NUM>Ctrl-w _            → 设置当前窗口行数，默认最大化
<NUM>Ctrl-w |            → 垂直设置
:[vertical] res(ize) num → 指定窗口的行数
:[vertical] res(ize)+num → 增大num行
:[vertical] res(ize)-num → 减小num行
{% endhighlight %}

也可以启动 VIM 时，使用 -[O|o]NUM 参数来垂直或水平分屏，例如，vim -O2 file1 file2。


![network]({{ site.url }}/images/vim/vim_split.gif){: .pull-center}

VIM 支持文件浏览，可以通过 :Ex 在当前窗口开启目录浏览器，:Sex 水平分割当前窗口，并在一个窗口中开启目录浏览器。
## 拷贝/粘贴以及粘贴板配置

{% highlight text %}
d    → delete删除
y    → yank复制
P    → 在当前位置之后粘贴
p    → 在当前位置之前粘贴

yap  → 复制整个段，注意段后的空白行也会被复制
yw   → 也会复制词后的空格
yy   → 复制当前行当行到 ddP
{% endhighlight %}

常用的一些技巧包括：xp (交换两个字符)，dwwP (交换两个词，删除一个 word，跳转到下一个，复制) 。


vim 中有十几个粘贴板，分别是 [0-9]、a、"、+ 等，可以直接通过 :reg 命令查看各个粘贴板里的内容，如果只是简单用 y 命令，只会复制到 " 粘贴板里，同样 p 粘贴的也是这个粘贴板里的内容。

\+ 为系统粘贴板，用 "+y 将内容复制到该粘贴板后可以使用 Ctrl＋V 将其粘贴到其他文档，反之则可以通过 "+p 复制到 VIM 。另外，对于系统粘贴板比较有效的是 Shift+Ctrl+v，以及 Shift+Insert 即可，实践中实际要比 "*p、"+p 有用！

要将 vim 的内容复制到某个粘贴板，需要在 Normal 模式中，选择要复制的内容，然后按 "Ny 完成复制，其中 N 为如上所述的粘贴板号。反之，复制到 VIM 时，也要在 Normal 模式下，通过 "Np 粘贴。

另需注意，默认 vim.basic 是没有 + 号寄存器的，需要单独编译，或者 vim.gtk、vim.gnome 才会有。
-->

## 其它插件

简单介绍一些常见的插件。

### xterm-color-table

查看颜色列表，可从 [color-table](http://www.vim.org/scripts/script.php?script_id=3412) 下载，下载解压后可以放置到 ```$VIMRUNTIME/colors``` 目录下，通过 ```:XtermColorTable``` 命令即可查看。

其它可以查看 doc 目录下的帮助文件，也可以查看如下的图片 [xterm color](/images/linux/vim_xterm_color_chart.png) 。

### number.vim

更好的显示行号，在 Normal 状态时会在主窗口中显示绝对/相对行号，插入状态时显示绝对行号，不需要进行任何配置。

{% highlight text %}
let g:numbers_exclude = ['tagbar', 'gundo', 'minibufexpl', 'nerdtree']
nnoremap <F3> :NumbersToggle<CR>
nnoremap <F4> :NumbersOnOff<CR>
{% endhighlight %}

### vim-startify

一个启动界面替换默认的 vim 启动界面，会显示一些最近打开的文件等信息。

### fencview

对于 vim 打开文件时如何自动识别，可以使用如下设置：

{% highlight text %}
set fileencodings=utf-8,gb2312,ucs-bom,euc-cn,euc-tw,gb18030,gbk,cp936
{% endhighlight %}

或者通过 fencview 插件，该插件主要用于自动识别汉字、日文等，主要有如下的命令。

{% highlight text %}
:FencAutoDetect                  " 自动识别文件编码
:FencView                        " 打开一个编码列表窗口，用户选择编码reload文件
{% endhighlight %}

可以在 vimrc 中设置如下选项：

{% highlight text %}
let g:fencview_autodetect = 1    " 打开文件时自动识别编码
let g:fencview_checklines = 10   " 检查前后10行来判断编码，或者'*'检查全文
{% endhighlight %}

可以将 `autodetect` 设置为 0，当发现为乱码时，那么可以使用 `:FencAutoDetect` 命令查看，然后通过 `:set fileencoding` 查看当前编码，并将其编码放置到 `fileencodings` 中。

可以参考 [VIM 文件编码识别与乱码处理](http://edyfox.codecarver.org/html/vim_fileencodings_detection.html) 。

<!-- /reference/linux/vim_fileencoding.maff
Adds file type glyphs/icons to popular Vim plugins
https://github.com/ryanoasis/vim-devicons
-->

### 其它


还有一些小众的常用。

* [vim-matrix-screensaver](https://github.com/uguu-org/vim-matrix-screensaver) 黑客帝国屏幕保护插件，很酷很炫。
* [Accelerated-Smooth-Scroll](https://github.com/yonchu/accelerated-smooth-scroll) 使得 Ctrl+F Ctrl+B 的翻页操作更顺滑一些。


<!--
https://xiaozhou.net/from-vim-to-neovim-2016-05-21.html
https://github.com/plasticboy/vim-markdown
https://github.com/TimothyYe/mydotfiles
https://xiaozhou.net/chrome-vim-and-markdown-2013-08-07.html
-->

{% highlight text %}
{% endhighlight %}
