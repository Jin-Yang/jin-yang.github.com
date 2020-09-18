---
Date: October 19, 2013
title: 一些不错的 HTML+CSS 入门示例
layout: post
comments: true
language: chinese
category: [webserver]
---


<!-- more -->


# 登陆

这个实际上是从 [Slick login form with HTML5 & CSS3](http://red-team-design.com/slick-login-form-with-html5-css3/) 参考的示例，该 blog 中有很多不错的前端设计示例，其效果可以直接参考 [login examle](/reference/linux/html_css/login/index.html) 。

{% highlight html %}
<form id="login">
    <h1>Log In</h1>
    <fieldset id="inputs">
        <input id="username" type="text" placeholder="Username" autofocus required>
        <input id="password" type="password" placeholder="Password" required>
    </fieldset>
    <fieldset id="actions">
        <input type="submit" id="submit" value="Log in">
        <a href="">Forgot your password?</a><a href="">Register</a>
    </fieldset>
</form>
{% endhighlight %}



根据最新的 HTML5 文档，其中的一些属性列举如下：

* placeholder 用来提示输入框中所需要输入的内容。

* required 表示当表单提交的时候，该输入框是必须的。

* autofocus 指定当加载之后自动聚焦到该输入框。

* type 指定类型，对于 password 来说，字段中的字符会被遮蔽；而 text 则会直接显示。





## 常见示例

* 弹出联系方式等，可以参考 [示例](/reference/linux/bootstrap/pop_toolbars/index.html) 。



<!-- http://www.cyzone.cn/a/20140113/248500.html     10个适合初学者的HTML5入门教程 -->

<!--
自由分享jQuery、html5和css3的插件库
http://www.htmleaf.com/index.html
-->

{% highlight text %}
{% endhighlight %}
