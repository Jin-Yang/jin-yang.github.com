---
title: JavaScript 环境
layout: post
comments: true
language: chinese
category: [program,misc]
keywords: javascript,npm
description: 简单介绍下 JavaScript 经常使用的工具。
---

简单介绍下 JavaScript 经常使用的工具。

<!-- more -->


## Node.js

### 安装测试

在 CentOS 中，可以直接通过如下方式安装并测试。

{% highlight text %}
$ yum install nodejs

$ echo 'console.log("Hello World!");' > /tmp/hello.js
$ node /tmp/hello.js
{% endhighlight %}

另外，可以通过如下代码执行一个服务端，然后通过浏览器访问即可。

{% highlight text %}
$ cat hello.js
var http = require("http");
http.createServer(function(request, response) {
    response.writeHead(200, {
        "Content-Type" : "text/plain" // 输出类型
    });
    response.write("Hello World");    // 页面输出
    response.end();
}).listen(8100);                      // 监听端口号
console.log("nodejs start listen 8100 port!");

$ node hello.js
{% endhighlight %}

然后，通过浏览器访问 [http://localhost:8100](http://localhost:8100) 即可。

## NPM

NPM 是 Node 的模块管理和发布工具，类似于 Python 的 setuptools，包括了 nodejs、grunt、bower 等工具都是通过 NPM 发布的。

### 安装配置

使用默认的镜像源时，可能会导致不稳定，可以通过如下两种方式指定源，如下是淘宝的。

{% highlight html %}
----- 1. 临时指定镜像源
$ npm install --registry http://registry.npm.taobao.org express

----- 2. 永久设置
$ npm config set registry http://registry.npm.taobao.org
{% endhighlight %}

NPM 安装分为本地安装和全局安装两种，区别在于是否使用 `-g` 参数。

{% highlight text %}
# npm install -g grunt-cli        ← 全局安装客户端
$ npm install grunt               ← 安装本项目的目录下
$ npm install grunt --save        ← 安装本项目的目录下，同时保存在package文件中
$ npm install grunt --save-dev    ← 安装本项目的目录下，同时保存在package文件中

$ npm config set prefix "PATH"    ← 设置全局路径
$ npm config get prefix           ← 获取当前设置的目录
{% endhighlight %}

全局会安装到 `/usr/lib/node_modules` 目录下，本地则会安装到 `node_modules` 目录下。

### 常用命令

{% highlight html %}
$ npm ls --depth 0         当前项目的依赖模块
$ npm ls -g --depth 0      全局模块

$ npm uninstall -g <package>   删除全局包
{% endhighlight %}

### package.json

包的定义和 NPM 都围绕着 `package.json` 文件做文章，用于存放模块的名称、版本、作者、模块入口、依赖项等信息，可以通过 `npm help json` 查看帮助文档，详细可查看 [docs.npmjs.com](https://docs.npmjs.com/files/package.json) 。

可以通过 `npm init` 交互式初始化项目，会生成一个 `package.json` 文件，一个 node package 有两种依赖，分别是：A) dependencies，是正常运行该包时所需要的依赖项；B) devDependencies，开发的时候需要的依赖项，像一些进行单元测试之类的包。

{% highlight text %}
{
    "name": "foobar-demo",   # 模块名称
    "version": "1.0.0",      # 版本号，通常是Major.Minor.Patch
    "description": "Hello World",

    "scripts": {
        "product": "webpack",
        "dev": "webpack"
    },

    "dependencies": {
        "jquery": "^3.1.0"
    },
    "devDependencies": {
        "clean-webpack-plugin": "^0.1.10",
        "copy-webpack-plugin": "^3.0.1",
        "css-loader": "^0.23.1",
        "extract-text-webpack-plugin": "^1.0.1",
        "less": "^2.7.1",
        "less-loader": "^2.2.3",
        "style-loader": "^0.13.1",
        "webpack": "^1.13.1",
        "webpack-merge": "^0.14.1"
    }
}
{% endhighlight %}

在定义版本号的时候，有如下的匹配方式：

{% highlight html %}
1.1.1        精确下载安装1.1.1版本的包
>,=1.1.1     大于、小于等于、大于等于1.1.1版本的包
1.0.1-1.1.1  版本范围是包含1.0.1到1.1.1版本的包
~1.1.1       尽量采用靠近1.1.1版本的包，可用版本1.1.1-0到1.1.x-x
~1.1         下载安装1.1.x-x版本的包
~1           下载安装1.x.x-x版本的包
{% endhighlight %}



### 依赖


可以通过如下方式分别安装。

{% highlight html %}
$ npm install --production
$ npm install --dev
{% endhighlight %}


## PhantomJS

