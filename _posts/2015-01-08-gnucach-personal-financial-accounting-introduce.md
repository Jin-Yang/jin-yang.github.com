---
title: GnuCash 使用简介
layout: post
comments: true
category: [linux,misc]
language: chinese
keywords: gnucach,personal,small-business,finacial
description: GnuCash 是一个免费开源，支持多平台的记账软件。
---

GnuCash 是一个免费开源，支持多平台的记账软件，免费开源的记账软件。

<!-- more -->

## 简介

主要用于复式记账，基本概念与会计上的概念相同。

其账目分为了五大类：资产 (Assets)、负债 (Liabilities)、收入 (Income)、支出 (Expenses)、净值 (Equity) 。资产包括了现金、房子、车子等，而信用卡、贷款则属于负债，而净值标示你真正拥有的权益，不过其概念感觉不太清晰，感觉就是为了平衡两者。

### 安装

在 CentOS 中，如果直接安装二进制包，需要使用 `epel` 数据源，可以通过如下方式安装。

{% highlight text %}
# yum --enablerepo=epel install gnucash
{% endhighlight %}

安装完后，默认显示的是英文，如果要使用中文，可以将 `/etc/gnucash/environment` 中的语言设置相关配置进行修改。

{% highlight text %}
LANG=zh_CN
LANGUAGE={LANG}
{% endhighlight %}

不过兼容性不太好，经常会崩溃。

### 设置账簿

可以通过如下方式新建一个账簿。

1. `[File]->[New File]` 然后在弹出的窗口中新建一个帐号。
2. 选择本币位，例如 (CNY 人民币) 。
3. 设置帐号的相关选项，如果是个人用户或者不太熟悉的话，可以直接使用默认，如果需要的话可以通过 `[File]->[Properties]` 进行设置。
4. 选择需要创建的科目，个人一般选择 `Common Accounts` 。
5. 设置科目名称，如果不想使用默认，可以在这里修改。

最终的界面如下所示。

![interface]({{ site.url }}/images/accounting/gnucash-accounting-interface.png "interface"){: .pull-center width="60%" }

可以看到帐簿的顶级有五类科目，其中子科目都对应着个人：

* 资产，记录拥有的东西，常用子科目有：流动资产、固定资产；
* 负债，记录欠的东西，常见子科目有：信用卡、贷款；
* 所有者权益，记录净值，常见子科目有：余额；
* 收入，记录从别人那里收到的利益，常见子科目有：薪金、奖金、利息收入；
* 支出，记录花费，常见子科目有：房租、用餐、公共交通、水电费、医疗费用、税费、保险。

科目之下可以创建子科目，例如在活期存款下为你的每个银行帐户创建一个子科目，可以直接右键点击创建。

<!--
资产可设为资产、银行、现金、股票、共同基金、应收帐款中最合适的一个
负债可设为信用卡、负债、应付帐款中最合适的一个

### 支出

假设用现金买了个 10 块钱的拉面，可以如下记帐：

* 在科目标签中，双击支出科目的用餐
在电子表格中输入日期、交易描述（如拉面）
在转帐中选取款项来源（如资产:流动资产:现金）
在支出中输入金额后回车确认交易

### 收入


#### 工资

收入的记帐与支出类似，作为一个例子，假如你的工行帐户收到4000块钱的薪金，可以如下记帐：

* 
在科目标签卡双击打开支出科目（如薪金）
在电子表格中输入日期、交易描述（如工资）
在转帐中选取款项去向（如资产:流动资产:活期存款:工行）
在收入中输入金额后回车确认交易
如果你稳定周期性地收到相同的收入 ，可以建立计划的交易，如右键上述交易计划：
-->

## 基本概念

首先会有几个基本的会计上的等式。

{% highlight text %}
Assets - Liabilities = Equity
{% endhighlight %}

另外，权益还可以通过收入来增加，同时消费会减少，也就是如下的等式。

{% highlight text %}
Assets - Liabilities = Equity + (Income - Expense)
{% endhighlight %}

也就是类似如下的等式。

![the basic accounts relationships]({{ site.url }}/images/accounting/the-basic-accounts-relationships.png "the basic accounts relationships"){: .pull-center width="60%" }

目前使用比较多的是复式记账法 (Double Entry Accounting System) ，也就是一个帐号的修改，必定会伴随着另外一个帐号的修改，两者会保持平衡。



## 名词参考

### 银行账户

一般个人在银行开设的账户，按照性质不同主要分为下列几种：

* 普通支票账户 (Checking Account)，一般无息，可以免费多次存取；
* 活期储蓄账户 (Savings Account)，一般固定利率，一般一个月取一次，超过次数会收取手续费；
* 定期储蓄账户 (Certificate of Deposit, CD)，固定利率；
* 浮动利率账户 (Money Market Account)，也算活期储蓄账户的一种，但利率随行就市每日调整。

### Net Worth

资产净值，也可以理解为权益 (Equity)。


## 参考

官方网站可以参考 [www.gnucash.org](https://www.gnucash.org/) ，官方相关的文档可以参考 [GnuCash Help Manual](https://code.gnucash.org/docs/C/gnucash-help.pdf) 以及 [GnuCash Tutorial and Concepts Guide](https://code.gnucash.org/docs/C/gnucash-guide.pdf) 。

<!--
https://zhuanlan.zhihu.com/p/34187912
https://blog.michitsoft.com/2008/03/gnucash_22.html

https://blog.csdn.net/chungkwong/article/details/79477560
http://www.dayanzai.me/gnucash.html
-->

{% highlight text %}
{% endhighlight %}
