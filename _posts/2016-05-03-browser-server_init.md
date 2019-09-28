---
Date: October 19, 2013
title: 前后端交互
layout: post
comments: true
language: chinese
category: [webserver]
---

对于一个网站，除了前端工程师的 UI 重构外，还有一个非常重要的职责是在相应的区域渲染出服务端的数据，毕竟，一个大的 web 应用，肯定不是普普通通的静态页面构成。

本文大致罗列一些前端与后端交互的方法方式。

<!-- more -->

# AJAX, Asynchronous JavaScript and XML

AJAX 是一种使用现有标准的新方法，用于和服务器交换数据，可以在不重新加载整个页面的情况下，更新部分网页。例如，使用如下示例加载相关内容，其中 ajax/basic.html 的内容如下。

{% highlight html %}
<html><!DOCTYPE html>
<meta http-equiv="Content-Type" Content="text/html; charset=UTF-8"/>
<head>
<script>
function loadXMLDoc() {
    var xmlhttp;
    if (window.XMLHttpRequest) { // code for IE7+, Firefox, Chrome, Opera, Safari
        xmlhttp=new XMLHttpRequest();
    } else { // code for IE6, IE5
        xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
    }

    xmlhttp.onreadystatechange=function() {
        if (xmlhttp.readyState==4 && xmlhttp.status==200) {
            document.getElementById("myDiv").innerHTML=xmlhttp.responseText;
        }
    }

    xmlhttp.open("GET","/ajax/info.txt",true);
    xmlhttp.send();
}
</script>
</head>
<body>

<div id="myDiv"><h2>使用 AJAX 修改该文本内容</h2></div>

<button type="button" onclick="loadXMLDoc()">修改内容</button>

</body>
</html>
{% endhighlight %}

其中在服务器目录下有一个 ajax_info.txt 文件，内容如下。

{% highlight text %}
<h2>JUST FOR TEST</h2>
{% endhighlight %}


接下来一步步介绍相关内容。

## 执行流程

### 1. 新建 XMLHttpRequest 对象

所有现代浏览器均支持 XMLHttpRequest 对象，其中 IE5、IE6 使用 ActiveXObject，该对象用于在后台与服务器交换数据，创建 XMLHttpRequest 对象的语法如下。

{% highlight html %}
var xmlhttp=new XMLHttpRequest();
{% endhighlight %}

### 2. 向服务器发送请求

简单来说，发送请求需要使用 XMLHttpRequest 对象的 open() 和 send() 方法。

{% highlight text %}
xmlhttp.open("GET","/ajax/info.txt",true);
xmlhttp.send();
{% endhighlight %}

两个函数的声明如下：
{% highlight text %}
open(method,url,async);      # 规定请求的类型、URL 以及是否异步处理请求
send(string);                # 将请求发送到服务器，参数仅用于 POST 请求
{% endhighlight %}

发送请求可以使用 POST 或者 GET，相比来说 GET 更简单迅速，在大部分情况下都能用； 但是，在以下情况中，需要使用 POST 请求：

* 无法使用缓存文件，需要更新服务器上的文件或数据库；
* 向服务器发送大量数据，POST 没有数据量限制；
* 发送包含未知字符的用户输入时，POST 比 GET 更稳定也更可靠。


一个简单的 GET 请求：

{% highlight text %}
xmlhttp.open("GET","demo_get.html",true);
xmlhttp.send();
{% endhighlight %}

不过，上面的示例可能会得到缓存的结果，为了避免这种情况，可以在 URL 中添加一个唯一的 ID 。

{% highlight text %}
xmlhttp.open("GET","demo_get.html?t=" + Math.random(),true);
xmlhttp.send();
{% endhighlight %}

当然，也可以通过 GET 方法发送相关参数信息。

{% highlight text %}
xmlhttp.open("GET","demo_get2.html?fname=Henry&lname=Ford",true);
xmlhttp.send();
{% endhighlight %}


如果使用 POST 方式，只需要将上述的 GET 参数改为 POST，如果需要像 HTML 表单那样 POST 数据，则使用 setRequestHeader() 来添加 HTTP 头，然后在 send() 方法中规定您希望发送的数据。

{% highlight text %}
xmlhttp.open("POST","ajax_test.html",true);
xmlhttp.setRequestHeader("Content-type","application/x-www-form-urlencoded");
xmlhttp.send("fname=Henry&lname=Ford");
{% endhighlight %}


### 3. 接收

如上述的示例，采用的是异步，那么就需要在响应处的 onreadystatechange 事件中设置就绪状态时执行的函数。如果采用同步，则不需要编写 onreadystatechange 函数，直接把代码放到 send() 语句后面即可。

{% highlight text %}
xmlhttp.open("GET","ajax_info.txt",false);
xmlhttp.send();
document.getElementById("myDiv").innerHTML=xmlhttp.responseText;
{% endhighlight %}


### 4. 获取服务器响应

如需获得来自服务器的响应，可以使用 XMLHttpRequest 对象的 responseText 或 responseXML 属性，分别对应字符串以及 XML 格式的数据。

