---
title: 使用 Jekyll 搭建 Blog
layout: post
comments: true
language: chinese
category: [misc]
keywords: jekyll,github,搭建,免费
description: 在此介绍如何通过 Jekyll 以及 Github 搭建个人的免费 blog 网站，包括了如何搭建，如何进行优化，配置 SEO 相关，一些不错的网站。另外，关键是所有的东西都可以自己控制，可以很方便记录一些常见文章。
---

Jekyll 是一个简单的免费的 Blog 生成工具，类似 WordPress，但是不同的是，Jekyll 只是一个生成静态网页的工具，不需要数据库支持，不支持评论功能，只关注 Blog 本身的内容。

不过，可以配合第三方评论服务，例如 Disqus；最关键的是 Jekyll 可以免费部署在 Github 上，而且可以绑定自己的域名。

接下来，我们看看如何搭建一个 Blog 。

<!-- more -->

![jekyll logo]({{ site.url }}/images/linux/jekyll-logo.png "jekyll logo"){: .pull-center }

## 一步步安装

可以通过如下方式一步步的安装测试。

### 1. 安装Jekyll

安装 Jekyll 环境。

{% highlight text %}
----- 在Ubuntu平台上
# apt-get install ruby rubygems python-pygments
----- 在CentOS平台上
# yum install ruby rubygems ruby-devel rubygem-ffi

----- 安装jekyll时，通过选项--no-rdoc、--no-ri可以使安装最小化
# gem install rdiscount --no-rdoc --no-ri
# gem install jekyll --no-rdoc --no-ri
# gem install redcarpet kramdown                ← 2016.5.1之后Github只支持kramdown
# yum install nodejs

----- 测试环境是否安装成功
$ jekyll -v
{% endhighlight %}

在 CentOS 上，安装 Nodejs 时，需要安装 EPEL 源，否则会出错 Could not find a JavaScript runtime 。

### 2. 创建默认目录结构

Jekyll 会动态创建 Blog ，并将结果保存在 \_site 中，因此访问时直接访问静态文件。

