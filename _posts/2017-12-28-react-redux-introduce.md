---
title: React + Redux 简介
layout: post
comments: true
language: chinese
category: [misc]
keywords: react,introduce,syntax,redux
description: React 只是提供了一个 DOM 的抽象层，并不是 Web 应用的完整解决方案，其中组件之间的通信并没有涉及，而大型系统通常需要这一功能。为了解决这一问题，2014 年 Facebook 提出了 Flux 架构的概念；2015 年，Redux 出现，将 Flux 与函数式编程结合一起，很短时间内就成为了最热门的前端架构。这里简单介绍一下 React 和 Redux 的使用语法。
---

React 只是提供了一个 DOM 的抽象层，并不是 Web 应用的完整解决方案，其中组件之间的通信并没有涉及，而大型系统通常需要这一功能。

为了解决这一问题，2014 年 Facebook 提出了 Flux 架构的概念；2015 年，Redux 出现，将 Flux 与函数式编程结合一起，很短时间内就成为了最热门的前端架构。

这里简单介绍一下 React 和 Redux 的使用语法。

<!-- more -->

![react redux logo]({{ site.url }}/images/javascripts/react-redux-logo.png "react redux logo "){: .pull-center width="55%" }

## 简介

一般来说，当需要根据角色判断使用方式、与服务器大量交互 (例如使用 WebSocket)、视图需要从多个来源获取数据，也就是说在交互复杂、多数据源时；或者从组件的角度考虑，如果需要组件的状态广播等时需要使用。

Redux 的设计思想很简单：A) Web 应用是一个状态机，视图与状态是一一对应；B) 所有的状态，保存在一个对象里面。

可以简单将 Redux 理解为是 JavaScript 的状态容器：

1. 应用中所有的状态都是以一个对象树的形式存储在一个单一的 store 中；
2. 当你想要改变应用的中的状态时，你就要 dispatch 一个 action，这也是唯一的改变 state 的方法；
3. 通过编写 reducer 来维护状态，返回新的 state，不直接修改原来数据；


### 工作流

首先简单介绍下 Redux 的工作流程。

![react redux model1]({{ site.url }}/images/javascripts/react-redux-model1.png "react redux model1"){: .pull-center width="70%" }

首先，用户发出 Action 。

```
store.dispatch(action);
```

然后，Store 自动调用 Reducer，并且传入两个参数：当前 State 和收到的 Action；Reducer 会返回新的 State 。

```
let nextState = todoApp(previousState, action);
```

State 一旦有变化，Store 就会调用监听函数。

```
// 设置监听函数
store.subscribe(listener);
```

listener 可以通过 `store.getState()` 得到当前状态，对于 React 此时会触发重新渲染 View。

{% highlight text %}
function listerner() {
	let newState = store.getState();
	component.setState(newState);
}
{% endhighlight %}


### 组件生命周期

![lifetime]({{ site.url }}/images/javascripts/react/react-component-lifetime.png "lifetime"){: .pull-center width="80%" }

从上图所示，React 组件的生命周期可以分为初始化阶段、存在阶段和销毁阶段。

<!--
https://blog.csdn.net/u013982921/article/details/53158941
-->


## 基本概念

### 1. Store

Store 就是保存数据的地方，可以把它看成一个容器，整个应用只能有一个 Store ； Redux 通过提供的 `createStore()` 这个函数来生成 Store 。

{% highlight text %}
import { createStore } from 'redux';
const store = createStore(fn);
{% endhighlight %}

其中 `createStore()` 函数接受另一个函数作为参数，返回新生成的 Store 对象。

### 2. State

Store 对象包含所有数据，如果想得到某个时点的数据，就要对 Store 生成快照，这种时点的数据集合，就叫做 State 。

当前时刻的 State 可以通过 `store.getState()` 拿到。

{% highlight text %}
import { createStore } from 'redux';
const store = createStore(fn);

const state = store.getState();
{% endhighlight %}

Redux 规定，state 和 view 一一对应，一个 State 对应一个 View，只要 State 相同，View 就相同；反之亦然。

### 3. Action

如上所述，State 的变化，会导致 View 的变化；但是，用户接触不到 State，只能接触到 View 。所以，State 的变化必须是 View 导致的，Action 就是 View 发出的通知，表示 State 应该要发生变化了。

