---
title: Ant Design Pro 使用
layout: post
comments: true
language: chinese
category: [program, react]
keywords: ant design,dva,javascript,react
description:
---

dva 是基于现有应用架构 (redux + react-router + redux-saga 等) 的一层轻量封装，没有引入任何新概念，全部代码不到 100 行。

<!-- more -->

## DVA

每个路由下都有一个 model，这个 model 掌管这个路由的所有状态 (action、state、reducer、sagas)，组件想改变状态 dispatch type 名字就行了。


![dva arch]({{ site.url }}/images/javascripts/react/dva-arch.png "dva arch"){: .pull-center width="80%" }

### Effect

Side Effect 副作用，之所以叫副作用是因为它使得函数变得不纯，同样的输入不一定获得同样的输出，通常是异步操作。底层引入 redux-sagas 做异步流程控制，采用 generator 将异步转成同步写法，从而将 effects 转为纯函数。

sages 采用 Generator 函数来实现，其中 Generator 函数的作用是可以暂停执行，再次执行的时候从上次暂停的地方继续执行，可以通过使用 effects API 如 fork，call，take，put，cancel 等来创建 Effect。


<!---
### 代码解析
https://www.jianshu.com/p/fb75353d805a
https://blog.csdn.net/zp1996323/article/details/73315096/
-->






### 示例

#### 安装启动 DVA-CLI

{% highlight text %}
----- 需要确保版本大于0.7.0
$ npm install dva-cli -g
$ dva -v
0.7.6

----- 创建新应用
$ dva new dva-quickstart

----- 启动程序
$ cd dva-quickstart
$ npm start
{% endhighlight %}

