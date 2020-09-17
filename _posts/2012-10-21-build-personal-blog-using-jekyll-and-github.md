---
title: 使用 Jekyll+Github 搭建 Blog
layout: post
language: chinese
category: [misc]
keywords: jekyll,github,搭建,免费,代码高亮,评论
description: Jekyll 是一个简单的免费的 Blog 生成工具，类似 WordPress，但是不同的是，Jekyll 只是一个生成静态网页的工具，不需要数据库支持，不支持评论功能，只关注 Blog 本身的内容。不过，可以配合第三方评论服务，例如 Disqus，也可以通过 Github 提供的接口实现评论功能；最关键的是 Jekyll 可以免费部署在 Github 上，而且可以绑定自己的域名。接下来，我们看看如何搭建一个 Blog 。
---

Jekyll 是一个简单的免费的 Blog 生成工具，类似 WordPress，但是不同的是，Jekyll 只是一个生成静态网页的工具，不需要数据库支持，不支持评论功能，只关注 Blog 本身的内容。

不过，可以配合第三方评论服务，例如 Disqus，也可以通过 Github 提供的接口实现评论功能；最关键的是 Jekyll 可以免费部署在 Github 上，而且可以绑定自己的域名。

接下来，我们看看如何搭建一个 Blog 。

<!-- more -->

![jekyll logo]({{ site.url }}/images/linux/jekyll-logo.png "jekyll logo")

## 简介

Jekyll 是一个简单的静态网站生成器，使用 Ruby 编写，通过 Markdown 和 Liquid 模板生成内容。

最初由 GitHub 联合创始人 Tom Preston-Werner 创立，当前的维护者 Parker Moore 也于 2016 年初加入了 GitHub 。

