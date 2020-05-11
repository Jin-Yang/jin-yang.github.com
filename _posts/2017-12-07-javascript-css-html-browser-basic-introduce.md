---
title: 前端基本概念
layout: post
comments: true
language: chinese
category: [program,linux]
keywords: golang,go,etcd
description:
---


<!-- more -->

## 简介

在前端的页面中，核心包括了三部分：

1. HTML 主要包括了文字、图片、链接，其它还有音乐、程序等非文字元素的标记语言；
2. CSS 可以定制化 HTML 中元素显示的样式，从而更加美观；
3. JavaScript 一种解释性脚本语言，可以用来给 HTML 页面增加动态、交互功能。

三者构成了现在比较成熟的前端页面，基本的位置为。

{% highlight html %}
<html>
	<head>
		<!-- 页面标题、序言、说明等，本身不作为内容来显示，但影响网页显示的效果 -->
	</head>
	<body>
		<!-- 显示实际内容 -->
	</body>
</html>
{% endhighlight %}



### 加载顺序

浏览器接收到代码后到最终的页面展示，主要包括了三个步骤：DOM 构造、生成页面布局以及绘制页面。

#### DOM 解析

解析接收到的 HTML 代码，构建为一颗 DOM 树。这棵树包含了各个节点之间的关联关系，例如父子、兄弟关系。同时，当接收到 CSS 之后会同时进行解析，并对应到 DOM 树上，得到一个带有样式属性的 DOM 树。


<!--
#### 生成页面布局

第二步，布局浏览器按从上到下，从左到右的顺序，读取DOM树的文档节点，顺序存放到一条虚拟的传送带上。传送带上的盒子就是节点，而这条流动的传送带就是文档流。如果我们读取到的节点是属于另一个节点下的子节点，那么在放入传送带的时候，就应该按顺序放到该节点盒子的内部。如果子节点下还有子节点，在传送带上的时候就继续套到子一级的盒子内部。根据它在DOM树上的结构，可以嵌套的层级没有限制的哦。文档流排完之后，开始获取计算节点的坐标和大小等CSS属性，作为盒子的包装说明。然后把盒子在仓库里一一摆放，这就将节点布局到了页面。

#### 绘制页面

第三步，绘制页面布局完成之后，我们在页面上其实是看不到任何内容的浏览器只是计算出了每一个节点对象应该被放到页面的哪个位置上，但并没有可视化。因此最后一步就是将所有内容绘制出来，完成整个页面的渲染。道理都讲过了，不如……举个栗子浏览器获得一些代码，它们最终会渲染成这样。第一步是将代码解析为一颗DOM树按从上到下，从左到右的顺序，将树上的节点压入文档流然后布局我们看看从body节点开始然后是body节点下的input节点，label节点，一个接一个的被压入文档流布局的时候因为是行内元素无法占满一行，会在同一行里挨个排列如果调整页面宽度让它变窄，行内元素会因为一行放不下而换行最后将它们渲染就完成了这个页面


①html：贯穿整个页面
②css：三种声明方式
外联样式表：在head便签中 用link标签的href属性来引用后缀名为.css的css样式文件
内联样式表：在head标签下的style标签中，选择器 + 样式声明
内部样式表：在标签的style属性中添加css样式声明
③JavaScript：在<script>标签中，可以在head标签中，也可以在body标签中（区别一会再说）
-->


## 其它

### 块级元素 VS. 行内元素

行内元素只占据它对应标签的边框所包含的空间，只能容纳文本或者其它内联元素；而块级元素会占据其父元素的整个空间，通常浏览器会在块级元素前后另起一行，此时可以容纳其它块元素或者内联元素。

<!--
四、1.常见的块元素：