其它的相关示例也可以参考 [DVA Getting Started](https://github.com/dvajs/dva-docs/blob/master/v1/en-us/getting-started.md) 。

此时会生成一个默认的路由 `src/touter.js`，以及起始页面 `src/routes/IndexPage.js` 。

#### 使用 antd 框架

{% highlight text %}
$ npm install antd babel-plugin-import --save
{% endhighlight %}

编辑 dva 的配置文件 `.roadhogrc`，使 `babel-plugin-import` (按需加载) 插件生效。

<!--
"extraBabelPlugins": [
     "transform-runtime",
     ["import", { "libraryName": "antd", "style": "css" }]
],
-->

#### 定义路由

新建 `src/routes/Product.js` 内容如下。

{% highlight text %}
import React from 'react';

const Products = (props) => (
	<h2>List of Products</h2>
);

export default Products;
{% endhighlight %}

添加路由信息到路由表，编辑 `src/router.js` 。

{% highlight text %}
import products from './routes/products';

<Route path="/products" component={products} />
{% endhighlight %}

然后在浏览器里打开 [http://localhost:8000/#/products](http://localhost:8000/#/products)，应该能看到前面定义的 `<h2>` 标签。

#### 新增组件

对于一些公用的模块可以直接提取出来作为组件。

{% highlight text %}
import React from 'react';
import PropTypes from 'prop-types';
import { Table, Popconfirm, Button } from 'antd';

const ProductList = ({ onDelete, products }) => {
  const columns = [{
    title: 'Name',
    dataIndex: 'name',
  }, {
    title: 'Actions',
    render: (text, record) => {
      return (
        <Popconfirm title="Delete?" onConfirm={() => onDelete(record.id)}>
          <Button>Delete</Button>
        </Popconfirm>
      );
    },
  }];
  return (
    <Table
      dataSource={products}
      columns={columns}
    />
  );
};

ProductList.propTypes = {
  onDelete: PropTypes.func.isRequired,
  products: PropTypes.array.isRequired,
};

export default ProductList;
{% endhighlight %}

其中 `propTypes` 用于校验变量的类型。

#### 定义 Model

完成 UI 后开始处理数据和逻辑，dva 通过 model 的概念把一个领域的模型管理起来，包含同步更新 state 的 reducers，处理异步逻辑的 effects，订阅数据源的 subscriptions 。

新建 `src/models/products.js`。

{% highlight text %}
export default {
  namespace: 'products',
  state: [],
  reducers: {
    'delete'(state, { payload: id }) {
      return state.filter(item => item.id !== id);
    },
  },
};
{% endhighlight %}

<!--
其中，

namespace 表示在全局 state 上的 key
state 是初始值，在这里是空数组
reducers 等同于 redux 里的 reducer，接收 action，同步更新 state

然后别忘记在 index.js 里载入他：

// 3. Model
+ app.model(require('./models/products').default);

#### connect

编辑 routes/products.js，替换为以下内容：

import React from 'react';
import { connect } from 'dva';
import List from '../components/list';

const Products = ({ dispatch, products }) => {
  function handleDelete(id) {
    dispatch({
      type: 'products/delete',
      payload: id,
    });
  }
  return (
    <div>
      <h2>List of Products</h2>
      <List onDelete={handleDelete} products={products} />
    </div>
  );
};

// export default Products;
export default connect(({ products }) => ({
  products,
}))(Products);

我们还需要一些初始数据让这个应用 run 起来。编辑 index.js：

-->


## 简介

直接从 [Github Ant-Design-Pro](https://github.com/ant-design/ant-design-pro) 下载，并运行。

{% highlight text %}
$ git clone https://github.com/ant-design/ant-design-pro.git --depth=1
$ cd ant-design-pro
$ npm install
$ npm start         # 访问 http://localhost:8000
{% endhighlight %}

详细的文档可以查看 [Ant Design Getting Started](https://pro.ant.design/docs/getting-started-cn) 。

### 关于 Session

实际上对于 Single Page web Application，SPA 应用来说，最好不要使用 Session ，因为前后分离的客户端是拿不到 Session 的，一般是通过 Token 来实现。

例如，登陆时不要把用户信息写到 session 里，而是把会话信息加密成一个 token 然后返给客户端，客户端持有这个 token ，并在每次的请求接口中带上这个 token ，服务端解析并获取 token 信息。


<!--
客户端持有这个 token ,然后在请求每个接口的时候客户端都要带上这个 token, 然后服务端解析 token, 获取到会话信息.
客户端通过有没有这个 token 来判断用户的登录状态.
客户端可以将 token 存到内存,cookie,或前端数据库
当会话信息超时,或者失效时,请求任何服务端接口都返回 HTTP Status Code 401 , 客户端接收到响应以后主动丢弃 token 或者主动刷新 token
当用户注销时,客户端主动丢弃 token

当然这个 token 有一个比较好的开源实现 JWT

不要用传统的 J2EE 的 B/S 思维去思考问题, 多从 C/S 的角度思考
-->




#### 目录结构

安装完成后的空间结构如下：

{% highlight text %}
├── mock                     # 本地模拟数据
├── public
│   └── favicon.ico          # Favicon
├── src                      # 业务功能代码都保存在该目录下
│   ├── assets               # 本地静态资源
│   ├── common               # 应用公用配置，如导航信息
│   ├── components           # >>>业务通用组件，每个页面由一些组件组成，在routes文件夹中的文件引入
│   ├── e2e                  # 集成测试用例
│   ├── layouts              # 通用布局
│   ├── models               # dva model，用于组件的数据存储，接收请求返回的数据等
│   ├── routes               # >>>业务页面入口和常用模板，定义页面的基本结构和内容
│   │   ├── Dashboard        # 业务页面
│   │   └── Exception        # 异常页面
│   ├── services             # >>>后台接口服务，用于与后台交互、发送请求等
│   ├── utils                # 工具库
│   ├── g2.js                # 可视化图形配置
│   ├── theme.js             # 主题配置
│   ├── index.ejs            # HTML 入口模板
│   ├── index.js             # 应用入口
│   ├── index.less           # 全局样式
│   └── router.js            # 路由入口
├── tests                    # 测试工具
├── README.md
└── package.json
{% endhighlight %}

#### 构建发布

在开发完成之后直接通过 `npm run build` 命令构建即可，完成后会生成一个 `dist` 目录，其底层使用的是 [roadhog](https://github.com/sorrycc/roadhog) 工具。

### 和服务端进行交互

<!--
Ant Design Pro 是一套基于 React 技术栈的单页面应用，我们提供的是前端代码和本地模拟数据的开发模式， 通过 Restful API 的形式和任何技术栈的服务端应用一起工作。下面将简单介绍和服务端交互的基本写法。
-->

一个完整的前端 UI 交互到服务端处理流程如下：

1. UI 组件交互操作；
2. 调用 model 的 effect；
3. 调用统一管理的 service 请求函数；
4. 使用封装的 request.js 发送请求；
5. 获取服务端返回；
6. 然后调用 reducer 改变 state；
7. 更新 model。

从上面的流程可以看出，为了方便管理维护，统一的请求处理都放在 services 文件夹中，并且一般按照 model 维度进行拆分文件，如：

{% highlight text %}
services/
	user.js
	api.js
	...
{% endhighlight %}

其中，`utils/request.js` 是基于 dva fetch 的封装，统一处理 POST、GET 等请求参数，请求头，以及错误提示信息等。

例如在 services 中的一个请求用户信息的例子：

{% highlight text %}
// services/user.js
import request from '../utils/request';

export async function query() {
  return request('/api/users');
}

export async function queryCurrent() {
  return request('/api/currentUser');
}

// models/user.js
import { queryCurrent } from '../services/user';
...
effects: {
  *fetch({ payload }, { call, put }) {
    ...
    const response = yield call(queryUsers);
    ...
  },
}
{% endhighlight %}

### Effect 处理异步请求

在处理复杂的异步请求的时候，很容易让逻辑混乱，陷入嵌套陷阱，所以 Ant Design Pro 的底层基础框架 dva 使用 effect 的方式来管理同步化异步请求。

<!--

effects: {
  *fetch({ payload }, { call, put }) {
    yield put({
      type: 'changeLoading',
      payload: true,
    });
    // 异步请求 1
    const response = yield call(queryFakeList, payload);
    yield put({
      type: 'save',
      payload: response,
    });
    // 异步请求 2
    const response2 = yield call(queryFakeList2, payload);
    yield put({
      type: 'save2',
      payload: response2,
    });
    yield put({
      type: 'changeLoading',
      payload: false,
    });
  },
},

通过 generator 和 yield 使得异步调用的逻辑处理跟同步一样，更多可参看 dva async logic。
从 mock 直接切换到服务端请求#

通常来讲只要 mock 的接口和真实的服务端接口保持一致，那么只需要重定向 mock 到对应的服务端接口即可。

// .roadhogrc.mock.js
export default {
  'GET /api/(.*)': 'https://your.server.com/api/',
};

这样你浏览器里这样的接口 http://localhost:8001/api/applications 会被反向代理到 https://your.server.com/api/applications 下。
关闭 mock#

关闭 mock 的方法我们推荐采用环境变量，你可以在 package.json 中设置：

"script" : {
  "start": "roadhog server",
  "start:no-proxy": "cross-env NO_PROXY=true roadhog server"
}

然后在 .roadhogrc.mock.js 中做个判断即可：

const noProxy = process.env.NO_PROXY === 'true';
...
export default noProxy ? {} : delay(proxy, 1000);
-->


### Mock

用于模拟接口返回的数据，项目的根目录下有个 `.roadhogrc.mock.js` 的文件，详细可以查看官方的 [说明介绍](https://github.com/sorrycc/roadhog#mock) 。

示例中的很多数据都是通过 Mock 模拟的。

<!--
export default {
  // 支持值为 Object 和 Array
  'GET /api/users': { users: [1,2] },

  // GET POST 可省略
  '/api/users/1': { id: 1 },

  // 支持自定义函数，API 参考 express@4
  'POST /api/users/create': (req, res) => { res.end('OK'); },

  // Forward 到另一个服务器
  'GET /assets/*': 'https://assets.online/',

  // Forward 到另一个服务器，并指定子路径
  // 请求 /someDir/0.0.50/index.css 会被代理到 https://g.alicdn.com/tb-page/taobao-home, 实际返回 https://g.alicdn.com/tb-page/taobao-home/0.0.50/index.css
  'GET /someDir/(.*)': 'https://g.alicdn.com/tb-page/taobao-home',
};
-->

dva 是整个项目的发动机，可参考 [Github DVA](https://github.com/dvajs/dva) 以及 [dva.js 知识导图](https://github.com/dvajs/dva-knowledgemap) 。


## 文件结构

### 初始化DVA

起始文件为 `src/index.js` ，初始化部分比较简单，首先一个个来说明一下。

{% highlight text %}
import '@babel/polyfill';
import 'url-polyfill';
import dva from 'dva';

import createHistory from 'history/createHashHistory';
// user BrowserHistory
// import createHistory from 'history/createBrowserHistory';
import createLoading from 'dva-loading';
import 'moment/locale/zh-cn';
import './rollbar';

import './index.less';
// 1. Initialize
const app = dva({
  history: createHistory(),
});

// 2. Plugins
app.use(createLoading());

// 3. Register global model
app.model(require('./models/global').default);

// 4. Router
app.router(require('./router').default);

// 5. Start
app.start('#root');

export default app._store; // eslint-disable-line
{% endhighlight %}


### 发送数据请求

在发送请求数据时，实际上时通过如下函数。

{% highlight text %}
dispatch( {type: 'app/login', payload: value })
{% endhighlight %}

dispatch 会根据设置的 type 内容，转发到指定的 model ，也就是说，需要确保 model 的设置正确才可以。

其中，`namespace: 'app'` 以及对应的 `effects`，也就是 login 。

### Model 部分

一般分为了三部分：A) reducers 处理数据；B) effects 接收数据；C) subscriptions 监听数据。

#### Reducers

以 Key/Value 格式定义，用于处理同步操作，唯一可以修改 state 的地方，由 action 触发。

格式为 `(state, action) => newState` 或者`[(state, action) => newState, enhancer]` 。

#### Effects

以 Key/Value 格式定义，用于处理异步操作和业务逻辑，不直接修改 state ，由 action 触发，可以触发 action ，可以和服务器交互，可以获取全局 state 的数据等等。

格式为 `*(action, effects) => void` 或者 `[*(action, effects) => void, { type }]` ，其中 `type` 有 `takeEvery`、`takeLatest`、`throttle`、`watcher` 。

该函数内部使用的大概有如下几个：

* put  用来发起一条action
* call 以异步的方式调用函数
* select 从state中获取相关的数据
* take 获取发送的数据

以及封装的函数 takeEvery、takeLatest、throttle、watcher 。

当使用 put 发送一条 action 的时候，与之对于的 reducers 就会接收到这个消息，然后在里面返回 state 等数据。


#### Subscriptions

同样是以 Key/Value 定义，用来订阅一个数据源，然后根据需要 dispatch 相应的 action，在 `app.start()` 时被执行，数据源可以是当前的时间、服务器的 WebSocket 链接、KeyBoard 的输入、GeoLocation 变化、History 变化等。

格式为 `({ dispatch, history ), done) => unlistenFucntion` 。





## 开发

在 Ant Design Pro 中，前端路由是通过 `react-router` 进行路由管理，接下来将会添加一个新的路由，并在前端页面中增加一个菜单来对应该路由。

### 1. 添加导航

首先在侧边栏中，也就是对应 `src/common/menu.js` 文件，添加对应的导航页。

{% highlight text %}
const menuData = [
  {
    name: 'TEST',
    icon: 'gift',
    path: 'test',
    children: [
      {
        name: '测试页',
        path: 'test-page',
      },
    ],
  },
]
{% endhighlight %}

这里的图标可以从 [icon](https://ant.design/components/icon-cn/) 中获取，此时添加完后，如果直接访问会显示 404 。

### 2. 新增路由

路由的配置文件统一由 `src/common/router.js` 文件进行管理，示例内容如下：

{% highlight text %}
const routerConfig = {
  '/test/test-page': {
    component: dynamicWrapper(app, ['chart'], () => import('../routes/Test/Blank')),
  },
}
{% endhighlight %}

其中包含了三个路由及其对应的页面文件。

### 3. 创建一个页面

继续创建一个页面，对应到我们添加的路由中，新增一个 `src/routes/Test/Blank.js` 文件。

{% highlight text %}
import React, { PureComponent } from 'react';
export default class Workplace extends PureComponent {
  render() {
    return (
      <h1>Hello World!</h1>
    );
  }
}
{% endhighlight %}

### 4. 新增一个组件

一般这一步可以直接省略。

一些通用的组件可以添加到 components 文件夹下，新增 `components/ImageWrapper/index.js` ，同样是一个示例，并在 `Blank.js` 中引入使用。

{% highlight text %}
// components/ImagesWrapper/index.less
.imageWrapper {
  img {
    width: 40%;
  }
}

// components/ImagesWrapper/index.js
import React from 'react';
import styles from './index.less';    // 按照 CSS Modules 的方式引入样式文件。

export default ({ src, desc, style }) => (
  <div style={style} className={styles.imageWrapper}>
    <img className={styles.img} src={src} alt={desc} />
    {desc && <div classname={styles.desc}>{desc}</div>}
  </div>
);
{% endhighlight %}

然后修改 `Blank.js` 。

{% highlight text %}
import React from 'react';
import ImageWrapper from '../../components/ImageWrapper';  // 注意保证引用路径的正确
export default () => (
  <imagewrapper src="https://os.alipayobjects.com/rmsportal/mgesTPFxodmIwpi.png" desc="示意图">;
)
{% endhighlight %}

假设我们的 `Blank.js` 页面需要发送请求，接收数据并在页面渲染时使用接收到的数据。

例如，我们可以在组件加载时发送一个请求来获取数据。

{% highlight text %}
componentDidMount() {
  const { dispatch } = this.props;
  dispatch({
    type: 'project/fetchNotice',
  });
  dispatch({
    type: 'activities/fetchList',
  });
  dispatch({
    type: 'chart/fetch',
  });
}
{% endhighlight %}


### 5. 增加 Service 和 Module

接上所述，此时，它将会找到对应的models中的函数来发送请求：

{% highlight text %}
export default {
  namespace: 'monitor',
  state: {
    currentServices: [],
    waitingServices: [],
  },
  effects: {
    *get_current_service_list(_, { call, put }) {
      const response = yield call(getCurrentServiceList);
      yield put({
        type: 'currentServiceList',
        currentServices: response.result,
      });
    },
    *get_waiting_service_list(_, { call, put }) {
      const response = yield call(getWaitingServiceList);
      yield put({
        type: 'waitingServiceList',
        waitingServices: response.result,
      });
    },
  },
  reducers: {
    currentServiceList(state, action) {
      return {
        ...state,
        currentServices: action.currentServices,
      };
    },
    waitingServiceList(state, action) {
      return {
        ...state,
        waitingServices: action.waitingServices,
      };
    },
  },
};
{% endhighlight %}

而真正发送请求的实际是service文件夹下的文件进行的。

{% highlight text %}
export async function getWaitingServiceList() {
  return request('/api/get_service_list', {
    method: 'POST',
    body: {"type": "waiting"},
  });
}
export async function getCurrentServiceList() {
  return request('/api/get_service_list', {
    method: 'POST',
    body: {"type": "current"},
  });
}
{% endhighlight %}

<!--
在routes文件夹中，我们可以直接在render部分使用应该得到的返回值进行渲染显示：

    const { monitor, loading } = this.props;
    // monitor指的是相当于数据流
    const { currentServices, waitingServices } = monitor;
    // 从数据流中取出了具体的变量

### Mock数据

在我们开发的过程中，通常不可避免的就是mock数据。
那具体应该如下进行mock数据呢？主要依赖的是.roadhogrc.mock.js文件。
打开指定文件，我们可以看到如下的类似内容：

    const proxy = {
      'GET /api/fake_list': [{
        key: '1',
        name: 'John Brown',
        age: 32,
        address: 'New York No. 1 Lake Park',
      }, {
        key: '2',
        name: 'Jim Green',
        age: 42,
        address: 'London No. 1 Lake Park',
      }, {
        key: '3',
        name: 'Joe Black',
        age: 32,
        address: 'Sidney No. 1 Lake Park',
      }],
      'POST /api/login/account': (req, res) => {
        const { password, userName, type } = req.body;
        if(password === '888888' &amp;&amp; userName === 'admin'){
          res.send({
            status: 'ok',
            type,
            currentAuthority: 'admin'
          });
          return ;
        }
        if(password === '123456' &amp;&amp; userName === 'user'){
          res.send({
            status: 'ok',
            type,
            currentAuthority: 'user'
          });
          return ;
        }
        res.send({
          status: 'error',
          type,
          currentAuthority: 'guest'
        });
      }
    }

在上面的例子中，我们分别描述了针对GET和POST请求应该如下进行数据Mock。
打包

当我们将完整的前端项目开发完成后，我们需要打包成为静态文件并准备上线。
此时，我们应该如何打包项目呢？
非常简单，只需要执行如下命令即可：

    npm run build

此外，有时我们希望分析依赖模块的文件大小来优化我们的项目：

    npm run analyze

title

运行完成后，我们可以打开dist/stats.html文件来查询具体内容：
-->


## 其它

![dva connect]({{ site.url }}/images/javascripts/react/dva-connect.png "dva connect"){: .pull-center width="80%" }


### 菜单 路由 组件

#### 菜单

在左侧的导航栏点击 `列表页 > 标准列表` 后，可以进入到所对应的页面，其中导航栏的内容在 `src/common/menu.js` 中。

{% highlight text %}
{
  name: '列表页',
  icon: 'table',
  path: 'list',
  children: [
    {
      name: '查询表格',
      path: 'table-list',
    },
    {
      name: '标准列表',
      path: 'basic-list',
    },
   //……
  ],
},
{% endhighlight %}

#### 路由

全局的路由关系是在 `src/index.js` 中通过 `app.router(require('./router').default);`，将 `src/router.js` 绑定到 dva 实例的 router 方法上。

而在 `src/router.js` 中又引入了 `src/common/router.js` 中的 `getRouterData` 作为数据源；也就相当于 `src/common/menu.js` 中 path 所指向的路径对应于 `src/common/router.js` 中的路由记录。

{% highlight text %}
export const getRouterData = (app) => {
  const routerConfig = {
    ...,
    '/list/basic-list': {
      component: dynamicWrapper(app, ['list'], () => import('../routes/List/BasicList')),
    },
    ...,
  };
  ...
}
{% endhighlight %}

这里调用了 lazy-loading 的动态加载函数 `dynamicWrapper()`，有 3 个参数，app 为全局 dva 实例，models 为一个带有相关 dva Model 的 Array，component 即为该路由记录对应的实际组件。

{% highlight text %}
const dynamicWrapper = (app, models, component) => {...};
{% endhighlight %}

可以看到，加载路由的时候会动态加载当前文件下的 model 文件，也就是对应文件下的 `src/models/list.js` 。

#### 组件

在 UI 的实现 `src/routes/List/BasicList.js` 可以看到如下的省略后的代码：

{% highlight text %}
import React, { PureComponent } from 'react';
import { connect } from 'dva';
import PageHeaderLayout from '../../layouts/PageHeaderLayout';

@connect(({ list, loading }) => ({
  list,
  loading: loading.models.list,
}))
export default class BasicList extends PureComponent {
  componentDidMount() {
    this.props.dispatch({
      type: 'list/fetch',
      payload: {
        count: 5,
      },
    });
  }

  render() {
    return (
      <PageHeaderLayout>{/* ...  */}</PageHeaderLayout>
    );
  }
}
{% endhighlight %}

### @connect 装饰器

在组件中，使用了 dva 所封装的 react-redux 的 `@connect` 装饰器，用来接收绑定的 list 这个 model 对应的 redux store。

这里的装饰器实际除了 `app.state.list` 以外还实际接收 `app.state.loading` 作为参数，这个 loading 的来源是 `src/index.js` 中调用的 dva-loading 这个插件。

{% highlight text %}
/*
* src/index.js
*/
import createLoading from 'dva-loading';
app.use(createLoading());
{% endhighlight %}

它返回的信息包含了 global、model 和 effect 的异步加载完成情况。

<!--
数据map一
{
  "global": true,
  "models": {
    "list": false,
    "user": true,
    "rule": false
  },
  "effects": {
    "list/fetch": false,
    "user/fetchCurrent": true,
    "rule/fetch": false
  }
}
-->

在 UI 页面的请求 `{count: 5}` 这个 payload 向 store 进行了一个类型为 `list/fetch` 的 dispatch，在 `src/models/list.js` 中就可以找到具体的对应操作。 

{% highlight text %}
import { queryFakeList } from '../services/api';

export default {
  namespace: 'list',

  state: {
    list: [],
  },

  effects: {
    *fetch({ payload }, { call, put }) {
      const response = yield call(queryFakeList, payload);
      yield put({
        type: 'queryList',
        payload: Array.isArray(response) ? response : [],
      });
    },
    /* ... */
  },

  reducers: {
    queryList(state, action) {
      return {
        ...state,
        list: action.payload,
      };
    },
    /* ... */
  },
};
{% endhighlight %}

在使用 connect 时，有两个参数 `mapStateToProps` 以及 `mapDispatchToProps`，分别将状态绑定到组件的 props 以及将方法绑定到组件的 props 。

{% highlight text %}
@connect(({ list, loading }) => ({
  list,                           // 1
  loading: loading.models.list,   // 2
}))
{% endhighlight %}

1. 将实体 list 中的 state 数据绑定到 props； <!-- 注意绑定的是实体list整体，使用时需要list.[state中的具体变量] -->
2. 通过 loading 将上文 <!-- “数据map一”中的models的list的key对应的value读取出来。赋值给loading，以方便使用，如表格是否有加载图标 -->

<!--
当然代码②也可以通过key value编写：loading.effects["list/fetch"]
-->

#### 变量获取

因为，在 `src/models/list.js` 中，有如下定义。

{% highlight text %}
export default {
  namespace: 'list',

  state: {
    list: [],
  },
{% endhighlight %}

故在 view 中使用

{% highlight text %}
  render() {
    const { list: { list }, loading } = this.props;
{% endhighlight %}

<!--
定义使用时：list: { list }  ，含义实体list下的state类型的list变量

https://www.cnblogs.com/bjlhx/p/9009056.html
https://segmentfault.com/q/1010000012254134/a-1020000012258002
-->

## 参考

关于 dva 文档详细可以查看 [github dva-docs](https://github.com/dvajs/dva-docs) 以及示例中的代码 [User Dashboard](https://github.com/dvajs/dva/tree/master/examples/user-dashboard) 。

官方网站可以参考 [Ant Desgin](https://ant.design/index-cn) 。

<!--
另外可以参考如下的示例
https://github.com/zuiidea/antd-admin

https://github.com/egliu/Flask-React-antd

dva示例
https://github.com/sorrycc/blog/issues/18

12 步 30 分钟，完成用户管理的 CURD 应用 (react+dva+antd)
https://github.com/dvajs/dva-example-user-dashboard

https://www.missshi.cn/api/view/blog/5ab755dd22890966e2000003
-->

{% highlight text %}
{% endhighlight %}
