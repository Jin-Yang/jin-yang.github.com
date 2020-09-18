---
title: Hello World !!!
layout: post
comments: true
language: chinese
category: [misc]
keywords: hello world,示例,sample,markdown
description: 简单记录一下一些与 Markdown 相关的内容，包括了一些使用模版。
---


<!-- more -->








    每个命令行块都是以根目录为基础的。例如下面命令行块，都是基于根目录的。

cd src/pages
mkdir Home

    技术栈均是目前最新的。

    react 15.6.1
    react-router-dom 4.2.2
    redux 3.7.2
    webpack 3.5.5

    目录说明

│  .babelrc                          #babel配置文件
│  package-lock.json
│  package.json
│  README.MD
│  webpack.config.js                 #webpack生产配置文件
│  webpack.dev.config.js             #webpack开发配置文件
│
├─dist
├─public                             #公共资源文件
└─src                                #项目源码
    │  index.html                    #index.html模板
    │  index.js                      #入口文件
    │
    ├─component                      #组建库
    │  └─Hello
    │          Hello.js
    │
    ├─pages                          #页面目录
    │  ├─Counter
    │  │      Counter.js
    │  │
    │  ├─Home
    │  │      Home.js
    │  │
    │  ├─Page1
    │  │  │  Page1.css                #页面样式
    │  │  │  Page1.js
    │  │  │
    │  │  └─images                    #页面图片
    │  │          brickpsert.jpg
    │  │
    │  └─UserInfo
    │          UserInfo.js
    │
    ├─redux
    │  │  reducers.js
    │  │  store.js
    │  │
    │  ├─actions
    │  │      counter.js
    │  │      userInfo.js
    │  │
    │  ├─middleware
    │  │      promiseMiddleware.js
    │  │
    │  └─reducers
    │          counter.js
    │          userInfo.js
    │
    └─router                        #路由文件
            Bundle.js
            router.js


### 1. 初始化项目

首先创建项目目录，并初始化。

{% highlight text %}
----- 创建文件夹并进入
$ mkdir react && cd react

----- 按照提示填写项目基本信息
$ npm init
{% endhighlight %}

### 2. 配置WebPack

首先是安装，默认是安装到本地目录下。

{% highlight text %}
----- 安装
$ npm install --save-dev webpack@3
{% endhighlight %}

一般来说，`--save-dev` 是开发时依赖的东西，`--save` 是发布之后还依赖的东西。

#### 2.1 编写配置文件

编写最基础的配置文件 `webpack.dev.config.js` 。

{% highlight javascript %}
const path = require('path');
module.exports = {
	/*入口*/
	entry: path.join(__dirname, 'src/index.js'),

	/*输出到dist文件夹，输出文件名字为bundle.js*/
	output: {
		path: path.join(__dirname, './dist'),
		filename: 'bundle.js'
	}
};
{% endhighlight %}

#### 2.2 编译文件

首先，根据如上的配置新建入口文件 `src/index.js` ，并添加如下内容。

{% highlight javascript %}
document.getElementById('app').innerHTML = "Webpack works"
{% endhighlight %}

然后，执行命令 `./node_modules/.bin/webpack --config webpack.dev.config.js` ，编译完成后，会自动生成 `dist/bundle.js` 文件。

最后，在 `dist` 文件夹下面新建一个 `index.html` 文件。

{% highlight text %}
<!doctype html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<title>Document</title>
</head>
<body>
<div id="app"></div>
<script type="text/javascript" src="./bundle.js" charset="utf-8"></script>
</body>
</html>
{% endhighlight %}

用浏览器打开 `index.html` 即可。

也就是说，这里把入口文件 `index.js` 处理后，生成 `bundle.js` 。

### 3. Babel

Babel 把用最新标准编写的 JavaScript 代码向下编译成可以在今天随处可用的版本，通俗的说，就是我们可以用 ES6, ES7 等来编写代码，Babel 会把他们统统转为 ES5 。

{% highlight text %}
babel-core            调用Babel的API进行转码
babel-loader
babel-preset-es2015   用于解析 ES6
babel-preset-react    用于解析 JSX
babel-preset-stage-0  用于解析 ES7 提案
{% endhighlight %}

先通过如下命令安装。

{% highlight text %}
$ npm install --save-dev babel-core babel-loader babel-preset-es2015 babel-preset-react babel-preset-stage-0
{% endhighlight %}

然后，新建 babel 配置文件 `.babelrc` 。

