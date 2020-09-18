---
Date: October 19, 2013
title: jquery 使用
layout: post
comments: true
language: chinese
category: [webserver]
---



<!-- more -->







## AJAX

load(url, [data], [callback])   载入远程HTML文件代码并插入至DOM中
   url (String): 请求的HTML页的URL地址；
   data (Map)  : 发送至服务器的key/value数据，可选；
   callback    : 请求完成时(不需要是success的)的回调函数，可选。

jQuery.get( url, [data], [callback] )：使用GET方式来进行异步请求
jQuery.post( url, [data], [callback], [type] ) ：使用POST方式来进行异步请求
jQuery.getScript( url, [callback] ) : 通过 GET 方式请求载入并执行一个 JavaScript 文件。
jQuery Ajax 事件
jQuery.ajax( options ) : 通过 HTTP 请求加载远程数据
jQuery.ajaxSetup( options ) : 设置全局 AJAX 默认选项。

设置 AJAX 请求默认地址为 "/xmlhttp/"，禁止触发全局 AJAX 事件，用 POST 代替默认 GET 方法。其后的 AJAX 请求不再设置任何选项参数。

jQuery 代码:

$.ajaxSetup({
      url: "/xmlhttp/",
        global: false,
          type: "POST"
});
$.ajax({ data: myData });



 serialize() 与 serializeArray()

 serialize() : 序列表表格内容为字符串。

 serializeArray() : 序列化表格元素 (类似 '.serialize()' 方法) 返回 JSON 数据结构数据。

 示例：:%s/\s\+$//g




http://www.cnblogs.com/onlys/articles/jQuery.html

## jquery 选择器

jQuery 的选择器可谓之强大无比，这里简单地总结一下常用的元素查找方法

$("#myELement")    选择id值等于myElement的元素，id值不能重复在文档中只能有一个id值是myElement所以得到的是唯一的元素
$("div")           选择所有的div标签元素，返回div元素数组
$(".myClass")      选择使用myClass类的css的所有元素
$("*")             选择文档中的所有的元素，可以运用多种的选择方式进行联合选择：例如$("#myELement,div,.myclass")

层叠选择器：
$("form input")         选择所有的form元素中的input元素
$("#main > *")          选择id值为main的所有的子元素
$("label + input")     选择所有的label元素的下一个input元素节点，经测试选择器返回的是label标签后面直接跟一个input标签的所有input标签元素
$("#prev ~ div")       同胞选择器，该选择器返回的为id为prev的标签元素的所有的属于同一个父元素的div标签

基本过滤选择器：
$("tr:first")               选择所有tr元素的第一个
$("tr:last")                选择所有tr元素的最后一个
$("input:not(:checked) + span")

过滤掉：checked的选择器的所有的input元素

$("tr:even")               选择所有的tr元素的第0，2，4... ...个元素（注意：因为所选择的多个元素时为数组，所以序号是从0开始）

$("tr:odd")                选择所有的tr元素的第1，3，5... ...个元素
$("td:eq(2)")             选择所有的td元素中序号为2的那个td元素
$("td:gt(4)")             选择td元素中序号大于4的所有td元素
$("td:ll(4)")              选择td元素中序号小于4的所有的td元素
$(":header")
$("div:animated")
内容过滤选择器：

$("div:contains('John')") 选择所有div中含有John文本的元素
$("td:empty")           选择所有的为空（也不包括文本节点）的td元素的数组
$("div:has(p)")        选择所有含有p标签的div元素
$("td:parent")          选择所有的以td为父节点的元素数组
可视化过滤选择器：

$("div:hidden")        选择所有的被hidden的div元素
$("div:visible")        选择所有的可视化的div元素
属性过滤选择器：

$("div[id]")              选择所有含有id属性的div元素
$("input[name='newsletter']")    选择所有的name属性等于'newsletter'的input元素

$("input[name!='newsletter']") 选择所有的name属性不等于'newsletter'的input元素

