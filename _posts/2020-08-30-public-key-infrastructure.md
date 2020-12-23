---
title: 详细介绍 PKI 以及 CA 基本概念
layout: post
comments: true
language: chinese
tag: [Linux, TLS/SSL, Security, SoftWare, DevOps]
keywords: Public Key Infrastructure,PKI,Certificate Authority,CA
description: PKI 以及 CA 时目前网络安全建设的基础核心，以此加强服务端与客户端的安全性，这里会详细介绍基本概念，包括了证书的基本概念、基本工作原理等。
---

公钥基础设施 (Public Key Infrastructure, PKI) 是目前网络安全建设的基础与核心，而证书认证机构 (Certificate Authority, CA) 更是核心中的核心，其中 CA 最重要的用途就是提供根证书，用来加强服务器和客户端之间信息交互的安全性。

这里简单介绍一些基本概念。

<!-- more -->

## 简介

假设你要通过支付宝转笔钱，首先要根服务器建立连接，浏览器收到了服务器发送的证书，其中包括了服务器的公钥，那么怎么证明收到的公钥就是该服务器的呢？

![https ca](/{{ site.imgdir }}/linux/https-ca-intro.png "https ca"){: width="40%" }

如果无法解决如上的问题，那么就可能会导致中间人攻击，以及信息抵赖，这时，CA 就出场了。

解决上述身份验证问题的关键是确保获取的公钥途径是合法的，能够验证服务器的身份信息，为此需要引入权威的第三方机构 (Certificated Authority, CA)，一个第三方可信证书颁发机构，具有权威性、公正性的机构。

CA 负责核实公钥的拥有者的信息，并颁发认证 "证书"，同时能够为使用者提供证书验证服务，即 PKI 体系。

### Certificated Authority

从机制上来说，任何人或者组织都可以扮演 CA 的角色，只是很难可以得到绝大部分客户端的信任，在 [WikiPedia](https://en.wikipedia.org/wiki/Certificate_authority) 中介绍了常用的 CA 提供商，例如
DigiCert、GoDaddy、GlobalSign、VeriSign 等等，另外，像 Windows 一般会内置自己的根证书，CentOS 会内置 Mozilla Foundation 的根证书。

一个新的 CA 公司，至少要 5~10 年才会被广泛信任，进入到根证书链，所以，通常开始是给 CA 付钱，买次级证书。

> 签署一个证书的成本可以忽略，但是为什么一个证书的价格会很高？其实 CA 公司的主要成本构成在于审核、保险担保费用等，例如要想进入到各个浏览器的根证书列表，需要通过 WebTrust 的审计，部分浏览器会对根证书收费；提供 CRL、OCSP 甚至 HSM 的服务器成本，也包括了支持证书申请、更新、撤销、恢复等操作的平台支持；而且在用户提交 CSR 之后，CA 需要进行多项认证，而且越高级证书验证越复杂；当然，也包括了保险费用。

那么选择 CA 的时候重点评估的有：A) 内置根证书范围，内置了越多的系统、浏览器就说明其覆盖的面越广；B) 保证体系是否完善，包括了之前是否黑历史，例如 DigiNotar 直接倒闭；C) 核心功能的支持，例如 SNI、CAA 等的支持。

### X509 证书