{% highlight text %}
{
	"presets": [
		"es2015",
		"react",
		"stage-0"
	],
	"plugins": []
}
{% endhighlight %}

修改 `webpack.dev.config.js` 文件，增加 `babel-loader` 。

{% highlight javascript %}
/*src文件夹下面的以.js结尾的文件，要使用babel解析*/
/*cacheDirectory是用来缓存编译结果，下次编译加速*/
module: {
	rules: [{
		test: /\.js$/,
		use: ['babel-loader?cacheDirectory=true'],
		include: path.join(__dirname, 'src')
	}]
}
{% endhighlight %}

简单测试下是否能正确转义 `ES6`，修改 `src/index.js` 。

{% highlight text %}
/*使用es6的箭头函数*/
var func = str => {
	document.getElementById('app').innerHTML = str;
};
func('Hello Babel!');
{% endhighlight %}

执行打包命令 `./node_modules/.bin/webpack --config webpack.dev.config.js`，并通过浏览器打开 `index.html` 。

<!--
Babel介绍
https://segmentfault.com/a/1190000008159877

Babel 入门教程
http://www.ruanyifeng.com/blog/2016/01/babel.html
-->


### 4. React

同样，首先要安装。

{% highlight text %}
$ npm install --save react react-dom
{% endhighlight %}

修改 `src/index.js` 使用 React 。

{% highlight text %}
import React from 'react';
import ReactDom from 'react-dom';

ReactDom.render(<div>Hello React!</div>, document.getElementById('app'));
{% endhighlight %}

执行打包命令 `./node_modules/.bin/webpack --config webpack.dev.config.js`，然后打开 `index.html` 看效果。

#### 4.1 组件化

这里简单处理下，把 `Hello React` 放到组件里面，新建文件 `src/component/Hello/Hello.js`。

{% highlight text %}
import React, {Component} from 'react';

export default class Hello extends Component {
	render() {
		return (<div>Hello Component React!</div>)
	}
}
{% endhighlight %}

然后修改 `src/index.js`，引用 `Hello` 组件！

{% highlight text %}
import React from 'react';
import ReactDom from 'react-dom';
import Hello from './component/Hello/Hello';

ReactDom.render(<Hello/>, document.getElementById('app'));
{% endhighlight %}

同上，在根目录执行打包命令，并测试。

#### 4.3 优化命令

修改 `package.json` 里面的 script，增加 `dev-build` ，然后可以通过 `npm run dev-build` 编译即可。

{% highlight text %}
"scripts": {
	"test": "echo \"Error: no test specified\" && exit 1",
	"dev-build": "webpack --config webpack.dev.config.js"
}
{% endhighlight %}


### 5. React Router


{% highlight text %}
----- 安装
$ npm install --save react-router-dom

----- 新建Router文件夹和组件
$ mkdir src/router && touch src/router/router.js
{% endhighlight %}

如下添加一个最基本的 Router 文件，也就是 `src/router/router.js` 。

{% highlight text %}
import React from 'react';
import {BrowserRouter as Router, Route, Switch, Link} from 'react-router-dom';
import Home from '../pages/Home/Home';
import Page1 from '../pages/Page1/Page1';

const getRouter = () => (
	<Router>
		<div>
			<ul>
				<li><Link to="/">首页</Link></li>
				<li><Link to="/page1">Page1</Link></li>
			</ul>
			<Switch>
				<Route exact path="/" component={Home}/>
				<Route path="/page1" component={Page1}/>
			</Switch>
		</div>
	</Router>
);

export default getRouter;
{% endhighlight %}

接着创建所需的文件。

{% highlight text %}
----- 新建两个页面 Home Page1
$ mkdir -p src/pages/{Home,Page1}
{% endhighlight %}

对应的源码文件如下，分别为 `src/pages/Home/Home.js` 和 `src/pages/Page1/Page1.js` 。

{% highlight text %}
import React, {Component} from 'react';

export default class Home extends Component {
	render() { return(<div>this is home~</div>) }
}
{% endhighlight %}

{% highlight text %}
import React, {Component} from 'react';

export default class Page1 extends Component {
	render() { return(<div>this is Page1~</div>) }
}
{% endhighlight %}

路由和页面建好了，然后需要修改入口文件 `src/index.js` 。

{% highlight text %}
import React from 'react';
import ReactDom from 'react-dom';
import getRouter from './router/router';

ReactDom.render(getRouter(), document.getElementById('app'));
{% endhighlight %}

最后，执行打包命令 `npm run dev-build` ，打开 `index.html` 页面。