Action 是一个对象，其中的 type 属性是必须的，表示 Action 的名称，其它属性可以自由设置，社区有一个 [规范](https://github.com/redux-utilities/flux-standard-action) 可以参考。

{% highlight text %}
const action = {
	type: 'ADD_TODO',
	payload: 'Learn Redux'
};
{% endhighlight %}

上面代码中，Action 的名称是 `ADD_TODO`，它携带的信息是字符串 Learn Redux 。

可以这样理解，Action 描述当前发生的事情，改变 State 的唯一办法，就是使用 Action，它会运送数据到 Store 。

### 4. Action Creator

View 要发送多少种消息，就会有多少种 Action，如果都手写，会很麻烦。可以定义一个函数来生成 Action，这个函数就叫 Action Creator。


{% highlight text %}
const ADD_TODO = '添加 TODO';

function addTodo(text) {
	return {
		type: ADD_TODO,
		text
	}
}

const action = addTodo('Learn Redux');
{% endhighlight %}

上面代码中，`addTodo()` 函数就是一个 Action Creator 。

### 5. store.dispatch()

`store.dispatch()` 是 View 发出 Action 的唯一方法。

{% highlight text %}
import { createStore } from 'redux';
const store = createStore(fn);

store.dispatch({
	type: 'ADD_TODO',
	payload: 'Learn Redux'
});
{% endhighlight %}

上面代码中，`store.dispatch` 接受一个 Action 对象作为参数，将它发送出去。

结合 Action Creator，这段代码可以改写如下。

{% highlight text %}
store.dispatch(addTodo('Learn Redux'));
{% endhighlight %}

### 6. Reducer

Store 收到 Action 以后，必须给出一个新的 State，这样 View 才会发生变化，这种 State 的计算过程就叫做 Reducer 。

Reducer 是一个函数，它接受 Action 和当前 State 作为参数，返回一个新的 State 。

{% highlight text %}
const reducer = function (state, action) {
	return new_state;
};
{% endhighlight %}

整个应用的初始状态，可以作为 State 的默认值，下面是一个实际的例子。

{% highlight text %}
const defaultState = 0;
const reducer = (state = defaultState, action) => {
	switch (action.type) {
	case 'ADD':
		return state + action.payload;
	default:
		return state;
	}
};

const state = reducer(1, {
	type: 'ADD',
	payload: 2
});
{% endhighlight %}

上面代码中，`reducer()` 函数收到名为 ADD 的 Action 以后，就返回一个新的 State，作为加法的计算结果。

实际应用中，`reducer()` 函数不用像上面这样手动调用，`store.dispatch()` 方法会触发 Reducer 的自动执行。为此，Store 需要知道 Reducer 函数，做法就是在生成 Store 的时候，将 Reducer 传入 `createStore()` 方法。

{% highlight text %}
import { createStore } from 'redux';
const store = createStore(reducer);
{% endhighlight %}

上面代码中，`createStore()` 接受 Reducer 作为参数，生成一个新的 Store，以后每当 `store.dispatch()` 发送过来一个新的 Action，就会自动调用 Reducer，得到新的 State。

这个函数之所以被称为 Reducer 是因为它可以作为数组的 reduce 方法的参数。请看下面的例子，一系列 Action 对象按照顺序作为一个数组。

{% highlight text %}
const actions = [
	{ type: 'ADD', payload: 0 },
	{ type: 'ADD', payload: 1 },
	{ type: 'ADD', payload: 2 }
];

const total = actions.reduce(reducer, 0); // 3
{% endhighlight %}

上面代码中，数组 actions 表示依次有三个 Action，分别是加0、加1和加2；数组的 reduce 方法接受 Reducer 函数作为参数，就可以直接得到最终的状态3。

### 7. 纯函数

Reducer 函数最重要的特征是，它是一个纯函数。也就是说，只要是同样的输入，必定得到同样的输出。

纯函数是函数式编程的概念，必须遵守以下一些约束：A) 不得改写参数；B) 不能调用系统 IO 的 API；C) 不能调用 `Date.now()` 或者 `Math.random()` 等不纯的方法，因为每次会得到不一样的结果。

由于 Reducer 是纯函数，就可以保证同样的 State，必定得到同样的 View。但也正因为这一点，Reducer 函数里面不能改变 State，必须返回一个全新的对象，请参考下面的写法。

{% highlight text %}
// State 是一个对象
function reducer(state, action) {
	return Object.assign({}, state, { thingToChange });
	// 或者
	return { ...state, ...newState };
}

// State 是一个数组
function reducer(state, action) {
	return [...state, newItem];
}
{% endhighlight %}

