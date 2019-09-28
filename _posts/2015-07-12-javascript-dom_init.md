---
Date: October 19, 2013
title: Javascript DOM 操作
layout: post
comments: true
language: chinese
category: [misc]
---


<!-- more -->

# 简介

通过 JavaScript，添加、移除、改变或重排页面上的项目，甚至可以重构整个 HTML 文档。如果要改变某个元素，就需要有一个对 HTML 文档中所有元素进行访问的入口，这个入口都是通过 DOM 操作的。

根据 DOM，HTML 文档中的每个成分都是一个节点：A) 整个文档是一个文档节点；B) 每个 HTML 标签是一个元素节点；C) 包含在 HTML 元素中的文本是文本节点；D) 每一个 HTML 属性是一个属性节点；E) 注释属于注释节点。

各个节点之间彼此都有等级关系，HTML 文档中的所有节点组成了一个文档树或节点树。

![DOM tree]({{ site.url }}/images/webserver/dom-tree.jpg){: .pull-center width="500"}


# Javascript 操作 DOM

## 获取节点元素

### getElementById()

根据 ID 获取元素节点 。

{% highlight html %}
<script language="JavaScript">
function getMethod1() {
    console.log(document.getElementById("p1").innerHTML)
}
</script>
<div id="div1">
  <span id="p1" class="class1">我是第一个P</span><br>
  <span id="p2" class="class2">我是第二个P</span>
</div>
<input type="button" name="mybutton" value="按钮1" onClick="getMethod1()">

<!--OUTPUT:
我是第一个P
-->
{% endhighlight %}


### document.getElementsByName()

根据 name 获取元素节点。

{% highlight html %}
<script language="JavaScript">
function getMethod2() {
    console.log(document.getElementsByName("userName")[0].value);
}
</script>
<input type="text" value="请输入值1" name="userName" />
<input type="text" value="请输入值2" name="userName" />
<input type="button" name="mybutton" value="按钮2" onClick="getMethod2()">

<!--OUTPUT:
请输入值1
-->
{% endhighlight %}


### document.getElementsByTagName()

根据 HTML 标签名获取元素节点，注意 getElementsXXX() 的选择器返回的是一个 NodeList 对象，能根据索引号选择其中 1 个，可以遍历输出。

{% highlight html %}
<script language="JavaScript">
function getMethod3() {
    // 获取某个元素
    console.log(document.getElementsByTagName("span2")[1].innerHTML);

    // 遍厉获取全部元素
    var arr = document.getElementsByTagName("span2");
    for (var i = 0; i < arr.length; i++) {
        console.log(arr[i].innerHTML);
    }

    var node = document.getElementById("div2");
    console.log(node.getElementsByTagName("span2")[1].innerHTML);    //从获取到的元素再获取
}
</script>
<div id="div2">
  <span2 id="p1">我是第一个SPAN</span2><br>
  <span2 id="p2">我是第二个SPAN</span2>
</div>
<input type="button" name="mybutton" value="按钮3" onClick="getMethod3()">

<!--OUTPUT:
我是第二个SPAN
我是第一个SPAN
我是第二个SPAN
我是第二个SPAN
-->
{% endhighlight %}

### document.getElementsByClassName()

根据 class 获取元素节点。

{% highlight html %}
<script language="JavaScript">
function getMethod4() {
    console.log(document.getElementsByClassName("class1")[0].innerHTML);
}
</script>
<div id="div1">
  <span id="p1" class="class1">我是第一个CLASS</span><br>
  <span id="p2">我是第二个CLASS</span>
</div>
<input type="button" name="mybutton" value="按钮4" onClick="getMethod4()">

<!--OUTPUT:
我是第一个CLASS
-->
{% endhighlight %}


### document.querySelector()/querySelectorAll()

Javascript 中的 CSS 选择器，前者会根据 CSS 选择器的规则，返回第一个匹配到的元素；后者会返回所有匹配到的元素。

{% highlight html %}
<script language="JavaScript">
function getMethod5() {
    console.log(document.querySelector("#div3 > span").innerHTML);
    console.log(document.querySelector(".class3").innerHTML);

    var nodelist = document.querySelectorAll("span3");
    console.log(nodelist[0].innerHTML + " - " + nodelist[1].innerHTML);    //输出  我是第一个P - 我是第二个P
}
</script>
<div id="div3">
  <span3 class="class3">我是第一个SLECTOR</span3><br>
  <span3 >我是第二个SLECTOR</span3>