{% highlight javascript %}
// 返回的为纯文本
document.getElementById("myDiv").innerHTML=xmlhttp.responseText;

// 返回的为 XML 格式
xmlDoc=xmlhttp.responseXML;
txt="";
x=xmlDoc.getElementsByTagName("ARTIST");
for (i=0;i<x.length;i++) {
    txt=txt + x[i].childNodes[0].nodeValue + "<br>";
}
document.getElementById("myDiv").innerHTML=txt;
{% endhighlight %}



### 5. onreadystatechange 事件

请求发送到服务器后会执行一些基于响应的任务，每当 readyState 改变时，就会触发 onreadystatechange 事件。常见的有如下的几种状态： 0(请求未初始化)、1(服务器连接已建立)、2(请求已接收)、3(请求处理中)、4(请求已完成，且响应已就绪)。



## 示例

除了开始的简单示例之外，我们在此展示一个自动自动提示的示例，简单来说就是当用户在输入框中键入字符时，网页如何与 web 服务器进行通信，返回提示的结果，参考 ajax/auto.html 。

{% highlight html %}
<html><!DOCTYPE html>
<meta http-equiv="Content-Type" Content="text/html; charset=UTF-8"/>
<head>
<script>
function showHint(str)
{
    var xmlhttp;
    if (str.length==0) {
        document.getElementById("txtHint").innerHTML="";
        return;
    }
    if (window.XMLHttpRequest) {// code for IE7+, Firefox, Chrome, Opera, Safari
        xmlhttp=new XMLHttpRequest();
    } else {// code for IE6, IE5
        xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
    }

    xmlhttp.onreadystatechange=function() {
        if (xmlhttp.readyState==4 && xmlhttp.status==200) {
            document.getElementById("txtHint").innerHTML=xmlhttp.responseText;
        }
    }

    xmlhttp.open("GET","/ajax/gethint.php?q="+str,true);
    xmlhttp.send();
}
</script>
</head>
<body>

<h3>Start typing a name in the input field below:</h3>
<form action="">
First name: <input type="text" id="txt1" onkeyup="showHint(this.value)" />
</form>
<p>Suggestions: <span id="txtHint"></span></p>

</body>
</html>
{% endhighlight %}


如上，当用户在上面的输入框中键入字符时，会执行函数 "showHint()"，该函数由 "onkeyup" 事件触发，如果输入框为空 (str.length==0)，则该函数清空 txtHint 占位符的内容，并退出函数。

如果输入框不为空 showHint() 函数会执行如下过程：

1. 创建 XMLHttpRequest 对象；
2. 注册当服务器响应就绪时执行的回调函数；
3. 把请求发送到服务器上；
4. 请注意我们向 URL 添加了一个参数 q 。

下面是用 PHP  编写的一个服务端的程序，也就是 ajax/gethint.php 文件。


{% highlight php %}
<?php
// Fill up array with names
$a[]="Anna";
$a[]="Brittany";
$a[]="Cinderella";
$a[]="Diana";
$a[]="Eva";
$a[]="Fiona";
$a[]="Gunda";
$a[]="Hege";
$a[]="Inga";
$a[]="Johanna";
$a[]="Kitty";
$a[]="Linda";
$a[]="Nina";
$a[]="Ophelia";
$a[]="Petunia";
$a[]="Amanda";
$a[]="Raquel";
$a[]="Cindy";
$a[]="Doris";
$a[]="Eve";
$a[]="Evita";
$a[]="Sunniva";
$a[]="Tove";
$a[]="Unni";
$a[]="Violet";
$a[]="Liza";
$a[]="Elizabeth";
$a[]="Ellen";
$a[]="Wenche";
$a[]="Vicky";

//get the q parameter from URL
$q=$_GET["q"];

//lookup all hints from array if length of q>0
if (strlen($q) > 0) {
    $hint="";
    for($i=0; $i<count($a); $i++) {
        if (strtolower($q)==strtolower(substr($a[$i],0,strlen($q)))) {
            if ($hint=="") {
                $hint=$a[$i];
            } else {
                $hint=$hint." , ".$a[$i];
            }
        }
    }
}

// Set output to "no suggestion" if no hint were found or to the correct values
if ($hint == "") {
    $response="no suggestion";
} else {
    $response=$hint;
}

//output the response
echo $response;
?>
{% endhighlight %}

## 与数据库的交互

通过 AJAX 可用来与数据库进行动态通信，如下的例子将演示网页如何通过 AJAX 从数据库读取信息，通过下拉列表中选择一个客户，然后列出相应的用户，对应了 ajax/db.html 。

{% highlight html %}
<html><!DOCTYPE html>
<meta http-equiv="Content-Type" Content="text/html; charset=UTF-8"/>
<head>
<script>
function showCustomer(str)
{
    var xmlhttp;
    if (str=="") {
        document.getElementById("txtHint").innerHTML="";
        return;
    }
    if (window.XMLHttpRequest) {// code for IE7+, Firefox, Chrome, Opera, Safari
        xmlhttp=new XMLHttpRequest();
    } else {// code for IE6, IE5
        xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
    }

    xmlhttp.onreadystatechange=function() {
        if (xmlhttp.readyState==4 && xmlhttp.status==200) {
            document.getElementById("txtHint").innerHTML=xmlhttp.responseText;
        }
    }

    xmlhttp.open("GET","/ajax/getcustomer.php?q="+str,true);
    xmlhttp.send();
}
</script>
</head>
<body>

