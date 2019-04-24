---
title: React 简明教程
layout: post
comments: true
language: chinese
category: [misc]
keywords: react,introduce
description: React 起源于 Facebook 的内部项目，据说是因为该对市场上 JavaScript MVC 框架都不满意，就决定自己写一套，用来架设 Instagram 的网站，做出来以后，发现这套东西很好用，就在 2013 年 5 月开源了。其设计思想极其独特，属于革命性创新，性能出众，代码逻辑却非常简单。这里简单介绍其使用方法。
---

React 起源于 Facebook 的内部项目，据说是因为该对市场上 JavaScript MVC 框架都不满意，就决定自己写一套，用来架设 Instagram 的网站，做出来以后，发现这套东西很好用，就在 2013 年 5 月开源了。

其设计思想极其独特，属于革命性创新，性能出众，代码逻辑却非常简单。

这里简单介绍其使用方法。

<!-- more -->

![react logo]({{ site.url }}/images/react-logo.png "react logo "){: .pull-center width="70%" }

## 1. 安装使用

React 的安装包可以从 [官网](https://reactjs.org/) 下载，简单的示例可以参考 [Try React](https://reactjs.org/docs/try-react.html)，其中 [Single File Example](https://raw.githubusercontent.com/reactjs/reactjs.org/master/static/html/single-file-example.html) 是一个单页的示例。

关于发布版本可以查看 [Github React Release](https://github.com/facebook/react/releases) 。


## 2. HTML 模版

最简单的可以参考如上的 [Single File Example](https://raw.githubusercontent.com/reactjs/reactjs.org/master/static/html/single-file-example.html) ，将其中的 JS 从 [Github React Release](https://github.com/facebook/react/releases) 下载后，可以简化如下。

{% highlight text %}
<!DOCTYPE html>
<html>
	<head>
		<meta charset="UTF-8" />
		<title>Hello World</title>
		<script src="../build/react.js"></script>
		<script src="../build/react-dom.js"></script>
		<script src="../build/browser.min.js"></script>
	</head>
	<body>
		<div id="example"></div>
		<script type="text/babel">
			ReactDOM.render(<h1>Hello, world!</h1>, document.getElementById('root'));
		</script>
	</body>
</html>
{% endhighlight %}

需要注意的是，在最后一个 `<script>` 标签的 `type` 属性为 `text/babel`，这是因为 React 独有的 JSX 语法，跟 JavaScript 不兼容。

这里供用到了三个库：A) react.js 是 React 的核心库；B) react-dom.js 是提供与 DOM 相关的功能；C) browser.js 的作用是将 JSX 语法转为 JavaScript 语法，这一步很消耗时间，实际上线的时候，应该将它放到服务器完成。

<!--
可以如下命令将 src 子目录的 js 文件进行语法转换，转码后的文件全部放在 build 子目录。

{% highlight text %}
$ babel src --out-dir build
{% endhighlight %}
-->

其中 `ReactDOM.render()` 是最基本用法，用于将模板转为 HTML 语言，并插入指定的 DOM 节点。

## 3. JMX 语法

如上，HTML 语言直接写在 JavaScript 语言之中，不加任何引号，这就是 JSX 的语法，它允许 HTML 与 JavaScript 的混写。

{% highlight text %}
var names = ['Alice', 'Emily', 'Kate'];

ReactDOM.render(
    <div>
    {
        names.map(function (name, index) {
            return <div key={index}>Hello, {name}!</div>
        })
    }
    </div>,
    document.getElementById('example')
);
{% endhighlight %}

如上是 JSX 的基本语法规则，遇到 HTML 标签 (以 `<` 开头)，就用 HTML 规则解析；遇到代码块 (以 `{` 开头)，就用 JavaScript 规则解析。

实际上，JSX 允许直接在模板插入 JavaScript 变量，如果这个变量是一个数组，则会展开这个数组的所有成员。

{% highlight text %}
var arr = [
    <h1 key="1">Hello world!</h1>,
    <h2 key="2">React is awesome</h2>,
];
ReactDOM.render(
    <div>{arr}</div>,
    document.getElementById('example')
);
{% endhighlight %}

上面代码的arr变量是一个数组，结果 JSX 会把它的所有成员，添加到模板。

## 4. 组件

允许将代码封装成组件，并像插入普通 HTML 标签一样，在网页中插入组件，`React.createClass` 方法就用于生成一个组件类。