最好把 State 对象设成只读。你没法改变它，要得到新的 State，唯一办法就是生成一个新对象。这样的好处是，任何时候，与某个 View 对应的 State 总是一个不变的对象。

### 8. store.subscribe()

Store 允许用 `store.subscribe()` 设置监听函数，一旦 State 发生变化，就自动执行这个函数。

{% highlight text %}
import { createStore } from 'redux';
const store = createStore(reducer);

store.subscribe(listener);
{% endhighlight %}

显然，只要把 View 的更新函数（对于 React 项目，就是组件的render方法或setState方法）放入listen，就会实现 View 的自动渲染。

`store.subscribe()` 方法返回一个函数，调用这个函数就可以解除监听。

{% highlight text %}
let unsubscribe = store.subscribe(() =>
	console.log(store.getState())
);

unsubscribe();
{% endhighlight %}





## Middleware

Action 发出以后 Reducer 立即算出 State，这叫做同步；Action 发出以后，过一段时间再执行 Reducer，这就是异步。

为了支撑异步操作，先从整个框架的角度看下如何添加这一异步操作：

* Reducer：纯函数，只承担计算 State 的功能，不合适承担其他功能，也承担不了，因为理论上，纯函数不能进行读写操作。
* View：与 State 一一对应，可以看作 State 的视觉层，也不合适承担其他功能。
* Action：存放数据的对象，即消息的载体，只能被别人操作，自己不能进行任何操作。

这样，只有在发送 Action 的这个步骤中，即 `store.dispatch()` 方法，可以添加功能。例如，要添加日志功能，把 Action 和 State 打印出来，可以对 `store.dispatch()` 进行如下改造。

{% highlight text %}
let next = store.dispatch;
store.dispatch = function dispatchAndLog(action) {
	console.log('dispatching', action);
	next(action);
	console.log('next state', store.getState());
}
{% endhighlight %}

上面代码中，对 `store.dispatch()` 进行了重定义，在发送 Action 前后添加了打印功能。

中间件就是一个函数，对 `store.dispatch()` 方法进行了改造，在发出 Action 和执行 Reducer 这两步之间，添加了其他功能。

### 中间件用法

很多常用的中间件都有现成的，只要引用即可，比如，上一节的日志中间件，就有现成的 `redux-logger` 模块。

{% highlight text %}
import { applyMiddleware, createStore } from 'redux';
import createLogger from 'redux-logger';
const logger = createLogger();

const store = createStore(
	reducer,
	applyMiddleware(logger)
);
{% endhighlight %}

如上，`redux-logger` 提供一个生成器 `createLogger`，可以生成日志中间件 logger。然后，将它放在 `applyMiddleware()` 方法之中，传入 `createStore()` 方法，就完成了 `store.dispatch()` 的功能增强。

这里需要注意：1) `createStore()` 可接受整个应用的初始状态作为参数，这样 `applyMiddleware` 就是第三个参数了；2) 中间件的次序有讲究 `applyMiddleware(thunk, promise, logger)` 。

### applyMiddlewares()

它是 Redux 的原生方法，作用是将所有中间件组成一个数组，依次执行，如下是源码。

{% highlight text %}
export default function applyMiddleware(...middlewares) {
	return (createStore) => (reducer, preloadedState, enhancer) => {
		var store = createStore(reducer, preloadedState, enhancer);
		var dispatch = store.dispatch;
		var chain = [];

		var middlewareAPI = {
			getState: store.getState,
			dispatch: (action) => dispatch(action)
		};
		chain = middlewares.map(middleware => middleware(middlewareAPI));
		dispatch = compose(...chain)(store.dispatch);

		return {...store, dispatch}
	}
}
{% endhighlight %}

上面代码中，所有中间件被放进了一个数组 chain，然后嵌套执行，最后执行 `store.dispatch()` 。

这样就可以考虑下异步操作了，同步操作只要发出一种 Action 即可，异步操作的差别是它要发出三种 Action：A) 操作发起时的 Action；B) 操作成功时的 Action；C) 操作失败时的 Action 。

以向服务器取出数据为例，三种 Action 可以有两种不同的写法。

{% highlight text %}
// 写法一：名称相同，参数不同
{ type: 'FETCH_POSTS' }
{ type: 'FETCH_POSTS', status: 'error', error: 'Oops' }
{ type: 'FETCH_POSTS', status: 'success', response: { ... } }

// 写法二：名称不同
{ type: 'FETCH_POSTS_REQUEST' }
{ type: 'FETCH_POSTS_FAILURE', error: 'Oops' }
{ type: 'FETCH_POSTS_SUCCESS', response: { ... } }
{% endhighlight %}