注意，这里点击链接实际上是没有反映的，主要是由于采用的是类似 `file:///your/index/path/index.html` 路径，而非 `http://localhost:3000/` 之类的路径。

<!--
    Nginx, Apache, IIS等配置启动一个简单的的WEB服务器。
    使用webpack-dev-server来配置启动WEB服务器。

下一节，我们来使用第二种方法启动服务器。这一节的DEMO，先放这里。

参考地址
    http://www.jianshu.com/p/e3adc9b5f75c
    http://reacttraining.cn/web/guides/quick-start
-->

### 6. webpack-dev-server

简单来说 `webpack-dev-server` 就是一个小型的静态文件服务器，可以为 webpack 打包生成的资源文件提供简单的 Web 服务。

{% highlight text %}
$ npm install webpack-dev-server@2 --save-dev
{% endhighlight %}

注意：这里没有全局进行安装，后面使用的时候要写相对路径，可以通过 `npm install webpack-dev-server@2 -g` 进行全局安装。

接着，修改 `webpack.dev.config.js` 增加 webpack-dev-server 的配置。

{% highlight text %}
devServer: {
	contentBase: path.join(__dirname, './dist')
}
{% endhighlight %}

接着执行 `./node_modules/.bin/webpack-dev-server --config webpack.dev.config.js`，然后浏览器打开 `http://localhost:8080`，现在就可以点击相关的链接了。


<!--
看URL地址变化啦！我们看到react-router已经成功了哦。

Q: --content-base是什么？

A：URL的根目录。如果不设定的话，默认指向项目根目录。

重要提示：webpack-dev-server编译后的文件，都存储在内存中，我们并不能看见的。你可以删除之前遗留的文件dist/bundle.js，
仍然能正常打开网站！

每次执行webpack-dev-server --config webpack.dev.config.js,要打很长的命令，我们修改package.json，增加script->start:

  "scripts": {
    "test": "echo \"Error: no test specified\" && exit 1",
    "dev-build": "webpack --config webpack.dev.config.js",
    "start": "webpack-dev-server --config webpack.dev.config.js"
  }

下次执行npm start就可以了。

既然用到了webpack-dev-server，我们就看看它的其他的配置项。
看了之后，发现有几个我们可以用的。

    color（CLI only） console中打印彩色日志
    historyApiFallback 任意的404响应都被替代为index.html。有什么用呢？你现在运行
    npm start，然后打开浏览器，访问http://localhost:8080,然后点击Page1到链接http://localhost:8080/page1，
    然后刷新页面试试。是不是发现刷新后404了。为什么？dist文件夹里面并没有page1.html,当然会404了，所以我们需要配置
    historyApiFallback，让所有的404定位到index.html。
    host 指定一个host,默认是localhost。如果你希望服务器外部可以访问，指定如下：host: "0.0.0.0"。比如你用手机通过IP访问。
    hot 启用Webpack的模块热替换特性。关于热模块替换，我下一小节专门讲解一下。
    port 配置要监听的端口。默认就是我们现在使用的8080端口。
    proxy 代理。比如在 localhost:3000 上有后端服务的话，你可以这样启用代理：

    proxy: {
      "/api": "http://localhost:3000"
    }

    progress（CLI only） 将编译进度输出到控制台。

根据这几个配置，修改下我们的webpack-dev-server的配置~

webpack.dev.config.js

    devServer: {
        port: 8080,
        contentBase: path.join(__dirname, './dist'),
        historyApiFallback: true,
        host: '0.0.0.0'
    }

CLI ONLY的需要在命令行中配置

package.json

"dev": "webpack-dev-server --config webpack.dev.config.js --color --progress"

现在我们执行npm start 看看效果。是不是看到打包的时候有百分比进度？在http://localhost:8080/page1页面刷新是不是没问题了？
用手机通过局域网IP是否可以访问到网站？

参考地址：
    https://segmentfault.com/a/1190000006670084
    https://webpack.js.org/guides/development/#using-webpack-dev-server
-->

### 7. 模块热替换

<!-- Hot Module Replacement -->

到目前为止，当修改代码时浏览器会自动刷新，这里看下 webpack 模块热替换教程。

<!--
使用自动编译代码时，可能会在保存文件时遇到一些问题。某些编辑器具有“安全写入”功能，可能会影响重新编译。