{% highlight text %}
var HelloMessage = React.createClass({
    render: function() {
        return <h1>Hello {this.props.name}</h1>;
    }
});

ReactDOM.render(
    <HelloMessage name="John" />,
    document.getElementById('example')
);
{% endhighlight %}

上面代码中，变量 `HelloMessage` 就是一个组件类，当模板插入 `<HelloMessage />` 时，会自动生成 `HelloMessage` 的一个实例，所有组件类都必须有自己的 render 方法，用于输出组件。

**注意** 组件类的第一个字母必须大写，而且只能包含一个顶层标签，否则会报错，例如如下代码包含了两个顶层标签 `h1` 和 `p` 。

{% highlight text %}
var HelloMessage = React.createClass({
    render: function() {
        return <h1>
            Hello {this.props.name}
        </h1><p>
            some text
        </p>;
    }
});
{% endhighlight %}

组件的用法与原生的 HTML 标签完全一致，可以任意加入属性，比如 `<HelloMessage name="John">` ，就是 HelloMessage 组件加入一个 name 属性，值为 John。组件的属性可以在组件类的 `this.props` 对象上获取，比如 name 属性就可以通过 `this.props.name` 读取。

添加组件属性，需要注意 `class` 属性需要写成 `className` ，`for` 属性需要写成 `htmlFor` ，这是因为 `class` 和 `for` 是 `JavaScript` 的保留字。

## 5. this.props.children

`this.props` 对象的属性与组件的属性一一对应，但是有一个例外，就是 `this.props.children` 属性，它表示组件的所有子节点。

{% highlight text %}
var NotesList = React.createClass({
    render: function() {
        return (
            <ol>
                {
                    React.Children.map(this.props.children, function (child) {
                        return <li>{child}</li>;
                    })
                }
            </ol>
        );
    }
});

ReactDOM.render(
    <NotesList>
        <span>hello</span>
        <span>world</span>
    </NotesList>,
    document.getElementById('root')
);
{% endhighlight %}

上面代码的 `NoteList` 组件有两个 `span` 子节点，它们都可以通过 `this.props.children` 读取。

**注意** `this.props.children` 的值有三种可能：A) 如果当前组件没有子节点，它就是 undefined；B) 如果有一个子节点，数据类型是 object；C) 如果有多个子节点，数据类型就是 array 。

为此提供一个工具方法 `React.Children` 来处理 `this.props.children`，可用 `React.Children.map` 来遍历子节点，而不用担心 `this.props.children` 的数据类型。

<!--
https://reactjs.org/docs/react-api.html#react.children
-->

## 6. PropTypes

组件的属性可以接受任意值，字符串、对象、函数等等都可以；有时需要一种机制，验证别人使用组件时，提供的参数是否符合要求。

组件类的 `PropTypes` 属性，就是用来验证组件实例的属性是否符合要求。

{% highlight text %}
var data = 123;

var MyTitle = React.createClass({
    propTypes: {
        title: React.PropTypes.string.isRequired,
    },

    render: function() {
        return <h1> {this.props.title} </h1>;
    }
});

ReactDOM.render(
    <MyTitle title={data} />,
    document.getElementById('example')
);
{% endhighlight %}

上面的 `Mytitle` 组件包含一个 title 属性，且通过 `PropTypes` 告诉 React，这个 title 属性是必须的，而且它的值必须是字符串，如上设置为了数值那么就会报错。

此时，控制台会显示一行错误信息。

{% highlight text %}
Warning: Failed propType: Invalid prop `title` of type `number` supplied to `MyTitle`, expected `string`.
{% endhighlight %}

