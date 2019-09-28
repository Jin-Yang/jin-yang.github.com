---
title: React & JavaScript 语法简介
layout: post
comments: true
language: chinese
category: [misc]
keywords: react,introduce,syntax
description: 这里简单介绍一下 React 的语法。
---

这里简单介绍一下 React 的语法。

<!-- more -->

![react logo]({{ site.url }}/images/react-logo.png "react logo "){: .pull-center width="70%" }


## 调试

使用 [FireFox Redux DevTools](https://addons.mozilla.org/en-US/firefox/addon/remotedev/) 或者 [Chrome Redux DevTools](https://chrome.google.com/webstore/detail/redux-devtools/lmhkpmbekcpmknklioeibfkpmmfibljd)，安装完后，如果打开的是 Redux 应用则会在地址栏中显示插件。

## 简介

### State Props

React 基于状态实现对 DOM 控制和渲染，组件状态分为两种：一种是组件间的状态传递；另一种是组件的内部状态，这两种状态使用 props 和 state 表示。

props 用于从父组件到子组件的数据传递；组件内部通过 state 表示自己的状态，这些状态只能在组件内部修改。

#### 数据流与 Props

React 中的数据流是单向的，只会从父组件传递到子组件，属性 Props(properties) 是父子组件间进行状态传递的接口，React 会向下遍历整个组件树，并重新渲染使用这个属性的组件。

<!--
##### 设置 Props

可以在组件挂载时设置 Props：

var sites = [{title:'itbilu.com'}];
<ListSites sites={sites} />

也可以通过调用组件实例的setProps()方法来设置props：

var sites = [{title:'itbilu.com'}];
var listSites = React.render(
  <ListSites />,
  document.getElementById('example')
)

setProps()方法只能在组件外调用，不能在组件内部调用this.setProps()修改组件属性。组件内部的this.props属性是只读的，只能用于访问props，不能用于修改组件自身的属性。

1.2 JSX语法中的属性设置

JSX语法中，props可以设置为字符串：

<a href="http://itbilu.com">IT笔录</a>

或是通过{}语法设置：

var obj = {url:'itbilu.com', name:'IT笔录'};
<a href="http://{obj.url}">{obj.name}</a>

JSX方法还支持将props直接设置为一个对象：

var site = React.createClass({
  render: function() {
    var obj = {url:'itbilu.com', name:'IT笔录'};
    return: <Site {...obj} />;
  }
})

props还可以用来添加事件处理：

var saveBtn =  React.createClass({
  render: function() {
    <a onClick={this.handleClick} >保存</>
  }
  handleClick: fuction() {
    //…
  }
})


1.3 props的传递

组件接收上级组件的props，并传递props到其下级组件。如：

var myCheckbox = React.createClass({
  render: myCheckbox() {
    var myClass = this.props.checked ? 'MyChecked' : 'MyCheckbox';
    return (
      <div className={myClass} onClick={this.props.onClick}>
        {this.props.children}
      </div>
    );
  }
});
React.render(
  <MyCheckbox checked={true} onClick={console.log.bind(console)}>
    Hello world!
  </MyCheckbox>,
  document.getElementById('example')
);
-->

#### 组件内部状态与 State

<!--
props可以理解为父组件与子组件间的状态传递，而React的组件都有自己的状态，这个内部状态使用state表示。

如，用state定义一个<DropDown />组件的状态：

var SiteDropdown = React.createClass({
  getInitalState: function() {
    return: {
      showOptions: false
    }
  },
  render: function() {
    var opts;
    if(this.state.showOptions) {
      <ul>
      	<li>itbilu.com</li>
      	<li>yijiebuyi.com</li>
      	<li>niefengjun.cn</li>
      </ul>
    };
    return (
      <div onClick={this.handleClick} >
      </ div>
    )
  },
  handleClick: function() {
    this.setSate({
      showOptions: true
    })
  }
});

随着state的改变，render也会被调用，React会对比render的返回值，如果有变化就会DOM。

state与props类似，只能通过setSate()方法修改。不同的是，state只能在组件内部使用，其是对组件本身状态的一个引用。
-->

State 是 React 中组件的一个对象，React 把用户界面当做是状态机，然后根据不同的状态进行渲染，从而简化处理，使用户界面与数据保持一致。

更新组件的 state 会导致重新渲染用户界面(不要操作DOM)。

常用通知 React 数据变化的方法是 `setState(data, callback)`，该方法会合并 data 到 `this.state` 并重新渲染组件；当渲染完成后，调用可选的 callback 回调。

<!--
　　3.那些组件应该有state?
　　　　大部分组件的工作应该是从props里取数据并渲染出来.但是,有时需要对用户输入,服务器请求或者时间变化等作出响应,这时才需要state.
　　　　组件应该尽可能的无状态化,这样能隔离state,把它放到最合理的地方(Redux做的就是这个事情?),也能减少冗余并易于解释程序运作过程.
　　　　常用的模式就是创建多个只负责渲染数据的无状态(stateless)组件,在他们的上层创建一个有状态(stateful)组件并把它的状态通过props
　　　　传给子级.有状态的组件封装了所有的用户交互逻辑,而这些无状态组件只负责声明式地渲染数据.

　　4.哪些应该作为state?
　　　　state应该包括那些可能被组件的事件处理器改变并触发用户界面更新的数据.这中数据一般很小且能被JSON序列化.当创建一个状态化的组件
　　　　的时候,应该保持数据的精简,然后存入this.state.在render()中在根据state来计算需要的其他数据.因为如果在state里添加冗余数据或计算
　　　　所得数据,经常需要手动保持数据同步.

　　5.那些不应该作为state?
　　　　this.state应该仅包括能表示用户界面状态所需要的最少数据.因此,不应该包括:
　　　　　　计算所得数据:
　　　　　　React组件:在render()里使用props和state来创建它.
　　　　　　基于props的重复数据:尽可能保持用props来做作为唯一的数据来源.把props保存到state中的有效的场景是需要知道它以前的值得时候,
　　　　　　因为未来的props可能会变化.


3. Props与state的比较

React会根据props或state更新视图状态。虽然二者有些类似，但应用范围确不尽相同。具体表现如下：

    props会在整个组件数中传递数据和配置，props可以设置任命类型的数据，应该把它当做组件的数据源。其不但可以用于上级组件与下组件的通信，也可以用其做为事件处理器。
    state只能在组件内部使用，state只应该用于存储简单的视图状（如：上面示例用于控制下拉框的可见状态）。
    props和state都不能直接修改，而应该分别使用setProps()和setSate()方法修改。
-->






















### Promise

如果需要向后端发送一个请求，并对返回的数据进行操作，可能首先想到的是回调函数，但如果在这个函数中又需要执行第二个、第三个...第 N 个异步操作，那么回调函数就会一层层的嵌套，严重影响了代码可读性和可维护性。

Promise 就是解决这个问题的方案。

#### 状态

Promise 有三种状态：pending、resolved、rejected，状态之间的转换只能从 pending 到 resolved 或 rejected，并且状态一旦转换就再也无法改变;

<!---
Promise的API：
    Promise的构造器接受一个函数，这个函数接受两个参数：resolved，rejected。
    promise.then(onResolved, onRejected), 不做赘述;
    promise.catch(onRejected), promise.then(undefined, onRejected)的语法糖。
    Promise.resolve(argument)，返回一个Promise对象，具体取决于它接受的参数类型。
        参数为一个Promise对象，直接返回这个对象；
        参数为一个“类promise”对象，将其转化成真正的Promise对象并返回；
        参数为其他值，返回一个以参数值作为其resolved函数参数的Promise对象；
    Promise.reject(obj), 返回一个以参数值(Error的实例)作为其reject函数参数的Promise对象；
    Promise.all(array), 参数值为Promise数组(也可以包含"类Promise"对象)，对数组的每一项调用Promise.resolve()，全部成功则resolved并返回返回值的数组，否则返回第一个rejected的error对象；
    Promise.race(array), 返回数组中最先resolved或者rejected的那个Promise对象的返回值或者error对象。
马克 末世
-->

#### 基本用法

Promise 是一个 JavaScript 对象，用来执行异步操作，并返回得到的值或者失败的信息。

{% highlight text %}
// Promise is something like this.
var promise = new Promise(function(resolved, rejected) {
    //doSomethingAsync();

    if (success) {
        resolved();
    } else {
        rejected();
    }
})

// How to use a promise. First arg is resolved, second is rejected
promise.then(function(res) {
    console.log(res);
}, function(err) {
    alert(err);
})
{% endhighlight %}

#### 链式调用

如果仅有一个 Promise 对象的话，情况较为简单，但当多个 Promise 要按照一定的顺序执行时，事情就变得复杂起来了。

{% highlight text %}
function fetchSomething() {
    return new Promise(function(resolved) {
        if (success) {
            resolved(res);
        }
    });
}
fetchSomething().then(function(res) {
    console.log(res);
    return fetchSomething();
}).then(function(res) {
    console.log('duplicate res');
    return 'done';
}).then(function(tip) {
    console.log(tip);
})
{% endhighlight %}

<!--
then函数始终返回一个promise对象，后续的then要等待返回的promise resolve后才能执行，这样就实现了线性逻辑的链式调用。而返回的promise取决于then函数本身return的值。如果return值本身就是一个promise对象，则替代默认的promise对象作为返回值；如果return值为其他值，则将这个值作为返回的promise的resolve函数的参数值。
四、异常处理

从上面的代码可以看出，then函数接受两个参数：resolved、rejected。上面没写rejected是因为rejected函数是可选的，当然也可以在then之后写catch，.catch(rejected)本质上是.then(undefined, rejected)的语法糖。
这两种方式是有区别的，.then(resolved, rejected)只能捕获之前的promise的异常，而写在其后的.catch(undefined, rejected)还可以捕获其resolved函数产生的异常。另外只要Promise链中有一个promise对象抛出异常，其后所有的resolved都被跳过，直到这个异常被rejected或者catch处理。
五、排序

当需要用数组的数据执行异步操作，因为数组的遍历方法forEach、map等都是同步的，所以结果的顺序就取决于异步操作完成的顺序，如果对顺序有要求，这样就不尽人意。

// 假设fetchID返回一个Promise对象
names.forEach(function(name) {
    fetchID(name).then(function(id) {
        renderInfo(id);
    })
})

这个时候就需要利用then()来制定顺序：

names.reduce(function(sequence, name) {
    return sequence.then(function() {
        return fetchID(name);
    }).then(function(id) {
        renderID(id);
    })
}, Promise.then())

因为此时先遍历的name处理的结果将作为后面的sequence，构成了链式关系，就避免了下载速度决定顺序的问题。但仍然可以优化：因为此时的ID是获取一个，render一个的。如果能够先获取所有的ID再逐条渲染的话，性能会更好。

Promise.all(names.map(fetchID))
       .then(function(IDs) {
           IDS.forEach(function(id) {
               renderID(id);    //同步
           })
       })
-->

### import

#### 1. 在B.js中引用A.js

也就是导入默认的变量。

{% highlight text %}
import A from './A';
{% endhighlight %}

此时 `A.js` 中必须有默认导出 `export default`，则 import 后面的命名可以随意取，因为总会解析到 `A.js` 中的 `export default`，例如：

{% highlight text %}
// A.js
export default 42

// B.js
import A from './A'
import MyA from './A'
import Something from './A'
{% endhighlight %}

#### 2. 使用花括号

也就是导入特定的变量。

{% highlight text %}
import {A} from './A';
{% endhighlight %}

其中花括号中的名字必须是 `A.js` 中通过 `export` 导出的名字，例如：

{% highlight text %}
export const A = 42
{% endhighlight %}

此时如果导入的是不同的名字，那么就会报错。

{% highlight text %}
// B.js
import { A } from './A'
import { myA } from './A'       // Doesn't work!
import { Something } from './A' // Doesn't work!
{% endhighlight %}

#### 3. 导入多个变量

一个模块的默认导出只能有一个，但是可以指定多个导出名，例如

{% highlight text %}
// B.js
import A, { myA, Something } from './A'
{% endhighlight %}

这里导入了默认的 A 以及 myA、Something 。

#### 4. 导入重命名

假设有如下的 `A.js` 文件。

{% highlight text %}
// A.js
export default 42
export const myA = 43
export const Something = 44
{% endhighlight %}

那么在导入时可以通过如下方式进行重命名。

{% highlight text %}
// B.js
import X, { myA as myX, Something as XSomething } from './A'
{% endhighlight %}

## JavaScript 语法

### This

在 Javascript 中 this 总是指向调用它所在方法的对象，this 是在函数运行时，自动生成的一个内部对象，只能在函数内部使用。

#### 1. 全局函数调用

{% highlight javascript %}
var name = "global this";
function globalTest() {
	this.name = "global this";
	console.log(this.name);
}
globalTest(); //global this
{% endhighlight %}

其中 `globalTest()` 是全局性的方法，属于全局性调用，因此 this 就代表全局对象 window。

#### 2. 对象方法调用

如果函数作为对象的方法调用，this 指向的是这个上级对象，即调用方法的对象。

{% highlight javascript %}
function showName() {
	console.log(this.name);
}
var obj = {};
obj.name = "object name";
obj.show = showName;
obj.show(); // object name
{% endhighlight %}

#### 3. 构造函数调用

构造函数中的 this 指向新创建的对象本身。

{% highlight javascript %}
function showName() {
	this.name = "showName function";
}
var obj = new showName();
console.log(obj.name); //showName function
{% endhighlight %}

通过 new 关键字创建一个对象的实例，将 this 指向对象 obj 。

#### 4. Call Apply Bind

三者在功能上没有区别，都是改变 this 的指向，其区别主要在于方法的实现形式和参数传递：

{% highlight text %}
function.call(object, arg1, arg2, ....)
function.apply(object，[arg1, arg2, ...])
var ss=function.bind(object, arg1, arg2, ....)
{% endhighlight %}

使用方式如下：

{% highlight javascript %}
function show(sex){
	console.log("function" + sex);
}
var person = {
	name: "your name",
	age: 14
};
show.call(person, "mail");
show.apply(person, ["femail"]);
var func = show.bind(person,"unknown");
func();
{% endhighlight %}



## 参考


{% highlight text %}
{% endhighlight %}