要在一些常见的编辑器中禁用此功能，请查看以下列表：
    Sublime Text 3 - 在用户首选项(user preferences)中添加 atomic_save: "false"。
    IntelliJ - 在首选项(preferences)中使用搜索，查找到 "safe write" 并且禁用它。
    Vim - 在设置(settings)中增加 :set backupcopy=yes。
    WebStorm - 在 Preferences > Appearance & Behavior > System Settings 中取消选中 Use "safe write"。
-->

<!-- 我相信看这个教程的人，应该用过别人的框架。我们在修改代码的时候，浏览器不会刷新，只会更新自己修改的那一块。我们也要实现这个效果。 -->

接下来需要在 `package.json` 文件中增加 `--hot`，也就是如下内容。

{% highlight text %}
"start": "./node_modules/.bin/webpack-dev-server --config webpack.dev.config.js --color --progress --hot"
{% endhighlight %}

同时在 `src/index.js` 文件中增加 `module.hot.accept()`，内容如下，当模块更新时，会通知 `index.js` 。

{% highlight text %}
import React from 'react';
import ReactDom from 'react-dom';
import getRouter from './router/router';

if (module.hot) {
	module.hot.accept();
}

ReactDom.render(getRouter(), document.getElementById('app'));
{% endhighlight %}

现在执行 `npm start`，打开浏览器，修改 Home.js ，在不刷新页面的情况下，内容会自动更新了。

<!--
做模块热替换，我们只改了几行代码，非常简单的。纸老虎一个~

现在我需要说明下我们命令行使用的--hot，可以通过配置webpack.dev.config.js来替换，
向文档上那样,修改下面三处。但我们还是用--hot吧。下面的方式我们知道一下就行，我们不用。同样的效果。

const webpack = require('webpack');

devServer: {
    hot: true
}

plugins:[
     new webpack.HotModuleReplacementPlugin()
]

HRM配置其实有两种方式，一种CLI方式，一种Node.js API方式。我们用到的就是CLI方式，比较简单。
Node.js API方式，就是建一个server.js等等，网上大部分教程都是这种方式，这里不做讲解了。

你以为模块热替换到这里就结束了？nonono~

上面的配置对react模块的支持不是很好哦。

例如下面的demo，当模块热替换的时候，state会重置，这不是我们想要的。

修改Home.js,增加计数state

src/pages/Home/Home.js

import React, {Component} from 'react';

export default class Home extends Component {
    constructor(props) {
        super(props);
        this.state = {
            count: 0
        }
    }

    _handleClick() {
        this.setState({
            count: ++this.state.count
        });
    }

    render() {
        return (
            <div>
                this is home~<br/>
                当前计数：{this.state.count}<br/>
                <button onClick={() => this._handleClick()}>自增</button>
            </div>
        )
    }
}

你可以测试一下，当我们修改代码的时候，webpack在更新页面的时候，也把count初始为0了。

为了在react模块更新的同时，能保留state等页面中其他状态，我们需要引入react-hot-loader~

Q:　请问webpack-dev-server与react-hot-loader两者的热替换有什么区别？

A: 区别在于webpack-dev-server自己的--hot模式只能即时刷新页面，但状态保存不住。因为React有一些自己语法(JSX)是HotModuleReplacementPlugin搞不定的。
而react-hot-loader在--hot基础上做了额外的处理，来保证状态可以存下来。（来自segmentfault）

下面我们来加入react-hot-loader v3,

安装依赖

npm install react-hot-loader@next --save-dev

根据文档，
我们要做如下几个修改~

    .babelrc 增加 react-hot-loader/babel

.babelrc

{
  "presets": [
    "es2015",
    "react",
    "stage-0"
  ],
  "plugins": [
    "react-hot-loader/babel"
  ]
}

    webpack.dev.config.js入口增加react-hot-loader/patch

webpack.dev.config.js

    entry: [
        'react-hot-loader/patch',
        path.join(__dirname, 'src/index.js')
    ]

    src/index.js修改如下

src/index.js

import React from 'react';
import ReactDom from 'react-dom';
import {AppContainer} from 'react-hot-loader';

import getRouter from './router/router';

/*初始化*/
renderWithHotReload(getRouter());

/*热更新*/
if (module.hot) {
    module.hot.accept('./router/router', () => {
        const getRouter = require('./router/router').default;
        renderWithHotReload(getRouter());
    });
}

function renderWithHotReload(RootElement) {
    ReactDom.render(
        <AppContainer>
            {RootElement}
        </AppContainer>,
        document.getElementById('app')
    )
}

现在，执行npm start，试试。是不是修改页面的时候，state不更新了？

参考文章：

    gaearon/react-hot-loader#243
-->