更多的 PropTypes 设置，可以查看[官方文档](https://reactjs.org/docs/components-and-props.html)。

此外，可以通过 `getDefaultProps()` 方法可以用来设置组件属性的默认值。

{% highlight text %}
var MyTitle = React.createClass({
    getDefaultProps : function () {
        return {
            title : 'Hello World'
        };
    },

    render: function() {
        return <h1> {this.props.title} </h1>;
    }
});

ReactDOM.render(
    <MyTitle />,
    document.body
);
{% endhighlight %}

上面代码会输出 `Hello World`。

## 7. 获取真实的DOM节点

组件并不是真实的 DOM 节点，而是存在于内存之中的一种数据结构，叫做虚拟 DOM (virtual DOM)，只有当它插入文档以后，才会变成真实的 DOM 。

所有 DOM 变动都先发生在虚拟 DOM，然后再将实际发生变动的部分，反映在真实 DOM 上，该算法叫做 DOM diff ，可以提高网页性能。

但是，有时需要从组件获取真实 DOM 的节点，这时就要用到 ref 属性。

{% highlight text %}
var MyComponent = React.createClass({
    handleClick: function() {
        this.refs.myTextInput.focus();
    },
    render: function() {
        return (
            <div>
                <input type="text" ref="myTextInput" />
                <input type="button" value="Focus the text input" onClick={this.handleClick} />
            </div>
        );
    }
});

ReactDOM.render(
    <MyComponent />,
    document.getElementById('root')
);
{% endhighlight %}

上面代码中，组件 `MyComponent` 的子节点有一个文本输入框，用于获取用户的输入，这时就必须获取真实的 DOM 节点，虚拟 DOM 是拿不到用户输入的。

为了做到这一点，文本输入框必须有一个 ref 属性，然后 `this.refs.[refName]` 就会返回这个真实的 DOM 节点。


<!--
需要注意的是，由于 this.refs.[refName] 属性获取的是真实 DOM ，所以必须等到虚拟 DOM 插入文档以后，才能使用这个属性，否则会报错。上面代码中，通过为组件指定 Click 事件的回调函数，确保了只有等到真实 DOM 发生 Click 事件之后，才会读取 this.refs.[refName] 属性。

-->

React 组件支持很多事件，除了 `Click` 事件以外，还有 `KeyDown`、`Copy`、`Scroll` 等，完整的事件清单请查看 [官方文档](https://reactjs.org/docs/events.html#supported-events) 。


## 8. this.state

组件免不了要与用户互动，React 的一大创新，就是将组件看成是一个状态机，一开始有一个初始状态，然后用户互动，导致状态变化，从而触发重新渲染 UI 。

{% highlight text %}
var LikeButton = React.createClass({
    getInitialState: function() {
        return {liked: false};
    },
    handleClick: function(event) {
        this.setState({liked: !this.state.liked});
    },
    render: function() {
        var text = this.state.liked ? 'like' : 'haven\'t liked';
        return (
            <p onClick={this.handleClick}>
                You {text} this. Click to toggle.
            </p>
        );
    }
});

ReactDOM.render(
    <LikeButton />,
    document.getElementById('root')
);
{% endhighlight %}

上面代码是一个 `LikeButton` 组件，其中 `getInitialState()` 用于定义初始状态，也就是一个对象，这个对象可以通过 `this.state` 属性读取。

用户点击组件状态发生变化，`this.setState()` 修改状态值，每次修改后会自动调用 `this.render()` 再次渲染组件。

其中 `this.props` 和 `this.state` 都用于描述组件特性，易混淆，简单区分方法是：A) `this.props` 表示那些一旦定义，就不再改变的特性；B) `this.state` 会随用户互动而产生变化的特性。

## 9. 表单

用户在表单填入的内容，属于用户跟组件的互动，所以不能用 `this.props` 读取。

{% highlight text %}
var Input = React.createClass({
    getInitialState: function() {
        return {value: 'Hello!'};
    },
    handleChange: function(event) {
        this.setState({value: event.target.value});
    },
    render: function () {
        var value = this.state.value;
        return (
            <div>
                <input type="text" value={value} onChange={this.handleChange} />
                <p>{value}</p>
            </div>
        );
    }
});

ReactDOM.render(<Input/>, document.getElementById('root'));
{% endhighlight %}

上面代码中，文本输入框的值，不能用 `this.props.value` 读取，而要定义一个 `onChange()` 事件的回调函数，通过 `event.target.value` 读取用户输入的值。

<!--
textarea 元素、select元素、radio元素都属于这种情况，更多介绍请参考官方文档。
https://reactjs.org/docs/forms.html
-->

## 10. 组件的生命周期

组件的生命周期分成三个状态。

<!--
        Mounting：已插入真实 DOM
        Updating：正在被重新渲染
        Unmounting：已移出真实 DOM
-->

React 为每个状态都提供了两种处理函数，will 函数在进入状态之前调用，did 函数在进入状态之后调用，三种状态共计五种处理函数。

<!--
        componentWillMount()
        componentDidMount()
        componentWillUpdate(object nextProps, object nextState)
        componentDidUpdate(object prevProps, object prevState)
        componentWillUnmount()