{% highlight text %}
|-- _includes/                  ← 通常是一些页面的组建
|   |-- head.html
|   `-- sidebar.html
|-- _layouts/                   ← 一些通用的模版
|   |-- default.html
|   `-- post.html
|-- _posts/                     ← 就是blog了
|   `-- 2012-08-15-hello.md
|-- _site/                      ← 编译结果
|-- _config.yml                 ← 默认配置文件
|-- index.html
|-- archives.html               ← 通常用来展示归档内容
`-- people/                     ← 其它目录设置
    `-- essay.html              ← 可以通过http://xxx.com/people/essay.html访问
{% endhighlight %}


### 3. 运行

安装完成后可以在源码目录下执行如下命令，

{% highlight text %}
$ jekyll server [port] -w --incremental
{% endhighlight %}

在浏览器中输入 [http://localhost:4000](http://localhost:4000)，其中 -w 与 \-\-watch 相同，修改文件时会自动刷新；\-\-incremental 参数表示只编译更新的文件。


### 4. 上传到 github

Jekyll 是 GitHub Pages 的后台博客引擎，也就是说 Github 可以支持 Jekyll 部署的。

首先，必须在 github 上搭建一个名称为 username.github.io 的代码仓库，其中 username 就是每个 github 用户的用户 ID，只有这样名称的库才能被 github 用于链接到个人博客。

需要注意的是，username 不区分大小写。

![jekyll github logo]({{ site.url }}/images/linux/jekyll-github-logo.png "jekyll github logo"){: .pull-center width="280px" }

可以通过如下方式将更改提交到 github，可以添加 .gitignore 文件，文件内容为。也就是在提交到 github 时，忽略动态生成的网页，忽略备份，忽略尚未完成的文章（未完成的文章使用 \_init.md 作为后缀）。

{% highlight text %}
_site/
*.bak
_posts/*_init.md
{% endhighlight %}

常用操作如下。

{% highlight text %}
----- 通过如下两个方式设置远端
$ git remote set-url origin git@github.com:Jin-Yang/jin-ynag.github.io.git
$ git remote add origin https://github.com/Jin-Yang/jin-yang.github.com.git

$ git add -A                      ← 添加目录下所有的文件
$ git ls-files                    ← 查看已经管理的文件
$ git commit -a -m "comment"      ← 提交添加注释
$ git push                        ← 推送到远端，也就是github
{% endhighlight %}

关于 Github Pages 的更多配置可以参考 [Categories / Customizing GitHub Pages](https://help.github.com/categories/customizing-github-pages/) 。

<!--
Jin-Yang
https://help.github.com/articles/using-jekyll-with-pages
https://kramdown.gettalong.org/syntax.html
-->

### 5. MarkDown 语法

如下是一些常见的 Markdown 语法，以及转换后的 tag 。

![markdown]({{ site.url }}/images/linux/jekyll-markdown.png "markdown"){: .pull-center }

## 配置优化

### 配置文件

也就是 _config.yml ，详细可以参考 [官方配置文件讲解](http://jekyllrb.com/docs/configuration/)，如下是我的一个简单配置文件。

{% highlight text %}
gems: [jekyll-paginate]
paginate: 8
permalink: /blog/:title.html    # 默认是通过月份分割，此时只使用标题
404: 404.html
mardown: kramdown
highlighter: pygments
{% endhighlight %}

注意：现在不再支持 auto: true ，应该在启动时使用 \-\-watch/-w 参数。

另外，**从 2016.5.1 之后，Markdown 引擎只支持 [kramdown](https://github.com/gettalong/kramdown)**，其它的 rdiscount、redcarpet 将不再支持。


### 添加分类

对于 blog 可以在头部通过如下方式设置 。

{% highlight text %}
----- 单个的分类
category: cate

----- 如果是多个分类应该使用
categories: cate1 cate2
categories: [cate1, cate2]
{% endhighlight %}

为文章设置好分类之后，就可以读取分类列表了，所有的分类都在 site.categories 变量中，所以只遍历它，就可以把分类读取出来， 其中： category \| fist 为分类名称， category \| last \| size 为该分类下文章数量。

如果 Category 为中文，显示将会出错，可以通过如下方式修改。

{% highlight text %}
permalink: /:blog/:year/:month/:day/:title
permalink: /blog/:year/:month/:day/:title
{% endhighlight %}

### 头部信息

{% highlight text %}
---
source: http://foobar.com/blog/test     # 如果有引用则指定引用的URL
title: 使用 Jekyll 搭建 Blog             # 本文章的标题
layout: post                            # 使用的模版
comments: false
language: chinese
category: [linux, program]
keywords: 关键词，通过英文逗号分割
description: 文章的描述。
---
{% endhighlight %}

### 字体

有些字体可以从网站上下载，不过对于中文来说，如果要覆盖所有的中文字体就会导致下载比较大的字体包，所以除非固定使用的字，建议还是使用系统自带的。

可以从 [www.dafont.com](http://www.dafont.com/) 下载相应的字体。

{% highlight css %}
/* 为了让多个浏览器支持，可以写成如下格式 */
@font-face {
 font-family: 'YourWebFontName';
 src: url('YourWebFontName.eot'); /* IE9 Compat Modes */
 src: url('YourWebFontName.eot?#iefix') format('embedded-opentype'), /* IE6-IE8 */
      url('YourWebFontName.woff') format('woff'), /* Modern Browsers */
      url('YourWebFontName.ttf')  format('truetype'), /* Safari, Android, iOS */
      url('YourWebFontName.svg#YourWebFontName') format('svg'); /* Legacy iOS */
 font-weight: normal;
 font-style: normal;
}
/* 代码中使用 */
<h2 class="neuesDemo">Neues Bauen Demo</h2>

/* 设置CSS格式 */
h2.neuesDemo {
  font-family: 'NeuesBauenDemo'
}
{% endhighlight %}

### 添加 TOC

网上有介绍使用 Bootstrap 的 Affix & Scrollspy 组件的，不过配置调试起来有些麻烦，不太符合需求，所以就用 javascript 实现了一个简单的 Table Of Contents 。

详细可以直接查看 main.js 源码，会自动区分是否是 post 页面，且文章的内容最高 head 是 h2 。


关于 ScrollSpy 内容，可以参考 [为Jekyll博客添加目录与ScrollSpy效果](http://t.hengwei.me/post/%E4%B8%BAjekyll%E5%8D%9A%E5%AE%A2%E6%B7%BB%E5%8A%A0%E7%9B%AE%E5%BD%95%E4%B8%8Escrollspy%E6%95%88%E6%9E%9C.html)，或者 [本地文档](/reference/linux/jekyll-toc.mht) 。

### 添加搜索

其中功能比较完善的是 [jekyll-lunr-js-search](https://github.com/slashdotdash/jekyll-lunr-js-search)，利用 lunr.js 实现的全文搜索，不过同样配置也有些麻烦，暂时就先不考虑了。

在此，简单实现了搜索，在根目录下保存了一个 json 格式文件，包括了需要搜索的内容，页面加载时同样会请求该文件，在前端进行搜索。


### 高亮显示

需要安装 Python、easy_install、Pygments 工具，对于一些平台，如果 easy_install 没有安装，可以通过如下命令安装。

{% highlight text %}
# apt-get install python-setuptools
# easy_install Pygments
# gem install pygments.rb
{% endhighlight %}

设置 _config.xml，注意 pygments: true 选项已经取消，目前采用的是 highlighter 。

Pygments 提供了多种样式，比如'native'、'emacs'、'vs' 等等，可以从 [Pygments demo](http://pygments.org/demo) 中选择某种语言的例子，支持的语法高亮可以查看 [Available lexers](http://pygments.org/docs/lexers/)，也可以参考如下 [网站](http://pygments.org/docs/styles/) 自定义高亮格式。

通过下面的命令可以查看当前支持的样式：

{% highlight text %}
$ python
>>> from pygments.styles import STYLE_MAP
>>> STYLE_MAP.keys()
['monokai', 'manni', 'rrt', 'perldoc', 'borland', 'colorful',
 'default', 'murphy', 'vs', 'trac', 'tango', 'fruity', 'autumn',
 'bw', 'emacs', 'vim', 'pastie', 'friendly', 'native']
{% endhighlight %}

选择一种样式，应用在 Jekyll 中执行如下命令，其中 "native" 是样式名，"html" 是 formatter 。

{% highlight text %}
$ pygmentize -S native -f html > pygments.css
{% endhighlight %}

### MathJax 支持

首先在头部添加如下的代码。

{% highlight html %}
<!-- mathjax config similar to math.stackexchange -->
<script type="text/x-mathjax-config">
  MathJax.Hub.Config({
    tex2jax: {
      inlineMath: [ ['$','$'], ["\\(","\\)"] ],
      processEscapes: true,
      skipTags: ['script', 'noscript', 'style', 'textarea', 'pre', 'code']
    }
  });
</script>
<script type="text/x-mathjax-config">
    MathJax.Hub.Queue(function() {
        var all = MathJax.Hub.getAllJax(), i;
        for(i=0; i < all.length; i += 1) {
            all[i].SourceElement().parentNode.className += ' has-jax';
        }
    });
</script>
<script type="text/javascript"
   src="http://cdn.mathjax.org/mathjax/latest/MathJax.js?config=TeX-AMS-MML_HTMLorMML">
</script>
{% endhighlight %}

然后，在文章中添加如下代码进行测试。

{% highlight text %}
$$
\begin{align*}
  & \phi(x,y) = \phi \left(\sum_{i=1}^n x_ie_i, \sum_{j=1}^n y_je_j \right)
  = \sum_{i=1}^n \sum_{j=1}^n x_i y_j \phi(e_i, e_j) = \\
  & (x_1, \ldots, x_n) \left( \begin{array}{ccc}
      \phi(e_1, e_1) & \cdots & \phi(e_1, e_n) \\
      \vdots & \ddots & \vdots \\
      \phi(e_n, e_1) & \cdots & \phi(e_n, e_n)
    \end{array} \right)
  \left( \begin{array}{c}
      y_1 \\
      \vdots \\
      y_n
    \end{array} \right)
\end{align*}
$$
{% endhighlight %}

更多的语法可以参考 [https://kramdown.gettalong.org/syntax.html#math-blocks](https://kramdown.gettalong.org/syntax.html#math-blocks) 。

#### Pygments 冲突解决

Pygments 会与 MathJax 的 CSS 渲染模式有所冲突，导致最终渲染后的公式会变成花花绿绿的。

主要是 MathJax 中的 `.mi` `.mo` 与 Pygments 中的 CSS 发生了冲突，出了修改 CSS 之外，还可以在 `base.html` 模板中添加如下的代码：

{% highlight text %}
<style type="text/css">.MathJax .mi, .MathJax .mo { color: #111; font-size: 100%; font-weight: normal; }</style>
{% endhighlight %}

这主要是 HTML 中的 CSS 要高于文件中的。

### 特殊用法

#### 竖杠

`|` 会被无脑识别为 table 的 column 的分隔符，所以在里使用还得用 `\|` ，也可以使用 `\vert` 或者 `\Vert` 替换掉，后者为双竖线。

<!--
http://yaoyao.codes/jekyll/2016/02/21/tears-for-mathjax
-->

### 优化

#### 去除 a 标签虚线

在 CSS 中添加如下内容即可。

{% highlight text %}
a:focus{outline:none;}
{% endhighlight %}

#### 响应式

实际上利用的是 BootStrap 的栅格系统，sidebar 的标签内容如下。

{% highlight text %}
<div class="col-lg-3 visible-lg blog-sidebar">
{% endhighlight %}

也就是只有在 lg 大小的屏幕时，才会显示侧边栏。

#### 表格支持

实际上 kramdown 默认已经支持，我们只需要设置好 ```table thead tbody th tr td``` 对应的 CSS 属性即可。

## SEO 技巧

Search Engine Optimization, SEO 也就是搜索引擎优化，指遵循搜索引擎的搜索原理，对网站结构、网页文字语言和站点间互动外交略等进行合理规划部署，以改善网站在搜索引擎的搜索表现，进而增加客户发现并访问网站的可能性。

在 Github 中，有一个 [jekyll-seo-tag](https://github.com/jekyll/jekyll-seo-tag) 插件可以参考，不过暂时没有采用，防止 Github 不兼容。

### 1. 标签优化

title 通常是标题名+网站名称；另外增加适当的 keywords、description、robots 等标签。

{% highlight html %}
<title>{% if page.title %}{{ page.title }} | {% endif %}JinYang's Blog</title>
<meta name="keywords" content="{{ page.keywords }}">
<meta name="description" content="{{ page.description }}">
{% endhighlight %}

### 2. 站长工具

通常有 [Google](http://www.google.com/webmasters/tools/)、[Baidu](http://zhanzhang.baidu.com/site/index) 等搜索引擎的站长工具，通常需要在网站的 ```<head> </head>``` 头部添加类似如下的特定元标记，用于标记确实是你网站，也就是用于认证。

{% highlight text %}
<meta name="baidu-site-verification" content="B786jeR0MV" />
{% endhighlight %}

这样就有利于搜索引擎的收录。

### 3. sitemap

sitemap 用于告知搜索引擎，在该网站上有哪些可供抓取的网页，以便搜索引擎可以更加智能地抓取网站。

最简单的就是 xml 文件，在其中列出网站中的网址以及关于每个网址的其他元数据，包括了上次更新的时间、更改的频率以及相对于网站上其他网址的重要程度等信息。

该文件设置的内容可以直接参考源码 sitemap.xml 。


<!--
1：博客要经常保持更新，文字始终紧紧围绕关键字，围绕网站主题。

2：SEO优化重点以文章内容页优化为主。


4：在新文章中适当增加一些老文章的链接，文章的内部链接一定要自然。

6：栏目分类尽量使用目录形式。如：http://lusongsong.com/youhua/而不是http://lusongsong.com/youhua.html

7：可以在网页底部加上站点地图sitemap，HTML格式是给用户看的，XML格式是给搜索引擎看的。

8：比较重要文章的URL可以使用拼音、英文和分割线等网址结构。

9：外部链接建设很重要，新博客推荐使用工具http://tool.lusongsong.com/seo/

10：垃圾留言一定要删除，非常影响排名。

11：记得给每个博文插图加上和文章内容相关的alt信息，并在图片周围添加相关信息。

12：文章中相同的关键词不能过多，如果非要堆砌关键词，可适当使用长尾关键词。

13：每篇文章的结尾处可加入“原创文章如转载，请注明出处”“本文首发于XXX网站”等信息，对SEO有一定帮助。

14：通过添加TAG标签可以增强主题的相关性和被搜索的概率。

16：文章标题最好能出现一次关键字。

17：关键字最好在第一段或最后一段能够出现。
-->


### 添加评论

Jekyll 打建后的是一个静态网站，可以通过 disqus 或者 "多说" 添加评论功能；其中 disqus 需要翻墙才可以，所以考虑到国内情况，还是用多说吧。

在 [duoshuo.com](http://duoshuo.com/) 选择 "我要安装"，填写信息，会自动生成一段代码；然后放置到 post 模版中。

## 使用 Gitment

[Gitment]() 是作者实现的一款基于 GitHub Issues 的评论系统，支持在前端直接引入，不需要任何后端代码，可以在页面进行登录、查看、评论、点赞等操作，适合各种基于 GitHub Pages 的静态博客或项目页面。

### 1. 注册 OAuth Application

通过 [GitHub Settings/applications](https://github.com/settings/applications/new) 注册一个新的 OAuth Application，其它内容可以随便填写，一定要确保 callback URL 正确，一般是评论页面对应的域名，如 https://imsun.net）。

然后会得到一个 client ID 和一个 client secret，这个将被用于之后的用户登录。

### 2. 调用gitment

如 gitment 项目页 Readme 所示，在你需要添加评论系统的地方，一般是 `_layout/post.html`, 添加如下代码。

{% highlight text %}
<div id="gitmentContainer"></div>
<link rel="stylesheet" href="https://imsun.github.io/gitment/style/default.css">
<script src="https://imsun.github.io/gitment/dist/gitment.browser.js"></script>
<script>
var gitment = new Gitment({
    id: '{{ page.date | date: "%s" }}',
    owner: 'Your GitHub username',
    repo: 'The repo to store comments',
    oauth: {
        client_id: 'Your client ID',
        client_secret: 'Your client secret',
    },
});
gitment.render('gitmentContainer');
</script>
{% endhighlight %}

需要修改的有 4 个地方。

* Your GitHub username：填写你的Github Pages博客所在的github账户名
* The repo to store comments：填写用来存放评论的github仓库
* Your client ID：第1步所申请到的应用的Client ID
* Your client secret：第1步所申请到的应用的Client Secret

填写完这 4 项把代码保存上传到 github 就可以了，上传代码后，随便打开一篇文章，然后选择帐号登陆。

### 3. 初始化评论系统

由于 gitment 的原理是为每一遍博文以其 URL 作为标识创建一个 github issue，对该篇博客的评论就是对这个 issue 的评论，因此需要为每篇博文初始化一下评论系统。

初始化后，会可以在 github 上创建相对应的 issue 。

<!--
接下来，介绍一下如何初始化评论系统

上面第2步代码添加成功并上传后，你就可以在你的博文页下面看到一个评论框，还 有看到以下错误Error: Comments Not Initialized，提示该篇博文的评论系统还没初始化

点击Login with GitHub后，使用自己的github账号登录后，就可以在上面错误信息 处看到一个Initialize Comments的按钮 (ps: 由于要求回调URL和当前地址一样，故第2步不能在本地调试， 需把代码先上传再调试)

点击Initialize Comments按钮后，就可以开始对该篇博文开始评论了， 同时也可以在对应的github仓库看到相应的issue
-->


## 参考

可以参考中文官方网站 [Jekyll 将纯文本转化为静态网站和博客](http://jekyll.com.cn/) 或者参考英文网站 [Jekyll Transform your plain text into static websites and blogs](http://jekyllrb.com/) ；其中可以参考 [各种 blog 模版](https://github.com/jekyll/jekyll/wiki/Sites) 。

关于搭建 Blog 可以查看 [Jekyll和Github搭建个人静态博客](http://pwnny.cn/original/2016/06/26/MakeBlog.html) 中的介绍；还有两篇之前保存的 [用Jekyll构建静态网站](http://yanping.me/cn/blog/2011/12/15/building-static-sites-with-jekyll/)、[教你一步一步搭建Jekyll博客](http://zhanglubing.github.io/2012-08-15/setup-jekyll-step-by-step.html)，也可以参考本地保存的 [blog1](/reference/misc/Jekyll和Github搭建个人静态博客.mht)、[blog2](/reference/misc/用Jekyll构建静态网站.mht)、[blog3](/reference/misc/教你一步一步搭建Jekyll博客.mht)。

Markdown 语法的目标是：成为一种适用于网络的书写语言，详细可以参考 [Markdown 语法说明 (简体中文版)](http://wowubuntu.com/markdown/)，或者 [本地保存文档](/reference/misc/Markdown-Syntax.tar.gz) 。


<!--
https://github.com/olakara/JekyllMetro/blob/master/feed.xml

-->

{% highlight text %}
{% endhighlight %}