### 8. 文件路径优化

在之前的代码中，在引用组件或者页面时，写的是相对路径，例如比如 `src/router/router.js` 里面，引用 Home.js 的时候就用的相对路径。

{% highlight text %}
import Home from '../pages/Home/Home';
{% endhighlight %}

webpack 提供了一个别名配置，就是我们无论在哪个路径下，引用都可以这样

{% highlight text %}
import Home from 'pages/Home/Home';
{% endhighlight %}

修改 `webpack.dev.config.js` 增加别名。

{% highlight text %}
    resolve: {
        alias: {
            pages: path.join(__dirname, 'src/pages'),
            component: path.join(__dirname, 'src/component'),
            router: path.join(__dirname, 'src/router')
        }
    }
{% endhighlight %}

然后把之前使用的绝对路径统统改掉，包括 `src/router/router.js` 以及 `src/index.js` 。

{% highlight text %}
import Home from 'pages/Home/Home';
import Page1 from 'pages/Page1/Page1';
{% endhighlight %}

{% highlight text %}
import getRouter from 'router/router';
{% endhighlight %}


### 9. Redux

这里简单实现一个计数器。

{% highlight text %}
$ npm install --save redux
{% endhighlight %}

初始化目录结构以及相关的文件。

{% highlight text %}
$ mkdir -p src/redux/{actions,reducers}
{% endhighlight %}

先来写 action 创建函数，通过该函数可以创建 action，也就是 `src/redux/actions/counter.js` 。

{% highlight text %}
/*action*/
export const INCREMENT = "counter/INCREMENT";
export const DECREMENT = "counter/DECREMENT";
export const RESET = "counter/RESET";

export function increment() {
	return {type: INCREMENT}
}

export function decrement() {
	return {type: DECREMENT}
}

export function reset() {
	return {type: RESET}
}
{% endhighlight %}

接着是 reducer，一个纯函数，接收 action 和旧的 state 生成新 state ，`src/redux/reducers/counter.js` 。

{% highlight text %}
import {INCREMENT, DECREMENT, RESET} from '../actions/counter';

/*
 * 初始化state
 */
const initState = {
	count: 0
};
/*
 * reducer
 */
export default function reducer(state = initState, action) {
	switch (action.type) {
	case INCREMENT:
		return { count: state.count + 1 };
	case DECREMENT:
		return { count: state.count - 1 };
	case RESET:
		return {count: 0};
	default:
		return state
	}
}
{% endhighlight %}

一个项目可能会有很多个 `reducers`，可以通过如下方式把它们整合到一起，`src/redux/reducers.js` 。

{% highlight text %}
import counter from './reducers/counter';

export default function combineReducers(state = {}, action) {
	return { counter: counter(state.counter, action) }
}
{% endhighlight %}

这里无论是 combineReducers 还是 reducer 函数，都是接收 state 和 action，然后返回一个新的 state。

#### 9.1 创建 Store

前面可以使用 action 来描述 "发生了什么"，使用 action 创建函数来返回 action ；并使用 reducers 来根据 action 更新 state 。

那如何提交 action 并触发 reducers 呢？

store 就是把它们联系到一起的对象，其具有以下职责：

* 维持应用的 state；
* 提供 getState() 方法获取 state；
* 提供 dispatch(action) 触发reducers方法更新 state；
* 通过 subscribe(listener) 注册监听器;
* 通过 subscribe(listener) 返回的函数注销监听器。

新建 `src/redux/store.js` 文件。

{% highlight text %}
import {createStore} from 'redux';
import combineReducers from './reducers.js';

let store = createStore(combineReducers);

export default store;
{% endhighlight %}

到此为止，可以使用 redux 了，这里简单测试下，新建 `src/redux/testRedux.js` 。

{% highlight text %}
import {increment, decrement, reset} from './actions/counter';

import store from './store';

// 打印初始状态
console.log(store.getState());

// 每次 state 更新时，打印日志
// 注意 subscribe() 返回一个函数用来注销监听器
let unsubscribe = store.subscribe(() =>
	console.log(store.getState())
);

// 发起一系列 action
store.dispatch(increment());
store.dispatch(decrement());
store.dispatch(reset());

// 停止监听 state 更新
unsubscribe();
{% endhighlight %}

当前文件夹执行命令 `../../node_modules/.bin/webpack testRedux.js build.js && node build.js` ，此时输出为。

{% highlight text %}
{ counter: { count: 0 } }
{ counter: { count: 1 } }
{ counter: { count: 0 } }
{ counter: { count: 0 } }
{% endhighlight %}