$("input[name^='news']")         选择所有的name属性以'news'开头的input元素
$("input[name$='news']")         选择所有的name属性以'news'结尾的input元素
$("input[name*='man']")          选择所有的name属性包含'news'的input元素

$("input[id][name$='man']")    可以使用多个属性进行联合选择，该选择器是得到所有的含有id属性并且那么属性以man结尾的元素

子元素过滤选择器：

$("ul li:nth-child(2)"),$("ul li:nth-child(odd)"),$("ul li:nth-child(3n + 1)")

$("div span:first-child")          返回所有的div元素的第一个子节点的数组
$("div span:last-child")           返回所有的div元素的最后一个节点的数组
$("div button:only-child")       返回所有的div中只有唯一一个子节点的所有子节点的数组

表单元素选择器：

$(":input")                  选择所有的表单输入元素，包括input, textarea, select 和 button

$(":text")                     选择所有的text input元素
$(":password")           选择所有的password input元素
$(":radio")                   选择所有的radio input元素
$(":checkbox")            选择所有的checkbox input元素
$(":submit")               选择所有的submit input元素
$(":image")                 选择所有的image input元素
$(":reset")                   选择所有的reset input元素
$(":button")                选择所有的button input元素
$(":file")                     选择所有的file input元素
$(":hidden")               选择所有类型为hidden的input元素或表单的隐藏域

表单元素过滤选择器：

$(":enabled")             选择所有的可操作的表单元素
$(":disabled")            选择所有的不可操作的表单元素
$(":checked")            选择所有的被checked的表单元素
$("select option:selected") 选择所有的select 的子元素中被selected的元素



选取一个 name 为”S_03_22″的input text框的上一个td的text值
$(”input[@ name =S_03_22]“).parent().prev().text()

名字以”S_”开始，并且不是以”_R”结尾的
$(”input[@ name ^='S_']“).not(”[@ name $='_R']“)

一个名为 radio_01的radio所选的值
$(”input[@ name =radio_01][@checked]“).val();





$("A B") 查找A元素下面的所有子节点，包括非直接子节点
$("A>B") 查找A元素下面的直接子节点
$("A+B") 查找A元素后面的兄弟节点，包括非直接子节点
$("A~B") 查找A元素后面的兄弟节点，不包括非直接子节点

1. $("A B") 查找A元素下面的所有子节点，包括非直接子节点

例子：找到表单中所有的 input 元素

HTML 代码:

<form>
<label>Name:</label>
<input name="name" />
<fieldset>
      <label>Newsletter:</label>
      <input name="newsletter" />
</fieldset>
</form>
<input name="none" />
jQuery 代码:

$("form input")
结果:

[ <input name="name" />, <input name="newsletter" /> ]

2. $("A>B") 查找A元素下面的直接子节点
例子：匹配表单中所有的子级input元素。

HTML 代码:

<form>
<label>Name:</label>
<input name="name" />
<fieldset>
      <label>Newsletter:</label>
      <input name="newsletter" />
</fieldset>
</form>
<input name="none" />
jQuery 代码:

$("form > input")
结果:

[ <input name="name" /> ]

3. $("A+B") 查找A元素后面的兄弟节点，包括非直接子节点
例子：匹配所有跟在 label 后面的 input 元素

HTML 代码:

<form>
<label>Name:</label>
<input name="name" />
<fieldset>
      <label>Newsletter:</label>
      <input name="newsletter" />
</fieldset>
</form>
<input name="none" />
jQuery 代码:

$("label + input")
结果:

[ <input name="name" />, <input name="newsletter" /> ]


4. $("A~B") 查找A元素后面的兄弟节点，不包括非直接子节点
例子：找到所有与表单同辈的 input 元素

HTML 代码:

<form>
<label>Name:</label>
<input name="name" />
<fieldset>
      <label>Newsletter:</label>
      <input name="newsletter" />
</fieldset>
</form>
<input name="none" />
jQuery 代码:

$("form ~ input")
结果:

[ <input name="none" /> ]
{% highlight python %}
{% endhighlight %}