所以，Github 也提供了静态 Jekyll 的能力，并且内置了很多不错的插件可以直接使用，在 [Pages Github Versions](https://pages.github.com/versions/) 中包含了当前 Github 所使用的软件版本，也包括了所支持的插件信息。

另外，除了自己搭建模板之外，还可以使用 [Github Pages Themes](https://github.com/pages-themes) 中提供的模板。

## 一步步安装

可以通过如下方式一步步的安装测试。

### 1. 安装Jekyll

在 CentOS 中，Jekyll 需要安装 `redhat-rpm-config` 包，以及一些基本编译环境，例如 `gcc` `gcc-c++` `make` 等，然后再通过如下命令安装 Jekyll 环境。

{% highlight text %}
----- 在Ubuntu平台上
# apt-get install ruby rubygems python-pygments
----- 在CentOS平台上
# yum install ruby rubygems ruby-devel rubygem-ffi

----- 安装jekyll时，通过选项--no-rdoc、--no-ri可以使安装最小化
# gem install rdiscount --no-rdoc --no-ri
# gem install jekyll --no-rdoc --no-ri
# gem install redcarpet kramdown rouge          ← 2016.5.1之后Github只支持kramdown
# yum install nodejs

----- 测试环境是否安装成功
$ jekyll -v
{% endhighlight %}

在 CentOS 上，安装 Nodejs 时，需要安装 EPEL 源，否则会有 `Could not find a JavaScript runtime` 的报错。

另外，在安装 Ruby 工具时，如果下载比较慢，可以将源替换掉，命令如下。

{% highlight text %}
gem sources --add https://gems.ruby-china.com/ --remove https://rubygems.org/
{% endhighlight %}

### 2. 创建默认目录结构

Jekyll 会动态创建 Blog ，并将结果保存在 `_site` 目录下，因此访问时直接访问静态文件。

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

在浏览器中输入 [http://localhost:4000](http://localhost:4000)，其中 `-w` 与 `--watch` 相同，修改文件时会自动刷新；`--incremental` 参数表示只编译更新的文件。


### 4. 上传到 github

Jekyll 是 GitHub Pages 的后台博客引擎，也就是说 Github 可以支持 Jekyll 部署的。

首先，必须在 github 上搭建一个名称为 `username.github.io` 的代码仓库，其中 username 就是每个 github 用户的用户 ID，只有这样名称的库才能被 github 用于链接到个人博客。

需要注意的是 username 不区分大小写。

![jekyll github logo]({{ site.url }}/images/linux/jekyll-github-logo.png "jekyll github logo"){: width="180px" }

通过如下方式将更改提交到 github，可以添加 `.gitignore` 文件，文件内容如下，也就是忽略动态生成的网页，忽略备份，忽略尚未完成的文章(未完成的文章使用 `_init.md` 作为后缀)。

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

### 5. 日常开发

这里简单介绍一些常见的技巧。

##### 调试模式

存在一个参数 `jekyll.environment` ，默认值是 `development`，当上传到 Github 之后，该值会变成 `production` ，在启动时可以通过环境变量设置。

```
JEKYLL_ENV=production jekyll build
```

这样，一些评论、广告等只需要线上有的就可以通过该参数进行定制，示例如下。

{% raw %}
```
{% if jekyll.environment == "production" %}
    {% include disqus.html %}
{% endif %}
```
{% endraw %}

### 6. MarkDown 语法

如下是一些常见的 Markdown 语法，以及转换后的 tag 。

![markdown]({{ site.url }}/images/linux/jekyll-markdown.png "markdown")

{% include ads_content.html %}

## 配置优化

### 配置文件

也就是 `_config.yml` ，详细可以参考 [官方配置文件讲解](http://jekyllrb.com/docs/configuration/)，如下是我的一个简单配置文件。

{% highlight text %}
permalink: /post/:title.html    # 修改链接地址，默认会通过月份分割
404: 404.html
paginate: 8
highlighter: rouge
mardown: kramdown
kramdown:
    input: GFM                  # use Github Flavored Markdown !important
    math_engine: mathjax
plugins:
    - jekyll-paginate
repo_example: 'https://github.com/Your-Repo/Project' # 可以定义一些变量在文章中使用
{% endhighlight %}

注意：现在不再支持 `auto: true` 配置项了，需要在启动时使用 `--watch/-w` 参数。

另外，**从 2016.5.1 之后，Markdown 引擎只支持 [kramdown](https://github.com/gettalong/kramdown)**，其它的 rdiscount、redcarpet 将不再支持。

### 高亮显示

在 Github 中，通过 rouge 兼容替换了 pygments ，可以在 `_config.yml` 中添加 `highlighter: rouge` 配置项即可，如果本地没有安装可以通过 `gem install rouge` 命令安装。

该插件支持 100+ 的语言，支持的语言可以通过 `rougify list` 命令查看。

这是通过 Ruby 编写的插件，高亮的内容可以在官网 [rouge.jneen.net](http://rouge.jneen.net/) 中查看，或者源码 [Rogue GitHub](https://github.com/rouge-ruby/rouge) 。

```
highlighter: rouge
markdown: kramdown
kramdown:
  input: GFM
  syntax_highlighter_opts:
    disable     : true      # 如果使用了highlight.js之类的，可以手动关闭
    default_lang: text
    css_class   : 'syntax'  # Pygments使用的是.syntax而Rouge默认是.highlight
```

在使用时有两种方法都可以。

{% raw %}
```
{% highlight javascript %}
document.write("Hello World!!!");
{% endhighlight %}
```
{% endraw %}

或者

{% highlight text %}
```javascript
document.write("Hello World!!!");
```
{% endhighlight %}

因为兼容 Pygments 的格式，所以可以直接使用 Pygments 的 CSS 文件，高亮的示例可以参考 [Syntax Themes](https://stylishthemes.github.io/Syntax-Themes/pygments/) 中的内容，而对应的 CSS 文件可以从 [Github](https://github.com/StylishThemes/Syntax-Themes/tree/master/pygments/css-github) 中下载。

最有将下载的 CSS 文件引入即可，例如是 `pygments-monokai.css` 文件，那么可以在 `<head>` 之间添加如下内容。

```
<link type="text/css" rel="stylesheet prefetch" href="/static/css/pygments-monokai.css">
```

<!--
Pygments 提供了多种样式，比如'native'、'emacs'、'vs' 等等，可以从 [Pygments demo](http://pygments.org/demo) 中选择某种语言的例子，支持的语法高亮可以查看 [Available lexers](http://pygments.org/docs/lexers/)，也
 以参考如下 [网站](http://pygments.org/docs/styles/) 自定义高亮格式。

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
-->

### 添加评论

Jekyll 打建后的是一个静态网站，可以通过 disqus 或者 "多说" (已经停止服务) 添加评论功能，其中 disqus 需要翻墙才可以。

另外，有很多基于 Github 中 Issues 的评论系统，例如 [gitment](https://github.com/imsun/gitment)、 [utterances](https://utteranc.es/)、[gitalk](https://gitalk.github.io/) 等等，这里使用 utterances 作为评论系统，相比这个要简单好多。

安装配置步骤很简单，首先通过 [apps utterances](https://github.com/apps/utterances) 在 Github 上安装应用，然后添加如下内容。

{% highlight text %}
<script src="https://utteranc.es/client.js"
	repo="[ENTER REPO HERE]"
	issue-term="pathname"
	theme="github-light"
	crossorigin="anonymous"
	async>
</script>
{% endhighlight %}

### 添加分类

对于 blog 可以在头部通过如下方式设置 。

{% highlight text %}
----- 单个的分类
category: cate

----- 如果是多个分类应该使用
categories: cate1 cate2
categories: [cate1, cate2]
{% endhighlight %}

为文章设置好分类之后，就可以读取分类列表了，所有的分类都在 `site.categories` 变量中，所以只遍历它，就可以把分类读取出来， 其中：`category | fist` 为分类名称，`category | last | size` 为该分类下文章数量。

如果 Category 为中文，显示将会出错，可以通过如下方式修改。

{% highlight text %}
permalink: /:blog/:year/:month/:day/:title
permalink: /blog/:year/:month/:day/:title
{% endhighlight %}

### 头部信息

在头部可以指定一些参数，然后可以用 `page.XXX` 引用该变量。

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

#### 高亮冲突解决

Pygments/Rouge 会与 MathJax 的 CSS 渲染模式有所冲突，导致最终渲染后的公式会变成花花绿绿的。

主要是 MathJax 中的 `.mi` `.mo` 与 Pygments/Rouge 中的 CSS 发生了冲突，出了修改 CSS 之外，还可以在 `base.html` 模板中添加如下的代码：

{% highlight text %}
<style type="text/css">.MathJax .mi, .MathJax .mo { color: #111; font-size: 100%; font-weight: normal; }</style>
{% endhighlight %}

这主要是 HTML 中的 CSS 要高于文件中的。

{% include ads_content1.html %}

### 其它

#### 日期选择

在版权信息中，通常需要获取当前的年份，正常应该使用 {% raw %}`{{ 'now' | data: '%Y' }}`{% endraw %}，不过貌似有些版本有 Bug ，暂时可以使用 {% raw %}`{{ site.time | date: '%Y' }}`{% endraw %} 替换使用，其中 `time` 的值是 jekyll 运行的时候生成的。

#### 竖杠

`|` 会被无脑识别为 table 的 column 的分隔符，所以在里使用还得用 `\|` ，也可以使用 `\vert` 或者 `\Vert` 替换掉，后者为双竖线。

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

## 参考

* 可以参考中文官方网站 [Jekyll 将纯文本转化为静态网站和博客](http://jekyll.com.cn/) 或者参考英文网站 [Jekyll Transform your plain text into static websites and blogs](http://jekyllrb.com/) ；其中可以参考 [各种 blog 模版](https://github.com/jekyll/jekyll/wiki/Sites) 。
* Markdown 语法的目标是：成为一种适用于网络的书写语言，详细可以参考 [Markdown 语法说明 (简体中文版)](http://wowubuntu.com/markdown/)，或者 [本地保存文档](/reference/misc/Markdown-Syntax.tar.gz) 。


<!--
移动端介绍
https://segmentfault.com/a/1190000008767416

https://github.com/olakara/JekyllMetro/blob/master/feed.xml
https://help.github.com/articles/using-jekyll-with-pages
https://kramdown.gettalong.org/syntax.html
-->

{% highlight text %}
{% endhighlight %}