此外，React 还提供两种特殊状态的处理函数。

        componentWillReceiveProps(object nextProps)：已加载组件收到新的参数时调用
        shouldComponentUpdate(object nextProps, object nextState)：组件判断是否重新渲染时调用
-->

下面是一个例子。

{% highlight text %}
var Hello = React.createClass({
    getInitialState: function () {
        return { opacity: 1.0 };
    },

    componentDidMount: function () {
        this.timer = setInterval(function () {
            var opacity = this.state.opacity;
            opacity -= .05;
            if (opacity < 0.1) {
                    opacity = 1.0;
            }
            this.setState({
                opacity: opacity
            });
        }.bind(this), 100);
    },

    render: function () {
        return (
            <div style= { { opacity: this.state.opacity } }>
                Hello {this.props.name}
            </div>
        );
    }
});

ReactDOM.render(
    <Hello name="world"/>,
    document.getElementById('root')
);
{% endhighlight %}

上面代码在 `Hello` 组件加载以后，通过 `componentDidMount()` 设置一个定时器，每隔 100 毫秒，就重新设置组件的透明度，从而引发重新渲染。

另外，组件的 style 属性的设置方式也值得注意，不能写成 `style="opacity:{this.state.opacity};"` 而要写成 ```style= { { opacity: this.state.opacity } }``` 这是因为 React 组件样式是一个对象，所以第一重大括号表示这是 JavaScript 语法，第二重大括号表示样式对象。


## 11. AJAX

组件的数据来源，通常是通过 AJAX 请求从服务器获取，可以使用 `componentDidMount()` 设置 AJAX 请求，等到请求成功，再用 `this.setState()` 方法重新渲染 UI 。

{% highlight text %}
var UserGist = React.createClass({
    getInitialState: function() {
        return {
            username: '',
            lastGistUrl: ''
        };
    },

    componentDidMount: function() {
        $.get(this.props.source, function(result) {
            var lastGist = result[0];
            this.setState({
                username: lastGist.owner.login,
                lastGistUrl: lastGist.html_url
            });
        }.bind(this));
    },

    render: function() {
        return (
            <div>
                {this.state.username}'s last gist is <a href={this.state.lastGistUrl}>here</a>.
            </div>
        );
    }
});

ReactDOM.render(
    <UserGist source="https://api.github.com/users/octocat/gists" />,
    document.getElementById('root')
);
{% endhighlight %}

另外一个示例如下。

{% highlight text %}
var RepoList = React.createClass({
    getInitialState: function() {
        return {
            loading: true,
            error: null,
            data: null
        };
    },

    componentDidMount() {
        this.props.promise.then(
            value => this.setState({loading: false, data: value}),
            error => this.setState({loading: false, error: error})
        );
    },

    render: function() {
        if (this.state.loading) {
            return <span>Loading...</span>;
        } else if (this.state.error !== null) {
            return <span>Error: {this.state.error.message}</span>;
        } else {
            var repos = this.state.data.items;
            var repoList = repos.map(function (repo, index) {
                return (
                    <li key={index}><a href={repo.html_url}>{repo.name}</a>
                    ({repo.stargazers_count} stars) <br/> {repo.description}</li>
                );
            });
            return (
                <main>
                    <h1>Most Popular JavaScript Projects in Github</h1>
                    <ol>{repoList}</ol>
                </main>
            );
        }
    }
});

ReactDOM.render(
    <RepoList promise={$.getJSON('https://api.github.com/search/repositories?q=javascript&sort=stars')} />,
    document.getElementById('example')
);
{% endhighlight %}

## 组件生命周期

![react lifecycle]({{ site.url }}/images/javascripts/react/component-lifecycle.jpg "react lifecycle"){: .pull-center width="90%" }

<!--
getDefaultProps

object getDefaultProps()

执行过一次后，被创建的类会有缓存，映射的值会存在this.props,前提是这个prop不是父组件指定的
这个方法在对象被创建之前执行，因此不能在方法内调用this.props ，另外，注意任何getDefaultProps()返回的对象在实例中共享，不是复制
getInitialState

object getInitialState()

控件加载之前执行，返回值会被用于state的初始化值
componentWillMount

void componentWillMount()

执行一次，在初始化render之前执行，如果在这个方法内调用setState，render()知道state发生变化，并且只执行一次
render

ReactElement render()