除了 Action 种类不同，异步操作的 State 也要进行改造，反映不同的操作状态，下面是 State 的一个例子。

{% highlight text %}
let state = {
	// ...
	isFetching: true,
	didInvalidate: true,
	lastUpdated: 'xxxxxxx'
};
{% endhighlight %}

上面代码中，State 的属性 `isFetching` 表示是否在抓取数据，`didInvalidate` 表示数据是否过时，`lastUpdated` 表示上一次更新时间。

现在，整个异步操作的思路就很清楚了。

* 操作开始时，送出一个 Action，触发 State 更新为"正在操作"状态，View 重新渲染；
* 操作结束后，再送出一个 Action，触发 State 更新为"操作结束"状态，View 再一次重新渲染。

## React-Redux

`React-Redux` 将所有组件分成两大类：UI 组件 (Presentational Component) 和容器组件 (Container Component)。

UI 组件有以下几个特征。

* 只负责 UI 的呈现，不带有任何业务逻辑
* 没有状态 (即不使用this.state这个变量)
* 所有数据都由参数（this.props）提供
* 不使用任何 Redux 的 API

下面就是一个 UI 组件的例子。

{% highlight text %}
const Title = value => <h1>{value}</h1>;
{% endhighlight %}

因为不含有状态，UI 组件又称为"纯组件"，即它纯函数一样，纯粹由参数决定它的值。

容器组件的特征恰恰相反。

* 负责管理数据和业务逻辑，不负责 UI 的呈现
* 带有内部状态
* 使用 Redux 的 API

总之，只要记住一句话就可以了：UI 组件负责 UI 的呈现，容器组件负责管理数据和逻辑。

React-Redux 规定，所有的 UI 组件都由用户提供，容器组件则是由 React-Redux 自动生成。也就是说，用户负责视觉层，状态管理则是全部交给它。

### 示例

在使用 Counter 示例前，需要安装 browserify 以及 http-server 。

{% highlight text %}
# npm install browserify -g
# npm install http-server -g
{% endhighlight %}

