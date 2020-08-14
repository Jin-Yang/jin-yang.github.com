---
title: SSL/TLS 证书详情
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: tls,ssl,ca
description:
---


<!-- more -->

## CA 证书详情

假设你要通过支付宝转笔钱，首先要根服务器建立连接，浏览器收到了服务器发送的证书，其中包括了服务器的公钥，那么怎么证明收到的公钥就是该服务器的呢？

![https]({{ site.url }}/images/linux/https-ca-intro.png "https"){: .pull-center width="50%" }

如果无法解决如上的问题，那么就可能会导致中间人攻击，以及信息抵赖，这时，CA 就出场了。

解决上述身份验证问题的关键是确保获取的公钥途径是合法的，能够验证服务器的身份信息，为此需要引入权威的第三方机构 (Certificated Authority, CA)，一个第三方可信证书颁发机构，具有权威性、公正性的机构。

CA 负责核实公钥的拥有者的信息，并颁发认证 "证书"，同时能够为使用者提供证书验证服务，即 PKI 体系。

### 证书内容

证书包含的内容可以概述为三部分，用户的信息、用户的公钥、还有 CA 中心对该证书里面的信息的签名。


![https]({{ site.url }}/images/linux/https-ca-github.png "https"){: .pull-center width="60%"}

如上是 Firfox 显示的 Github 中的证书，该页面显示了证书中比较重要的内容，而详细的信息可以从 Details 标签页中查看。

下面是详细内容：

* 版本号(Version)，用来指明 X.509 证书的格式版本，包括了 V1、V2、V3；
* 序列号(Serial Number)，CA 分配给证书的唯一标识符，用于证书被取消时；
* 签名算法(Signature Algorithm)，CA 签发证书使用的签名算法，如 sha256WithRSAEncryption；
* 签发机构名(Issuer)，标识签发证书的 CA 的 X.500 DN(DN-Distinguished Name) 名字，包括了国家(C)、省市(ST)、地区(L)、组织机构(O)、单位部门(OU)、通用名(CN)、邮箱地址；
* 有效期(Validity)，指定证书的有效期，包括了开始和失效日期；
* 证书用户名(Subject)，使用该证书的机构，包含的内容与签发机构名相同；
* 证书持有者公钥信息(Subject Public Key Info)，包括了公钥值以及公钥使用的算法；
* 扩展项(extension)，V3 证书对 V2 的扩展项，以使证书能够附带额外信息；

<!--
### 证书链

.减少根证书结构的管理工作量，可以更高效的进行证书的审核与签发;

b.根证书一般内置在客户端中，私钥一般离线存储，一旦私钥泄露，则吊销过程非常困难，无法及时补救;

c.中间证书结构的私钥泄露，则可以快速在线吊销，并重新为用户签发新的证书;

d.证书链四级以内一般不会对 HTTPS 的性能造成明显影响。

CA 机构能够签发证书，同样也存在机制宣布以往签发的证书无效，也就是吊销证书。证书使用者不合法，CA 需要废弃该证书；或者私钥丢失，使用者申请让证书无效。
-->

### 提交验证流程

首先，介绍下服务器的开发者是如何向 CA 申请证书的。

服务器的开发者向 CA 提出申请，CA 在核实申请者的身份之后，会用自己的私钥对信息进行加密，也就是数字签名；服务器则将这份由 CA 颁发的公钥证书在握手阶段发送给客户端，证书就相当于是服务器的身份证。

![encrypt digital signature]({{ site.url }}/images/linux/encrypt-digital-signature.jpg "encrypt digital signature"){: .pull-center width="60%" }

在验证证书的有效性的时候，会逐级去寻找签发者的证书，直至根证书为结束，然后通过公钥一级一级验证数字签名的正确性。

![client verify]({{ site.url }}/images/linux/https-client-verify.gif "client verify"){: .pull-center width="60%" }

也就是说，无论如何，都需要有根证书才可以的，其中有些浏览器是使用的系统的，而有些是自己管理。

### 自签名证书

如上所述，Firefox 使用自己的证书管理器 (Certificates Manager)，并没有像 IE 和 Chrome 那样直接使用系统中的证书存储，所以，对于 Firefox 来说，需要将自签发的 SSL 证书导入受信任根证书。

对于 Firefox 来说，在 `Preference->Privacy&Security->Certificates->Certificate Manager` 选项中导入生成的证书文件即可，列表中是按照 Organize 排序的。

{% highlight text %}
{% endhighlight %}