render的时候会调用render()会被调用
调用render()方法时，首先检查this.props和this.state返回一个子元素，子元素可以是DOM组件或者其他自定义复合控件的虚拟实现
如果不想渲染可以返回null或者false，这种场景下，React渲染一个<noscript>标签，当返回null或者false时，ReactDOM.findDOMNode(this)返回null
render()方法是很纯净的，这就意味着不要在这个方法里初始化组件的state，每次执行时返回相同的值，不会读写DOM或者与服务器交互，如果必须如服务器交互，在componentDidMount()方法中实现或者其他生命周期的方法中实现，保持render()方法纯净使得服务器更准确，组件更简单
componentDidMount

void componentDidMount()

在初始化render之后只执行一次，在这个方法内，可以访问任何组件，componentDidMount()方法中的子组件在父组件之前执行

从这个函数开始，就可以和 JS 其他框架交互了，例如设置计时 setTimeout 或者 setInterval，或者发起网络请求
shouldComponentUpdate

boolean shouldComponentUpdate(
  object nextProps, object nextState
)

    1
    2
    3

这个方法在初始化render时不会执行，当props或者state发生变化时执行，并且是在render之前，当新的props或者state不需要更新组件时，返回false

shouldComponentUpdate: function(nextProps, nextState) {
  return nextProps.id !== this.props.id;
}

    1
    2
    3

当shouldComponentUpdate方法返回false时，讲不会执行render()方法，componentWillUpdate和componentDidUpdate方法也不会被调用

默认情况下，shouldComponentUpdate方法返回true防止state快速变化时的问题，但是如果·state不变，props只读，可以直接覆盖shouldComponentUpdate用于比较props和state的变化，决定UI是否更新，当组件比较多时，使用这个方法能有效提高应用性能
componentWillUpdate

void componentWillUpdate(
  object nextProps, object nextState
)

    1
    2
    3

当props和state发生变化时执行，并且在render方法之前执行，当然初始化render时不执行该方法，需要特别注意的是，在这个函数里面，你就不能使用this.setState来修改状态。这个函数调用之后，就会把nextProps和nextState分别设置到this.props和this.state中。紧接着这个函数，就会调用render()来更新界面了
componentDidUpdate

void componentDidUpdate(
  object prevProps, object prevState
)

    1
    2
    3

组件更新结束之后执行，在初始化render时不执行
componentWillReceiveProps

void componentWillReceiveProps(
  object nextProps
)

    1
    2
    3

当props发生变化时执行，初始化render时不执行，在这个回调函数里面，你可以根据属性的变化，通过调用this.setState()来更新你的组件状态，旧的属性还是可以通过this.props来获取,这里调用更新状态是安全的，并不会触发额外的render调用

componentWillReceiveProps: function(nextProps) {
  this.setState({
    likesIncreasing: nextProps.likeCount > this.props.likeCount
  });
}

    1
    2
    3
    4
    5

componentWillUnmount

void componentWillUnmount()

当组件要被从界面上移除的时候，就会调用componentWillUnmount(),在这个函数中，可以做一些组件相关的清理工作，例如取消计时器、网络请求等
总结

React Native的生命周期就介绍完了，其中最上面的虚线框和右下角的虚线框的方法一定会执行，左下角的方法根据props state是否变化去执行，其中建议只有在componentWillMount,componentDidMount,componentWillReceiveProps方法中可以修改state值
-->

## 参考

这里大部分是参考阮一峰的 [React 入门实例教程](http://www.ruanyifeng.com/blog/2015/03/react.html) 。


<!--
https://reactjs.org/

https://github.com/ruanyf

http://recharts.org/#/zh-CN
https://react-bootstrap.github.io/

https://ui-router.github.io/react/

Admin antd
https://github.com/zuiidea/antd-admin
https://github.com/ant-design/ant-design-pro

Admin on rest
https://github.com/marmelab/admin-on-rest-demo
https://github.com/api-platform/admin

Adminite
https://github.com/zksailor534/react-adminlte-dash
https://github.com/booleanhunter/ReactJS-AdminLTE

CoreUI
https://github.com/mrholek/CoreUI-Free-Bootstrap-Admin-Template
https://github.com/mrholek/coreui-free-react-admin-template

Blur Admin
https://github.com/knledg/react-blur-admin

https://github.com/MacKentoch/react-director-admin-template
https://github.com/rafaelhz/react-material-admin-template
https://github.com/jtg2078/campaign-admin
https://github.com/vaclav-zeman/dashboard-react-template
-->

{% highlight text %}
{% endhighlight %}