PhantomJS 提供了一个浏览器环境的命令行接口，可以把它看作一个 "虚拟浏览器"，其内核采用 WebKit 引擎，除了不能浏览，其他与正常浏览器一样。

通过 npm install phantomjs -g 下载时比较慢，可以从 [npm.taobao.org](https://npm.taobao.org/dist/phantomjs/) 上下载，然后解压，并将文件复制到一个 PATH 目录下即可。

查看当前版本，同时用于测试是否成功。

{% highlight html %}
$ phantomjs --version
$ phantomjs
phantomjs> 1+2
3
phantomjs> function add(a,b) { return a+b; }
undefined
phantomjs> add(1,2)
3
{% endhighlight %}

下面，把上面的 add() 函数保存到一个文件中，然后测试下。

{% highlight html %}
$ cat add.js
function add(a,b){ return a+b; }
console.log(add(1,2));
phantom.exit();

$ phantomjs add.js
3
{% endhighlight %}

console.log() 会将内容在终端显示，exit() 表示退出 phantomjs 环境，一般来说，这行是必须的。其它的一些常见操作可以参考如下。

{% highlight html %}
phantomjs> phantom.version
{
  "major": 1,
  "minor": 5,
  "patch": 0
}

phantomjs> console.log("phantom is awesome")
phantom is awesome

phantomjs> window.navigator
{
   "appCodeName": "Mozilla",
   "appName": "Netscape",
   ... ...
}
{% endhighlight %}

<!--
http://javascript.ruanyifeng.com/tool/phantomjs.html
-->


## GRUNT

这个是 JavaScript 的构建工具，用来执行一些需要反复重复的任务，例如压缩 (minification)、编译、单元测试、linting 等，从而可以简化工作。

另外，还有个 WebPack 不错，不过还没有仔细研究过。

Grunt 基于 Node.js ，用 JS 开发，这样就可以借助 Node.js 实现跨系统跨平台的桌面端的操作，例如文件操作等等；而且，Grunt 及其插件都可以用 NPM 进行管理。

详细参考 [GRUNT 中文官方](http://www.gruntjs.net/)，或者 [英文网站](http://gruntjs.com/) ，及其简单示例 [jquery-tiny-pubsub](https://github.com/cowboy/jquery-tiny-pubsub)。

### 1. 安装

需要安装 grunt-cli 命令行工具，用来通过 grunt 命令执行 Gruntfile.js 定义的 task。

{% highlight html %}
# npm install -g grunt-cli

$ grunt --version
{% endhighlight %}

<!--
grunt-init 使用模版
http://www.gruntjs.net/project-scaffolding
-->

其中 -g 参数表示安装到全局，有两个必须的文件：

* package.json 保存项目元数据，通过 npm 管理；
* Gruntfile.js  配置或定义任务、加载 grunt 插件。

### 2. 编辑package.json

这个文件其实是 Node.js 用来描述一个项目的 JSON 格式文件，可以通过 npm init 命令交互生成文件；当然，也可以直接编辑生成 package.json 文件即可，只需要保证内容正确即可。

{% highlight json %}
{
  "name": "foobar",
  "version": "1.0.0"
}
{% endhighlight %}

上面只是基本的信息，可以在项目的根目录下运行下面的命令测试下。

{% highlight text %}
$ npm install
npm WARN foobar@1.0.0 No description
npm WARN foobar@1.0.0 No repository field.
npm WARN foobar@1.0.0 No license field.
{% endhighlight %}

接下来安装 grunt 插件，然后通过 \-\-save-dev 参数自动添加到 devDependencies 区域中，且包括版本范围；例如，安装最新版本的 grunt 。

{% highlight html %}
$ npm install grunt --save-dev
{% endhighlight %}

此时，该目录下多了一个 node_modules 的文件夹，而且会在 package.json 文件内容增加了一些 devDependencies 相关的包信息。

### 3. 安装插件

可以通过 Grunt 实现一些常用的功能，例如：检查每个 JS 文件语法、合并两个 JS 文件、将合并后的 JS 文件压缩、将 SCSS 文件编译、新建一个本地服务器监听文件变动自动刷新 HTML 文件。

接下来仅看看其中的一个插件 grunt-contrib-uglify 的使用方法，用于压缩 js、css 文件。

{% highlight html %}
$ npm install --save-dev grunt-contrib-uglify
{% endhighlight %}

与上相同，同样利用 \-\-save-dev 参数将刚安装的插件添加到 package.json 文件中。

### 4. 编辑Gruntfile.js

grunt 会调用 Gruntfile.js 这个文件，解析里面的任务并执行相应操作。

首先，还是看一个简单的示例。

{% highlight javascript %}
module.exports = function(grunt) {   // 包装函数

  grunt.initConfig({    // 任务配置,所有插件的配置信息
    pkg: grunt.file.readJSON('package.json'),
    uglify: {           // uglify插件的配置信息
      options: {
        banner: '/*! <%= pkg.name %> <%= grunt.template.today("yyyy-mm-dd") %> */\n'
      },
      build: {
        src: 'src/<%= pkg.name %>.js',
        dest: 'dist/<%= pkg.name %>.min.js'
      }
    }
  });

  // 使用插件的名称
  grunt.loadNpmTasks('grunt-contrib-uglify');

  // 在终端中输入grunt时需要执行的默认操作，可以定义其它操作
  grunt.registerTask('default', ['uglify']);
};
{% endhighlight %}

其中的源码文件保存了简单的 javascript 代码。

{% highlight html %}
console.log('hello world');
{% endhighlight %}

按照规范，所有的代码要包裹在如下的函数里面。

{% highlight javascript %}
module.exports = function(grunt) {
    ...
};
{% endhighlight %}

其中与 Grunt 有关的主要有三块代码：任务配置代码 (调用插件配置一下要执行的任务和实现的功能)、插件加载代码 (把需要用到的插件加载进来)、任务注册代码 (注册一个任务，包含之前在前面编写的任务配置代码) 。

#### 任务配置代码

简单示例如下，具体的任务配置代码以对象格式放在 grunt.initConfig 函数里面。

{% highlight javascript %}
grunt.initConfig({
  pkg: grunt.file.readJSON('package.json'),
  uglify: {
    options: {
      banner: '/*! <%= pkg.name %> <%= grunt.template.today("yyyy-mm-dd") %> */\n'
    },
    build: {
      src: 'src/<%= pkg.name %>.js',
      dest: 'dist/<%= pkg.name %>.min.js'
    }
  }
});
{% endhighlight %}

第二句的作用是读取 JSON 文件，获取其中的信息，方便在后面任务中使用，例如可以通过 ```<%= pkg.name %>``` 显示项目名称。

之后就是 uglify 对象，表示下面任务是调用 uglify 插件的，首先先配置了一些全局的 options 然后新建了一个 build 任务。也就是说，在 uglify 插件下面，有一个 build 任务，将 js 文件压缩。


#### 插件加载代码

上面任务需要用到 grunt-contrib-uglify，当安装完之后，通过如下的代码加载即可。

{% highlight javascript %}
grunt.loadNpmTasks('grunt-contrib-uglify');
{% endhighlight %}

#### 任务注册代码

通过下面的语法注册一个任务。

{% highlight javascript %}
grunt.registerTask('default', ['uglify']);
{% endhighlight %}

上面也就是在 default 上面注册了一个 Uglify 任务，default 就是别名，它是默认的 task，当在项目目录执行 grunt 的时候，会执行注册到 default 上面的任务，也可以注册其它 task，例如：

{% highlight html %}
grunt.registerTask('compress', ['uglify:build']);
{% endhighlight %}

如果想要执行这个 task，输入 grunt compress 命令即可，这个任务会执行 uglify 下面的 build 任务，而不会执行 uglify 里面定义的其他任务。

### 5. 运行

直接通过 grunt 运行即可，此时会在 dist 目录下生成压缩后的文件。

### 其它插件

常用的一些插件可以参考如下：

{% highlight html %}
grunt-contrib-jshint        语法检查
grunt-contrib-concat        合并文件
grunt-contrib-uglify        压缩文件
grunt-contrib-watch         监听文件变动
grunt-contrib-connect       建立本地服务器
grunt-contrib-clean         语法检查
grunt-contrib-qunit         语法检查
grunt-contrib-sass          Scss编译
{% endhighlight %}

这些插件都用 NPM 管理，命名和文档都很规范，更多的插件可参考 [github.com/gruntjs](https://github.com/gruntjs/) ，同样可以通过如下方式安装。

{% highlight html %}
$ npm install --save-dev grunt
$ npm install --save-dev grunt-contrib-concat
$ npm install --save-dev grunt-contrib-jshint
$ npm install --save-dev grunt-contrib-sass
$ npm install --save-dev grunt-contrib-watch
$ npm install --save-dev grunt-contrib-connect
{% endhighlight %}


<!--
http://www.jshint.com/ 用于 jshint 检测，还有 jslint 。

http://tool.css-js.com/ 使用 Uglify 压缩。

http://koala-app.com/index-zh.html 自动编译生成 CSS 。
-->


## 参考

[GRUNT 中文官方](http://www.gruntjs.net/)，或者 [英文网站](http://gruntjs.com/) ，其中的简单示例可以参考 [jquery-tiny-pubsub](https://github.com/cowboy/jquery-tiny-pubsub)。

{% highlight html %}
{% endhighlight %}