一个证书主要包含三部分：A) 用户的信息；B) 用户的公钥；C) CA 对该证书信息的签名。标准的数字证书格式如下，详细可以参考 [RFC2459](https://www.ietf.org/rfc/rfc2459.txt) 中的介绍。

{% highlight text %}
+-------------------------------+----------
|           Version             |  |  |  |  <-- 版本号
+-------------------------------+  |  |  |
|  Certificate Serial Number    |  V  V  V  <-- 证书序列号，也可以指定
+-------------------------------+  E  E  E
| Sinature Algorithm Identifier |  R  R  R  <-- 签名加密算法标识
+-------------------------------+  S  S  S
|         Issuer Name           |  I  I  I  <-- 颁发机构信息，包括了C/ST/L/O/CN等信息
+-------------------------------+  O  O  O
|    Not Before & Not After     |  N  N  N  <-- 证书有效期，指定了开始结束范围
+-------------------------------+  1  2  3
|        Subject Name           |  |  |  |  <-- 证书持有人信息
+-------------------------------+  |  |  |
|   Subject's Public Key Info   |  |  |  |  <-- 证书持有人公钥信息
+-------------------------------+---- |  |
|   Issuer Unique Identifier    |     |  |  <-- 签发机构唯一标识
+-------------------------------+     |  |
|   Subject Unique Identifier   |     |  |  <-- 证书持有人唯一标识
+-------------------------------+------- |
|          Extensions           |        |  <-- 扩展信息
+-------------------------------+----------
|          Signature            |  ALL      <-- 证书签名信息
+-------------------------------+
{% endhighlight %}


在浏览器中，可以点击地址栏中的锁标志然后查看证书信息，如下是 Chrome 中关于 `github.com` 的证书信息。

![tls github certificate general info](/{{ site.imgdir }}/linux/tls-github-certificate-general-info.png "tls github certificate general info")

该页面显示了证书中比较重要的内容，包括了证书颁发者、拥有者信息，以及有效期等信息。

而详细的信息可以从 Details 标签页中查看，如下是证书的详细信息。

![tls github certificate details info](/{{ site.imgdir }}/linux/tls-github-certificate-details-info.png "tls github certificate details info")

详细信息一般包括了：

* 版本号 `Version` 用来指明 `X.509` 证书的格式版本，包括了 `V1`、`V2`、`V3`；
* 序列号 `Serial Number` CA 分配给证书的唯一标识符，用于证书被取消时使用；
* 签名算法 `Signature Algorithm` CA 签发证书使用的签名算法，如 `sha256WithRSAEncryption`；
* 签发机构名 `Issuer` 标识签发证书 CA 的信息，包含的字段及其含义详见下面的介绍；
* 有效期 `Validity` 指定证书的有效期，包括了开始 `Not Before` 和失效日期 `Not After`；
* 证书用户名 `Subject` 使用该证书的机构，包含的内容与签发机构类似，同样详见如下介绍；
* 证书持有者公钥信息 `Subject Public Key Info` 包括了公钥使用算法以及公钥值；
* 扩展项 `Extension` V3 证书对 V2 的扩展项，以使证书能够附带额外信息；
* 签名信息 `Certificate` 包括了签名算法及其值；

上述颁发者和持有者信息一般包含了如下字段：

* 国家信息 `Country Name, C` 使用的是 ISO 格式，通过两个字母代号代表相应的国家，例如 `US`、`CN` 等；
* 州或者省信息 `State or Province Name, ST` 按照英文名填写即可，例如 `California`、`ShangHai`、`ZheJiang` 等；
* 地区信息 `Locality Name, L` 可选，可以是城市的名称，例如 `San Francisco`、`HangZhou` 等；
* 组织信息 `Organization Name, O` 可以是公司名称，例如 `Github, Inc`、`Alipay.com Co.,Ltd` 等；
* 单位部门 `Organizational Unit Name, OU` 可以省略，例如 `IT Dept`、`Secure Web Server` 等；
* 通用名称 `Common Name, CN` 一般是网站的名称，也可以使用通配符，例如 `github.com`、`*.alipay.com` 等；

当然，可以将网站的证书导出，那么也可通过 OpenSSL 的命令查看。

```
openssl x509 -noout -text -in cert.pem
```

另外，也可以将 `-text` 替换为 `-serial` `-subject` `-fingerprint` 查看具体的信息。

### 证书申请流程

由用户自己生成 Certificate Signing Request, CSR 后，向 CA 提出申请，CA 在核实申请者身份后，会用自己的私钥对信息进行签名生成证书。

![encrypt digital signature](/{{ site.imgdir }}/linux/encrypt-digital-signature.jpg "encrypt digital signature"){: width="60%" }

服务器则将这份由 CA 颁发的公钥证书在握手阶段发送给客户端，证书就相当于是服务器的身份证。

在验证证书的有效性的时候，会逐级去寻找签发者的证书，直至根证书为结束，然后通过公钥一级一级验证数字签名的正确性。

![client verify certificate chains](/{{ site.imgdir }}/linux/https-client-verify.gif "client verify certificate chains"){: width="50%" }

也就是说，无论如何，都需要有根证书才可以的，其中有些浏览器是使用的系统的，而有些是自己管理。

{% include ads_content01.html %}

## 证书实践操作

如上所述 CA 之所以重要，主要原因是因为在操作系统 (Windows、CentOS、RedHat等)、浏览器 (Chrome、FireFox等) 中已经内置，这样通过 CA 颁发的证书就可以直接通过证书链查找到上层的次级证书或者根证书等。

在操作系统以及浏览器中包含了一系列内置的 CA 根证书文件，对于操作系统来说，并没有严格规定其存放的位置，这也就导致了不同操作系统的存放位置也有所区别。

而且不同软件保存的路径也略有区别，例如 OpenSSL 会保存在 `/etc/ssl/certs` 目录下，Java 也会有自己的目录以及证书管理方式，如下介绍常见的根证书管理。

### 操作系统

对于 GoLang 来说，在使用 TLS 时就需要兼容各个操作系统，例如，对于 Linux 可以查看 `crypto/x509/root_linux.go` 文件中的实现；还有就是 Unix 的 `root_unix.go` 文件。

``` go
var certFiles = []string{
	"/etc/ssl/certs/ca-certificates.crt",                // Debian/Ubuntu/Gentoo etc.
	"/etc/pki/tls/certs/ca-bundle.crt",                  // Fedora/RHEL 6
	"/etc/ssl/ca-bundle.pem",                            // OpenSUSE
	"/etc/pki/tls/cacert.pem",                           // OpenELEC
	"/etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem", // CentOS/RHEL 7
	"/etc/ssl/cert.pem",                                 // Alpine Linux
}
```

如下简单介绍一些常见操作系统的配置，包括了如何更新配置。

#### CentOS

通过 `ca-certificates` 安装 Mozilla Foundation 提供的 PKI 组件，详见 [Fedora CA-Certificates](https://fedoraproject.org/wiki/CA-Certificates) 中的介绍，大概有 150+ 个根证书，可以直接查看 `/etc/pki/tls/certs/ca-bundle.crt` 文件中的内容，包括了根证书、次级证书等。

添加证书的时候将证书复制到 `/etc/pki/ca-trust/source/anchors/` 目录下，然后执行 `update-ca-trust` 命令即可。

如果使用上述的 `openssl x509` 命令，只会显示第一个，如果要查看全部，可以通过如下方式查看。

```
openssl crl2pkcs7 -nocrl -certfile cert.pem | openssl pkcs7 -print_certs -noout
```

<!--
openssl s_client -connect foo.whatever.com:443 -CApath /etc/ssl/certs
-->

## 其它

### 证书类型

在申请 SSL/TLS 证书时，按照安全等级、审核严格程度、成本开销等，可以将证书主要分成三类：

* Domain Validation, DV 域名验证型证书，这是最简单的，通过验证域名所有权即可签发证书，适合个人和小微企业申请，浏览器中会有锁标识。
* Organization Validation, OV 企业验证型证书，除了验证域名所有权之外，还会检查申请企业的真实身份，这也是使用最广的，通常中小企业、非营利、政府机构会申请，浏览器中除了锁标识外，还可以查看企业信息。
* Extended Validation, EV 增强验证型证书，证书审核级别为所有类型最严格验证方式，再 OV 的基础上增加了额外验证其它相关信息，例如银行开户信息，多用于银行、金融、证券等高安全标准行业，浏览器中会有独特的绿色标识。

另外，还有 Individual Validation, IV 证书，通常用于个人使用。

简单来说，可以按照申请方的身份简单区分：个人申请 DV 类型证书，中小企业选择 OV 类型证书，大型企业、银行、金融、支付等行业申请 EV 类型证书。而且，OV 及其以上的类型可以做到身份认证，而 DV 不具备身份认证功能；OV 以上支持目前国际主流 ECC 算法，可以做到更快，更复杂的加密防护。

三种不同的类型可以通过浏览器查看，不同类型在 Subject 字段的详细程度不同，如下

##### DV 证书

![ca certificates type DV](/{{ site.imgdir }}/security/ca-certificates-type-dv.png "ca certificates type DV"){: width="60%" }

##### OV 证书

![ca certificates type OV](/{{ site.imgdir }}/security/ca-certificates-type-ov.png "ca certificates type OV"){: width="60%" }

##### EV 证书

![ca certificates type EV](/{{ site.imgdir }}/security/ca-certificates-type-ev.png "ca certificates type EV"){: width="60%" }

### 证书导入

如上所述，Firefox 使用自己的证书管理器 (Certificates Manager)，并没有像 IE 和 Chrome 那样直接使用系统中的证书存储，所以，对于 Firefox 来说，需要将自签发的 SSL 证书导入受信任根证书。

对于 Firefox 来说，在 `Preference->Privacy&Security->Certificates->Certificate Manager` 选项中导入生成的证书文件即可，列表中是按照 Organize 排序的。

{% include ads_content02.html %}


{% highlight text %}
{% endhighlight %}