<form action="">
<select name="customers" onchange="showCustomer(this.value)"
style="font-family:Verdana, Arial, Helvetica, sans-serif;">
<option value="APPLE">Apple Computer, Inc.</option>
<option value="BAIDU ">BAIDU, Inc</option>
<option value="Canon">Canon USA, Inc.</option>
<option value="Google">Google, Inc.</option>
<option value="Nokia">Nokia Corporation</option>
<option value="SONY">Sony Corporation of America</option>
</select>
</form>
<br>
<div id="txtHint">Customer info will be listed here...</div>

</body>
</html>
{% endhighlight %}

如下是服务端的 PHP 程序，也就是 ajax/getcustomer.php 。

{% highlight php %}
<?php
$q=$_GET["q"];

$con = mysqli_connect('localhost','root','justkidding','my_db');
if (!$con) {
    die('Could not connect: ' . mysqli_error($con));
}

mysqli_select_db($con,"ajax_demo");
$sql="SELECT * FROM user WHERE id = '".$q."'";

$result = mysqli_query($con,$sql);

echo "<table border='1'>
<tr>
<th>Firstname</th>
<th>Lastname</th>
<th>Age</th>
<th>Hometown</th>
<th>Job</th>
</tr>";

while($row = mysqli_fetch_array($result)) {
    echo "<tr>";
    echo "<td>" . $row['FirstName'] . "</td>";
    echo "<td>" . $row['LastName'] . "</td>";
    echo "<td>" . $row['Age'] . "</td>";
    echo "<td>" . $row['Hometown'] . "</td>";
    echo "<td>" . $row['Job'] . "</td>";
    echo "</tr>";
}
echo "</table>";

mysqli_close($con);
?>
{% endhighlight %}

<!-- http://www.runoob.com/ajax/ajax-examples.html AJAX 实例 -->




# Comet

对于一些定时更新数据的场景，可以通过 setInterval 设置时间间隔，例如 10s 去获取一次请求，从而做一些更新，不过这样的话比较低效，会增加服务器的请求数量。为此，可以由服务器进行数据推送，Comet 能够让信息近乎实时的被推送到页面上，非常适合要求实时性的获取的数据的页面。

其执行的效果类似于实时聊天、淘宝上在手机支付成功之后的自动调转、微信网页版在手机扫描完二维码之后的自动调转等等。

Comet 目前有如下的几种实现方式：长轮询 (long-polling) 和 iframe 流 (streaming)。


## long polling

## iframe streaming



pushlet 提供了基于 AJAX 的 JavaScript 库文件用于实现长轮询方式的“服务器推”；还提供了基于 iframe 的 JavaScript 库文件用于实现流方式的“服务器推”。








<!-- comet相关资料: http://www.ibm.com/developerworks/cn/web/wa-lo-comet/ -->


# Server-Sent Events




# Web Sockets

HTML5 WebSocket 设计出来的目的就是使客户端浏览器具备像 C/S 架构下桌面系统的实时通讯能力，浏览器通过 JavaScript 向服务器发出建立 WebSocket 连接的请求，连接建立以后，客户端和服务器端就可以通过 TCP 连接直接交换数据。

也就是我们可以使用 web 技术构建实时性的程序比如聊天游戏等应用，而 Web Sockets 的 API 却很少，就下面这些。

{% highlight text %}
websocket = new WebSocket("ws://your.socket.com:9001");
websocket.onopen = function(evt) { /* do stuff */ };     // 当打开时
websocket.onclose = function(evt) { /* do stuff */ };    // 当web socket关闭
websocket.onmessage = function(evt) { /* do stuff */ };  // 进行通信时
websocket.onerror = function(evt) { /* do stuff */ };    // 发生错误时
websocket.send(message);           // 向服务器发发送消息
websocket.close();                 // 关闭连接
{% endhighlight %}

而服务端大概会执行如下的内容：创建一个socket + 绑定地址和端口 + 监听进入的连接 + 接收新的连接 + web socket 握手 + 解码数据 + 返回数据 + 关闭连接。


可以参考一个简单的聊天示例 [Simple Chat Using WebSocket and PHP Socket](https://www.sanwebe.com/2013/05/chat-using-websocket-php-socket) 。

# 总结


    AJAX - 请求 → 响应 （频繁使用）
    Comet - 请求 → 挂起 → 响应 （模拟服务端推送）
    Server-Sent Events - 客户单 ← 服务端 （服务端推送）
    WebSockets - 客户端 ↔ 服务端 (未来趋势,双工通信)

<!--
http://www.tuicool.com/articles/b2eUJzj  写给刚入门的前端工程师的前后端交互指南
http://ohcoder.com/blog/2014/11/11/raptor-part-1/  为什么我们开发的Raptor比Unicorn快4倍，比Puma，Torquebox快2倍
-->

{% highlight text %}
{% endhighlight %}