也就是说，redux 和 react 没关系，虽说他俩能合作。

<!--
到这里，我建议你再理下redux的数据流，看看这里。

    调用store.dispatch(action)提交action。
    redux store调用传入的reducer函数。把当前的state和action传进去。
    根 reducer 应该把多个子 reducer 输出合并成一个单一的 state 树。
    Redux store 保存了根 reducer 返回的完整 state 树。

就是酱紫~~

-->

调整 `webpack.dev.config.js` 路径别名。

{% highlight text %}
alias: {
	actions: path.join(__dirname, 'src/redux/actions'),
	reducers: path.join(__dirname, 'src/redux/reducers'),
	redux: path.join(__dirname, 'src/redux')
}
{% endhighlight %}

把前面的相对路径都改改。

#### 9.2 配合 React 使用

写一个 Counter 页面 `src/pages/Counter/Counter.js` 。

{% highlight text %}
import React, {Component} from 'react';

export default class Counter extends Component {
	render() {
		return (
			<div>
				<div>当前计数为(显示redux计数)</div>
				<button onClick={() => {
					console.log('调用自增函数');
				}}>自增
				</button>
				<button onClick={() => {
					console.log('调用自减函数');
				}}>自减
				</button>
				<button onClick={() => {
					console.log('调用重置函数');
				}}>重置
				</button>
			</div>
		)
	}
}
{% endhighlight %}

修改路由，增加 Counter，也就是 `src/router/router.js` 。

{% highlight text %}
import React from 'react';

import {BrowserRouter as Router, Route, Switch, Link} from 'react-router-dom';

import Home from 'pages/Home/Home';
import Page1 from 'pages/Page1/Page1';
import Counter from 'pages/Counter/Counter';

const getRouter = () => (
	<Router>
		<div>
			<ul>
				<li><Link to="/">首页</Link></li>
				<li><Link to="/page1">Page1</Link></li>
				<li><Link to="/counter">Counter</Link></li>
			</ul>
			<Switch>
				<Route exact path="/" component={Home}/>
				<Route path="/page1" component={Page1}/>
				<Route path="/counter" component={Counter}/>
			</Switch>
		</div>
	</Router>
);

export default getRouter;
{% endhighlight %}

接着运行 `npm start` 看下效果。

#### 9.3 联合 Counter 和 Redux

使 Counter 能获得到 Redux 的 state，并且能发射 action。

<!--
当然我们可以使用刚才测试testRedux的方法，手动监听~手动引入store~但是这肯定很麻烦哦。

react-redux提供了一个方法connect。

    容器组件就是使用 store.subscribe() 从 Redux state 树中读取部分数据，并通过 props 来把这些数据提供给要渲染的组件。你可以手工来开发容器组件，但建议使用 React Redux 库的 connect() 方法来生成，这个方法做了性能优化来避免很多不必要的重复渲染。

connect接收两个参数，一个mapStateToProps,就是把redux的state，转为组件的Props，还有一个参数是mapDispatchToprops,
就是把发射actions的方法，转为Props属性函数。
-->

先通过命令 `npm install --save react-redux` 安装 `react-redux` 。

然后修改 `src/pages/Counter/Counter.js` 。

{% highlight text %}
import React, {Component} from 'react';
import {increment, decrement, reset} from 'actions/counter';

import {connect} from 'react-redux';

class Counter extends Component {
    render() {
        return (
            <div>
                <div>当前计数为{this.props.counter.count}</div>
                <button onClick={() => this.props.increment()}>自增
                </button>
                <button onClick={() => this.props.decrement()}>自减
                </button>
                <button onClick={() => this.props.reset()}>重置
                </button>
            </div>
        )
    }
}

const mapStateToProps = (state) => {
    return {
        counter: state.counter
    }
};

const mapDispatchToProps = (dispatch) => {
    return {
        increment: () => {
            dispatch(increment())
        },
        decrement: () => {
            dispatch(decrement())
        },
        reset: () => {
            dispatch(reset())
        }
    }
};

export default connect(mapStateToProps, mapDispatchToProps)(Counter);
{% endhighlight %}

下面我们要传入store

    所有容器组件都可以访问 Redux store，所以可以手动监听它。一种方式是把它以 props 的形式传入到所有容器组件中。但这太麻烦了，因为必须要用 store 把展示组件包裹一层，仅仅是因为恰好在组件树中渲染了一个容器组件。

    建议的方式是使用指定的 React Redux 组件 来 魔法般的 让所有容器组件都可以访问 store，而不必显示地传递它。只需要在渲染根组件时使用即可。