可以参考 [GitHub Redux Example](https://github.com/jackielii/simplest-redux-example) 。

### Connect

React-Redux 提供的 `connect()` 方法用于从 UI 组件生成容器组件。

{% highlight text %}
import { connect } from 'react-redux'
const App = connect()(Counter);
{% endhighlight %}

Counter 是 UI 组件，而 App 就是由 React-Redux 通过 `connect()` 自动生成的容器组件。

不过因为没有定义业务逻辑，上面这个容器组件毫无意义，只是 UI 组件的一个单纯的包装层。为了定义业务逻辑，需要给出下面两方面的信息。

* 输入逻辑：外部的数据 （即state对象）如何转换为 UI 组件的参数
* 输出逻辑：用户发出的动作如何变为 Action 对象，从 UI 组件传出去。

因此，`connect()` 方法的调用为如下：

{% highlight text %}
import { connect } from 'react-redux'

const App = connect(
	mapStateToProps,
	mapDispatchToProps
)(Counter)
{% endhighlight %}

上述的两个入参，前者负责输入逻辑，即将 state 映射到 UI 组件的参数 (props)；后者负责输出逻辑，即将用户对 UI 组件的操作映射成 Action 。

### mapStateToProps()

建立一个从外部的 state 对象到 UI 组件 props 对象的映射关系，该函数会返回一个对象，里面的每个键值对就是一个映射。

{% highlight text %}
function mapStateToProps(state) {
	return {
		value: state.count
	}
}
{% endhighlight %}

如上函数，接受 state 作为参数，返回一个对象；如上的对象有一个 value 属性，代表 UI 组件的同名参数，可以是一个值，也可以是一个函数 (从 state 计算出 value 的值) 。

`mapStateToProps()` 会订阅 Store，每当 state 更新的时候，就会自动执行，重新计算 UI 组件的参数，从而触发 UI 组件的重新渲染。

`connect()` 可以省略 `mapStateToProps` 参数，那样的话，UI 组件就不会订阅 Store，就是说 Store 的更新不会引起 UI 组件的更新。

### mapDispatchToProps()

用来建立 UI 组件的参数到 `store.dispatch()` 方法的映射，定义了哪些用户的操作应该当作 Action，传给 Store，可以是一个函数，也可以是一个对象。

{% highlight text %}
function mapDispatchToProps(dispatch) {
	return {
		onIncreaseClick: () => dispatch(increaseAction)
	}
}
{% endhighlight %}

如上，`mapDispatchToProps` 是一个函数，其入参可以有两个参数 dispatch 和 ownProps (容器组件的 props 对象) 两个参数。

<!--
如果mapDispatchToProps是一个函数，会得到dispatch和ownProps（容器组件的props对象）两个参数。

    const mapDispatchToProps = (
      dispatch,
      ownProps
    ) => {
      return {
        onClick: () => {
          dispatch({
            type: 'SET_VISIBILITY_FILTER',
            filter: ownProps.filter
          });
        }
      };
    }

从上面代码可以看到，mapDispatchToProps作为函数，应该返回一个对象，该对象的每个键值对都是一个映射，定义了 UI 组件的参数怎样发出 Action。

如果mapDispatchToProps是一个对象，它的每个键名也是对应 UI 组件的同名参数，键值应该是一个函数，会被当作 Action creator ，返回的 Action 会由 Redux 自动发出。举例来说，上面的mapDispatchToProps写成对象就是下面这样。

    const mapDispatchToProps = {
      onClick: (filter) => {
        type: 'SET_VISIBILITY_FILTER',
        filter: filter
      };
    }
-->

### Provider 组件

`connect()` 生成容器组件后，需要让容器组件拿到 state 对象，才能生成 UI 组件的参数。

React-Redux 提供 Provider 组件，可以让容器组件拿到 state 。

{% highlight text %}
import { Provider } from 'react-redux'
import { createStore } from 'redux'
import todoApp from './reducers'
import App from './components/App'

let store = createStore(todoApp);

render(
	<Provider store={store}>
		<App />
	</Provider>,
	document.getElementById('root')
)
{% endhighlight %}

上面代码中，Provider 在根组件外面包了一层，这样一来，App 的所有子组件就默认都可以拿到 state 了。


## 示例

在官方的仓库中有很多的示例程序可以参考 [Github redux/examples](https://github.com/reactjs/redux/tree/master/examples)，还有一些比较经典的示例 [Github awesome-redux](https://github.com/xgrommx/awesome-redux) 。

### 计数器

这里是参考 [github jackielii/simplest-redux-example](https://github.com/jackielii/simplest-redux-example)


{% highlight text %}
import React, { Component } from 'react'
import PropTypes from 'prop-types'
import ReactDOM from 'react-dom'
import { createStore } from 'redux'
import { Provider, connect } from 'react-redux'

// React component, 纯UI组件，有两个参数value和onIncreaseClick。
// 前者需要从state计算得到，后者需要向外发出 Action。
class Counter extends Component {
  render() {
    const { value, onIncreaseClick } = this.props
    return (
      <div>
        <span>{value}</span>
        <button onClick={onIncreaseClick}>Increase</button>
      </div>
    )
  }
}

Counter.propTypes = {
  value: PropTypes.number.isRequired,
  onIncreaseClick: PropTypes.func.isRequired
}

// Action Creator
const increaseAction = {
	type: 'increase'
}

// Reducer
function counter(state = { count: 0 }, action) {
  const count = state.count
  switch (action.type) {
    case 'increase':
      return { count: count + 1 }
    default:
      return state
  }
}

// Store
const store = createStore(counter)

// Map Redux state to component props
function mapStateToProps(state) {
  return {
    value: state.count
  }
}

// Map Redux actions to component props
function mapDispatchToProps(dispatch) {
  return {
    onIncreaseClick: () => dispatch(increaseAction)
  }
}

// Connected Component
const App = connect(
  mapStateToProps,
  mapDispatchToProps
)(Counter)

ReactDOM.render(
  <Provider store={store}>
    <App />
  </Provider>,
  document.getElementById('root')
)
{% endhighlight %}

## 参考

可以查看其 [官方文档](https://redux.js.org/)，还有配套的小视频 ([前 30 集](https://egghead.io/courses/getting-started-with-redux)、[后 30 集](https://egghead.io/courses/building-react-applications-with-idiomatic-redux))。

关于前端的模式可以参考 [现代 Web 开发基础与工程实践](https://github.com/wxyyxc1992/Web-Series) 以及 [React 模式](http://sangka-z.com/react-in-patterns-cn/) 。

<!--
Redux 入门教程（三）：React-Redux 的用法
http://www.ruanyifeng.com/blog/2016/09/redux_tutorial_part_three_react-redux.html

React设计模式:深入理解React&Redux原理套路
https://segmentfault.com/a/1190000006112093
-->

{% highlight text %}
{% endhighlight %}