address - 地址
blockquote - 块引用
center - 举中对齐块
dir - 目录列表
div - 常用块级容易，也是css layout的主要标签
dl - 定义列表
fieldset - form控制组
form - 交互表单
h1 - 大标题
h2 - 副标题
h3 - 3级标题
h4 - 4级标题
h5 - 5级标题
h6 - 6级标题
hr - 水平分隔线
isindex - input prompt
menu - 菜单列表
noframes - frames可选内容（对于不支持frame的浏览器显示此区块内容）
noscript - 可选脚本内容（对于不支持script的浏览器显示此内容）
ol - 排序列表
p - 段落
pre - 格式化文本
table - 表格
ul - 非排序列表
2.常见的内联元素：
a - 锚点
abbr - 缩写
acronym - 首字
b - 粗体（不推荐）
bdo - bidi override
big - 大字体
br - 换行
cite - 引用
code - 计算机代码（在引用源码的时候需要）
dfn - 定义字段
em - 强调
font - 字体设定（不推荐）
i - 斜体
img - 图片
input - 输入框
kbd - 定义键盘文本
label - 表格标签
q - 短引用
s - 中划线（不推荐）
samp - 定义范例计算机代码
select - 项目选择
small - 小字体文本
span - 常用内联容器，定义文本内区块
strike - 中划线
strong - 粗体强调
sub - 下标
sup - 上标
textarea - 多行文本输入框
tt - 电传文本
u - 下划线
var - 定义变量
五、块级元素和内联元素之间的转换：
1.display
块元素默认display:block;行内非替换元素(a,span)默认为display：inline;行内替换元素(input)默认为display:inline-block;
a.display:none;不显示该元素，也不会保留该元素原先占有的文档流位置。
b.display:block;转换为块级元素。
c.display:inline;转换为行内元素。
d.display:inline-block;转换为行内块级元素。
2.float
当把行内元素设置完float:left/right后，该行内元素的display属性会被赋予block值，且拥有浮动特性。行内元素去除了之间的莫名空白。
3.position
当为行内元素进行定位时，position:absolute与position:fixed.都会使得原先的行内元素变为块级元素。

https://www.cnblogs.com/stfei/p/9084915.html
-->

TCP 在建立链接时，因为无法确定真正的带宽是多少，所以会有 TCP Slow Start 启动，也就是最开始只传 14Kb 字节。


### JavaScript

JS 通常会添加到两个位置，一个是在 `<head>` 标签里面，另一个是 `<body>` 标签快结束的地方，也就是 `</body>` 上面。

* CSS 文件的解析与 DOM 解析是可以并行的，不存在相互阻塞；
* JS 解析会阻塞 DOM，主要是因为 JS 可能会修改 DOM 内容，导致需要重新构建 DOM 树；

因为 JS 阻塞 DOM 解析，也就意味着，如果 JS 文件过大，会导致浏览器页面加载过慢，出现假死状态，导致用户体验较差；另外，如果 JS 要操作 DOM 元素，就需要等页面加载完成后在操作，否则可能会出现找不到 DOM 节点。

#### 优化

需要保证 JS 是一个外部脚本，有两种方式进行优化：A) 标签中添加 `defer="ture"` ，表示 JS 与 DOM 并行加载，待页面加载完成后再执行 JS 文件；B) 标签添加 `async="ture"` ，也就是 JS 是异步加载，不依赖其它 JS 和 CSS 等。

如果两者都出现，那么 `defer` 属性会失效，另外，也可以将 `scirpt` 标签放在 `body` 标签之后，这样就不会出现加载的冲突了。

## 参考

* [JavaScript 教程](https://www.runoob.com/js/js-tutorial.html) 在 runoob.com 中的相关教程。
* 关于当前浏览器如何工作的，可以参考 [how browsers work](http://taligarsiel.com/Projects/howbrowserswork1.htm) 以及 [mozilla.org](https://developer.mozilla.org/en-US/docs/Web/Performance/How_browsers_work) 中的介绍。

<!--
加载过程
https://www.cnblogs.com/yuezk/archive/2013/01/11/2855698.html
https://blog.csdn.net/Lonely_Devil/article/details/81153443
-->

{% highlight text %}
{% endhighlight %}