src/index.js

import React from 'react';
import ReactDom from 'react-dom';
import {AppContainer} from 'react-hot-loader';
import {Provider} from 'react-redux';
import store from './redux/store';

import getRouter from 'router/router';

/*初始化*/
renderWithHotReload(getRouter());

/*热更新*/
if (module.hot) {
    module.hot.accept('./router/router', () => {
        const getRouter = require('router/router').default;
        renderWithHotReload(getRouter());
    });
}

function renderWithHotReload(RootElement) {
    ReactDom.render(
        <AppContainer>
            <Provider store={store}>
                {RootElement}
            </Provider>
        </AppContainer>,
        document.getElementById('app')
    )
}

到这里我们就可以执行npm start，打开localhost:8080/counter看效果了。

但是你发现npm start一直报错

ERROR in ./node_modules/react-redux/es/connect/mapDispatchToProps.js
Module not found: Error: Can't resolve 'redux' in 'F:\Project\react\react-family\node_modules\react-redux\es\connect'

ERROR in ./src/redux/store.js
Module not found: Error: Can't resolve 'redux' in 'F:\Project\react\react-family\src\redux'

WTF？这个错误困扰了半天。我说下为什么造成这个错误。我们引用redux的时候这样用的

import {createStore} from 'redux'

然而，我们在webapck.dev.config.js里面这样配置了

    resolve: {
        alias: {
            ...
            redux: path.join(__dirname, 'src/redux')
        }
    }

然后webapck编译的时候碰到redux都去src/redux去找了。但是找不到啊。所以我们把webpack.dev.config.js里面redux这一行删除了，就好了。
并且把使用我们自己使用redux文件夹的地方改成相对路径哦。

现在你可以npm start去看效果了。

这里我们再缕下（可以读React 实践心得：react-redux 之 connect 方法详解）

    Provider组件是让所有的组件可以访问到store。不用手动去传。也不用手动去监听。

    connect函数作用是从 Redux state 树中读取部分数据，并通过 props 来把这些数据提供给要渲染的组件。也传递dispatch(action)函数到props。

接下来，我们要说异步action

参考地址： http://cn.redux.js.org/docs/advanced/AsyncActions.html

想象一下我们调用一个异步get请求去后台请求数据：

    请求开始的时候，界面转圈提示正在加载。isLoading置为true。
    请求成功，显示数据。isLoading置为false,data填充数据。
    请求失败，显示失败。isLoading置为false，显示错误信息。

下面，我们以向后台请求用户基本信息为例。

    我们先创建一个user.json，等会请求用，相当于后台的API接口。

cd dist
mkdir api
cd api
touch user.json

dist/api/user.json

{
  "name": "brickspert",
  "intro": "please give me a star"
}

    创建必须的action创建函数。

cd src/redux/actions
touch userInfo.js

src/redux/actions/userInfo.js

export const GET_USER_INFO_REQUEST = "userInfo/GET_USER_INFO_REQUEST";
export const GET_USER_INFO_SUCCESS = "userInfo/GET_USER_INFO_SUCCESS";
export const GET_USER_INFO_FAIL = "userInfo/GET_USER_INFO_FAIL";

function getUserInfoRequest() {
    return {
        type: GET_USER_INFO_REQUEST
    }
}

function getUserInfoSuccess(userInfo) {
    return {
        type: GET_USER_INFO_SUCCESS,
        userInfo: userInfo
    }
}

function getUserInfoFail() {
    return {
        type: GET_USER_INFO_FAIL
    }
}

我们创建了请求中，请求成功，请求失败三个action创建函数。

    创建reducer

再强调下，reducer是根据state和action生成新state的纯函数。

cd src/redux/reducers
touch userInfo.js

src/redux/reducers/userInfo.js

import {GET_USER_INFO_REQUEST, GET_USER_INFO_SUCCESS, GET_USER_INFO_FAIL} from 'actions/userInfo';


const initState = {
    isLoading: false,
    userInfo: {},
    errorMsg: ''
};

export default function reducer(state = initState, action) {
    switch (action.type) {
        case GET_USER_INFO_REQUEST:
            return {
                ...state,
                isLoading: true,
                userInfo: {},
                errorMsg: ''
            };
        case GET_USER_INFO_SUCCESS:
            return {
                ...state,
                isLoading: false,
                userInfo: action.userInfo,
                errorMsg: ''
            };
        case GET_USER_INFO_FAIL:
            return {
                ...state,
                isLoading: false,
                userInfo: {},
                errorMsg: '请求错误'
            };
        default:
            return state;
    }
}