</div>
<input type="button" name="mybutton" value="按钮5" onClick="getMethod5()">

<!-- OUTPUT:
我是第一个SLECTOR
我是第一个SLECTOP
我是第一个SLECTOP - 我是第二个SLECTOR
-->
{% endhighlight %}


### 文档结构和遍历(节点树)

{% highlight text %}
----- 作为节点树的文档，如下6个方法连元素节点也算一个节点
    1 parentNode        获取该节点的父节点
    2 childNodes        获取该节点的子节点数组
    3 firstChild        获取该节点的第一个子节点
    4 lastChild         获取该节点的最后一个子节点
    5 nextSibling       获取该节点的下一个兄弟元素
    6 previoursSibling  获取该节点的上一个兄弟元素
    7 nodeType          节点的类型，9代表Document节点，1代表Element节点，3代表Text节点，8代表Comment节点，11代表DocumentFragment节点
    8 nodeVlue          Text节点或Comment节点的文本内容
    9 nodeName          元素的标签名(如P,SPAN,#text(文本节点),DIV)，以大写形式表示
{% endhighlight %}

测试如下：

{% highlight html %}
<script language="JavaScript">
function getMethod6() {
    var node = document.querySelector(".class4"); // 输出父节点所包含的内容
    console.log(node.parentNode.innerHTML);

    var nodelist = document.getElementById("div4");
    var arr = nodelist.childNodes;
    //console.log(nodelist.childNodes);
    console.log(arr[1].innerHTML + " - " + arr[4].innerHTML);
}
</script>
<div id="div4">
  <span4 class="class4">我是第一个SLECTOR</span4><br>
  <span4 >我是第二个SLECTOR</span4>
</div>
<input type="button" name="mybutton" value="按钮6" onClick="getMethod6()">

<!--OUTPUT:
  <span4 class="class4">我是第一个SLECTOR</span4><br>
  <span4>我是第二个SLECTOR</span4>
我是第一个SLECTOR - 我是第二个SLECTOR
-->
{% endhighlight %}

文档、元素、属性以及 HTML 或 XML 文档的其他方面拥有不同的节点类型，存在 12 种不同的节点类型。

{% highlight html %}
<script language="JavaScript">
function getMethod7() {
    var node = document.getElementById("div5");
    for (var i = 0; i < node.childNodes.length; i++) {
        if (node.childNodes[i].nodeType == 1) {
            console.log(node.childNodes[i].innerHTML);
        } else if (node.childNodes[i].nodeType == 3) {
            console.log(node.childNodes[i].nodeValue);
        }
    }
}
</script>
<div id="div5">
  文本1
  <span>我是第一个SLECTOR</span>
  文本2
  <span>我是第二个SLECTOR</span>
  文本3
</div>
<input type="button" name="mybutton" value="按钮7" onClick="getMethod7()">

<!--OUTPUT:
  文本1
我是第一个SLECTOR
  文本2
我是第二个SLECTOR
  文本3
-->
{% endhighlight %}




### 文档结构和遍历(元素树)

此 5 个方法文本节点不算进去。

{% highlight html %}
    1 firstElementChild        第一个子元素节点
    2 lastElementChild         最后一个子元素节点
    3 nextElementSibling       下一个兄弟元素节点
    4 previousElementSibling   前一个兄弟元素节点
    5 childElementCount        子元素节点个数量
{% endhighlight %}

示例如下：

{% highlight html %}
<script language="JavaScript">
function getMethod8() {
    var node = document.getElementById("div6");
    var node1 = node.firstElementChild;
    var node2 = node.lastElementChild;

    console.log(node.childElementCount);
    console.log(node1.innerHTML);
    console.log(node2.innerHTML);
    console.log(node2.previousElementSibling.innerHTML);
    console.log(node1.nextElementSibling.innerHTML);
}
</script>
<div id="div6">
    <p>我是第一个P</p>
    <p>我是第二个P</p>
    <p>我是第三个P</p>
</div>
<input type="button" name="mybutton" value="按钮8" onClick="getMethod8()">

<!--OUTPUT:
 3
我是第一个P
我是第三个P
我是第二个P
我是第二个P
-->
{% endhighlight %}



## 操作 HTML 属性


### 属性的读取

此处要注意的是，某些 HTML 属性名称在 javascript 之中是保留字，因此会有些许不同，如 class、lable 中的 for 在 javascript 中变为 htmlFor、className。

{% highlight html %}
<script language="JavaScript">
function getMethod9() {
    console.log(document.getElementById("input1").value);
    console.log(document.getElementById("img1").alt);
    console.log(document.getElementById("p11").className);
}
</script>
<div id="div6">
    <input type="text" value="我是一个文本框" id="input1" />
    <img src="123.jpg" alt="我是一张图片" id="img1" />
    <p id="p11" class="class1"> 我是第一个P</p>
</div>
<input type="button" name="mybutton" value="按钮9" onClick="getMethod9()">
{% endhighlight %}


### 属性的设置

此处同样要注意的是保留字，如下实现的效果是，当点击图片时变小。

{% highlight html %}
<script language="JavaScript">
function getMethod10() {
    document.getElementById("img1").src = "1small.jpg";
}
</script>
<div>
    <img src="1big.jpg" alt="我是一张图片" id="img11" onclick="fun1()" />
</div>
{% endhighlight %}

### 非标准HTML属性

分别使用 getAttribute()、setAttribute()，这两个方法是不必理会 javascript 保留字的，HTML 属性是什么就怎么写。

{% highlight html %}
<script language="JavaScript">
function getMethod12() {
    document.getElementById("img12").setAttribute("src", "1small.jpg");
    console.log(document.getElementById("img12").getAttribute("class"));
}
</script>
<div>
    <img src="1big.jpg" alt="我是一张图片" class="imgClass" id="img12" onclick="getMethod12()" />
</div>
{% endhighlight %}



### Attr节点的属性

attributes 属性，对于非 Element 对象返回 NULL，Element 一般返回 Attr 对象。Attr 对象是一个特殊的 Node，通过 name 与 value 获取属性名称与值。


{% highlight html %}
<script language="JavaScript">
function getMethod13() {
     // 通过索引器访问是写在右面在排前面，从0开始
    console.log(document.getElementById("img13").attributes[0].name);
    console.log(document.getElementById("img13").attributes.src.value);
    document.getElementById("img13").attributes.src.value = "1small.jpg";
}
</script>
<div>
    <img src="1big.jpg" alt="我是一张图片" class="imgClass" id="img13" onclick="getMethod13()" />
</div>
{% endhighlight %}



## 元素的内容

对于 innerText、textContent 的区别，当文本为空时，innerText 是 ""，而 textContent 是 undefined 。

{% highlight html %}
<script language="JavaScript">
function getMethod14() {
    console.log(document.getElementById("p14").innerText);
    console.log(document.getElementById("p14").textContent);
    console.log(document.getElementById("p15").textContent);
    // innerHTML与innerText的区别，就是对HTML代码的输出方式Text不会输出HTML代码
    console.log(document.getElementById("p15").innerHTML);
    document.getElementById("p14").textContent = "我是p14，javascript改变了我";    //设置文档Text
}
</script>
<div id="div1">
    <p id="p14">我是第一个CONTEXT</p>
    <p id="p15">我是第<b>二</b>个CONTEXT</p>
</div>
<input type="button" name="mybutton" value="按钮14" onClick="getMethod14()">
{% endhighlight %}

## 创建、插入节点

###  document.createTextNode()

创建一个文本节点。

{% highlight html %}
<script language="JavaScript">
function getMethod15() {
    var textNode = document.createTextNode("<p>我是一个javascript新建的节点</p>");
    document.getElementById("div15").appendChild(textNode);
}
</script>
<div id="div15">
    <p id="p151">我是第一个CONTEXT</p>
    <p id="p152">我是第二个CONTEXT</p>
</div>
<input type="button" name="mybutton" value="按钮15" onClick="getMethod15()">
{% endhighlight %}

完成后 HTML 变为：

{% highlight html %}
<div id="div15">
    <p id="p151">我是第一个P</p>
    <p id="p152">我是第二个P</p>
    &lt;p&gt;我是一个javascript新建的节点&lt;/p&gt;
</div>
{% endhighlight %}

### document.createElement()

创建一个元素节点。

{% highlight html %}
<script language="JavaScript">
function getMethod16() {
    var pNode = document.createElement("p");
    pNode.textContent = "新建一个P节点";
    document.getElementById("div16").appendChild(pNode);
}
</script>
<div id="div16">
    <p id="p161">我是第一个P</p>
    <p id="p162">我是第二个P</p>
</div>
<input type="button" name="mybutton" value="按钮16" onClick="getMethod16()">
{% endhighlight %}

执行之后 HTML 代码变为：

{% highlight html %}
<div id="div16">
    <p id="p161">我是第一个P</p>
    <p id="p161">我是第二个P</p>
    <p>新建一个P节点</p>
</div>
{% endhighlight %}

### 插入节点

appendChild()、insertBefore()，其中前者将一个节点插入到调用节点的最后面；后者接受两个参数，第一个为待插入的节点，第二个指明在哪个节点前面，如果不传入第二个参数，则跟 appendChild() 一样，放在最后。

{% highlight html %}
<script language="JavaScript">
function getMethod17() {
    var pNode1 = document.createElement("p");
    pNode1.textContent = "insertBefore插入的节点";
    var pNode2 = document.createElement("p");
    pNode2.textContent = "appendChild插入的节点";
    document.getElementById("div17").appendChild(pNode2);
    document.getElementById("div17").insertBefore(pNode1,document.getElementById("p171"));
}
</script>
<div id="div17">
    <p id="p171">我是第一个P</p>
    <p id="p172">我是第二个P</p>
</div>
<input type="button" name="mybutton" value="按钮17" onClick="getMethod17()">
{% endhighlight %}

执行之后HTML代码为：

{% highlight html %}
<div id="div17">
    <p>insertBefore插入的节点</p>
    <p id="p171">我是第一个P</p>
    <p id="p172">我是第二个P</p>
    <p>appendChild插入的节点</p>
</div>
{% endhighlight %}



## 删除节点

removeChild(); 由父元素调用，删除一个子节点。注意是直接父元素调用，删除直接子元素才有效，删除孙子元素就没有效果了。

{% highlight html %}
<script language="JavaScript">
function getMethod18() {
    var div1 = document.getElementById("div18");
    div1.removeChild(document.getElementById("p182"));
}
</script>
<div id="div18">
    <p id="p181">我是第一个P</p>
    <p id="p182">我是第二个P</p>
</div>
<input type="button" name="mybutton" value="按钮18" onClick="getMethod18()">
{% endhighlight %}

执行之后代码变为：

{% highlight html %}
<div id="div18">
    <p id="p182">我是第一个P</p>
</div>
{% endhighlight %}

## 替换节点

replaceChild(); 删除一个子节点，并用一个新节点代替它，第一个参数为新建的节点，第二个节点为被替换的节点

{% highlight html %}
<script language="JavaScript">
function getMethod19() {
    var div1 = document.getElementById("div19");
    var span1 = document.createElement("span");
    span1.textContent = "我是一个新建的span";
    div1.replaceChild(span1,document.getElementById("p192"));
}
</script>
<div id="div19">
    <p id="p191">我是第一个P</p>
    <p id="p192">我是第二个P</p>
</div>
<input type="button" name="mybutton" value="按钮19" onClick="getMethod19()">
{% endhighlight %}

执行完成后 HTML 代码变为：

{% highlight html %}
<div id="div1">
    <p id="p1">我是第一个P</p>
    <span>我是一个新建的span</span>
</div>
{% endhighlight %}


## 操作元素CSS

通过元素的 style 属性可以随意读取和设置元素的 CSS 样式，例子：

{% highlight html %}
<script language="JavaScript">
function getMethod20() {
    console.log(document.getElementById("div20").style.backgroundColor);
    document.getElementById("div20").style.backgroundColor = "yellow";
}
</script>
<div id="div20" style="width:100px; height:100px; background-color:red">我是一个DIV</div>
<input type="button" name="mybutton" value="按钮20" onClick="getMethod20()">
{% endhighlight %}









# 参考

如上的示例可以直接参考 [javascript-dom](/reference/webserver/javascript-dom.html) 。

另外可以参考一个不错的调试工具 [10分钟学会前端调试利器——FireBug](http://www.imooc.com/article/3012)，或者 [本地文档](/reference/javascript/10分钟学会前端调试利器.mht) 。




{% highlight html %}
{% endhighlight %}