这里的...state语法，是和别人的Object.assign()起同一个作用，合并新旧state。我们这里是没效果的，但是我建议都写上这个哦

组合reducer

src/redux/reducers.js

import counter from 'reducers/counter';
import userInfo from 'reducers/userInfo';

export default function combineReducers(state = {}, action) {
    return {
        counter: counter(state.counter, action),
        userInfo: userInfo(state.userInfo, action)
    }
}

    现在有了action，有了reducer，我们就需要调用把action里面的三个action函数和网络请求结合起来。
        请求中 dispatch getUserInfoRequest
        请求成功 dispatch getUserInfoSuccess
        请求失败 dispatch getUserInfoFail

src/redux/actions/userInfo.js增加

export function getUserInfo() {
    return function (dispatch) {
        dispatch(getUserInfoRequest());

        return fetch('http://localhost:8080/api/user.json')
            .then((response => {
                return response.json()
            }))
            .then((json) => {
                    dispatch(getUserInfoSuccess(json))
                }
            ).catch(
                () => {
                    dispatch(getUserInfoFail());
                }
            )
    }
}

我们这里发现，别的action创建函数都是返回action对象：

{type: xxxx}

但是我们现在的这个action创建函数 getUserInfo则是返回函数了。

为了让action创建函数除了返回action对象外，还可以返回函数，我们需要引用redux-thunk。

npm install --save redux-thunk

这里涉及到redux中间件middleware，我后面会讲到的。你也可以读这里Middleware。

简单的说，中间件就是action在到达reducer，先经过中间件处理。我们之前知道reducer能处理的action只有这样的{type:xxx}，所以我们使用中间件来处理
函数形式的action，把他们转为标准的action给reducer。这是redux-thunk的作用。
使用redux-thunk中间件

我们来引入redux-thunk中间件

src/redux/store.js

import {createStore, applyMiddleware} from 'redux';
import thunkMiddleware from 'redux-thunk';
import combineReducers from './reducers.js';

let store = createStore(combineReducers, applyMiddleware(thunkMiddleware));

export default store;

到这里，redux这边OK了，我们来写个组件验证下。

cd src/pages
mkdir UserInfo
cd UserInfo
touch UserInfo.js

src/pages/UserInfo/UserInfo.js

import React, {Component} from 'react';
import {connect} from 'react-redux';
import {getUserInfo} from "actions/userInfo";

class UserInfo extends Component {

    render() {
        const {userInfo, isLoading, errorMsg} = this.props.userInfo;
        return (
            <div>
                {
                    isLoading ? '请求信息中......' :
                        (
                            errorMsg ? errorMsg :
                                <div>
                                    <p>用户信息：</p>
                                    <p>用户名：{userInfo.name}</p>
                                    <p>介绍：{userInfo.intro}</p>
                                </div>
                        )
                }
                <button onClick={() => this.props.getUserInfo()}>请求用户信息</button>
            </div>
        )
    }
}

export default connect((state) => ({userInfo: state.userInfo}), {getUserInfo})(UserInfo);

这里你可能发现connect参数写法不一样了，mapStateToProps函数用了es6简写，mapDispatchToProps用了react-redux提供的简单写法。

增加路由
src/router/router.js

import React from 'react';

import {BrowserRouter as Router, Route, Switch, Link} from 'react-router-dom';

import Home from 'pages/Home/Home';
import Page1 from 'pages/Page1/Page1';
import Counter from 'pages/Counter/Counter';
import UserInfo from 'pages/UserInfo/UserInfo';

const getRouter = () => (
    <Router>
        <div>
            <ul>
                <li><Link to="/">首页</Link></li>
                <li><Link to="/page1">Page1</Link></li>
                <li><Link to="/counter">Counter</Link></li>
                <li><Link to="/userinfo">UserInfo</Link></li>
            </ul>
            <Switch>
                <Route exact path="/" component={Home}/>
                <Route path="/page1" component={Page1}/>
                <Route path="/counter" component={Counter}/>
                <Route path="/userinfo" component={UserInfo}/>
            </Switch>
        </div>
    </Router>
);

export default getRouter;

现在你可以执行npm start去看效果啦！

















https://doc.webpack-china.org/guides/

Redux 中文文档
http://cn.redux.js.org/index.html

{% highlight text %}
{% endhighlight %}
