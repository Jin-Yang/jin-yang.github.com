---
title: 给 Jekyll Blog 定制域名
layout: post
comments: true
language: chinese
tag: [Blog,Others]
keywords: jekyll,github,domain,custom,域名,定制
description: 可以为 GitHub Pages 指定自己申请的域名，这里介绍如何通过 GoDaddy 申请域名，然后再一步步进行详细配置。
---

通过 GitHub Pages 可以很方便的发布文章，同时还允许用户自己定义域名，这样可以显得更加专业，而且对于普通人来说，使用类似 foobar.github.io 的域名确实难记。

所以，这里简单介绍如何通过 GoDaddy 申请域名，并配置好 GitHub Pages 的域名。

<!-- more -->

## 简介

假设已经搭建了一个 GitHub Pages 的网站，对应的默认地址为 `foobar.github.io` ，然后，自己申请了一个 `example.me` 的域名，希望 `www.example.me` 以及 `blog.example.me` 都可以跳转到原始的 GitHub Pages 网站上。

### 申请域名

域名可以通过国内云厂商购买，也可以使用万网，购买时还可以备案，当然，也可以使用 GoDaddy 购买，支持支付宝付款，简单很多。

直接到 [GoDaddy](https://www.godaddy.com/) 搜索想要的域名，如果是个人 Blog 使用，建议可以使用 `.me` 顶级域名，像 `.org` `.cn` `.net` 的话一般价格会比较高，而 `.me` 相对来说便宜，而且有一定的特殊含义。

> `.me` 域名是前南斯拉夫地区西南部的国家黑山共和国 (Montenegro) 的国家域名，属于国家或地区顶级域名 (Country Code Top Level Domain, ccTLD) 而非通用顶级域名 (Generic Top Level Domain, gTLD)。该域名于 2008.7.16 正式对外开放注册，比较适合个人、博客等彰显自我形象的网站。

另外，在申请 GoDaddy 域名时可以先搜索使用优惠码，能稍微再便宜一部分。

## GitHub 配置

可以直接在顶级目录下添加 `CNAME` 文件，内容为申请的域名，例如上述的 `example.me` ；也可以在 `Settings -> GitHub Pages -> Custom domain` 中指定，此时会在源码目录下创建 CNAME 文件。

当再次访问 GitHub Pages 默认域名时，会自动跳转到新的域名，而且从 2018.5.1 之后开始支持自定义域名的 HTTPS 支持，详见 [blog.github.com](https://blog.github.com/2018-05-01-github-pages-custom-domains-https/) 中的介绍。

### HTTPS

GitHub 提供的 TLS 证书是由 Let's Encrypt 颁发的，一般是通过 API 免费申请。

在配置时，需要一段时间创建与 HTTPS 相关的配置，当配置完成后 (提示说要24小时，实际只需要几分钟)，需要在配置中勾选 `Enforce HTTPS` 选项即可。

![jekyll github pages setting enforce https](/{{ site.imgdir }}/linux/jekyll-github-pages-setting-enforce-https.png "jekyll github pages setting enforce https")

另外，在配置 DNS 解析时，不建议将 CNAME 的值配置为 `xxx.github.io` 地址，此时浏览器会报错，因为证书实际有效的是 `*.github.io` 域名，而浏览器访问的则是自定义的域名，会导致浏览器在匹配域名的时候报错。

<!--
GitHub使用letsencrypt提供的CAA证书签名服务：Certificate Authority Authorization (CAA) - Let’s Encrypt - Free SSL/TLS Certificates
需要说明的是有些域名服务商是不提供CAA的选项。
-->

{% include ads_content01.html %}

## GoDaddy 配置

购买完之后，一般几分钟后就可以打开申请的链接，只是一个空页面，这是 GoDaddy 提供的默认页面。

在 GoDaddy 右上角的 `My Products` 链接中可以看到已经购买的域名，然后，在 `All Products and Services` 标签页中，选择对应的域名，点击 `DNS` 默认会存在几个配置，如下。

![jekyll custom domain godaddy default settings](/{{ site.imgdir }}/linux/jekyll-custom-domain-godaddy-default-settings.png "jekyll custom domain godaddy default settings")

这也就是为什么在申请了域名之后就可以直接访问的原因，虽然返回的是空。

默认会提供两个 DNS 解析服务器，会有如下配置，其中 `ns43.domaincontrol.com` 为主，用户可以在 `Nameservers` 配置标签页中进行修改，例如指定自己搭建的服务器。

### CNAME 记录

CNAME 用来将一个域名 (同名) 映射到另外一个域名 (真实)，这样域名解析服务器在遇到 CNAME 记录时，会以目标域名开始重新查询，同样用在同一个 IP 上运行多个服务。

在使用 HTTPS 时可以配置为如下，也就是，即使访问 `www.example.me` 也会重定向到 `example.me` 中。

```
Type     Name    Value   TTL
CNAME    www     @       1 Hour
```

另外，在 GoDaddy 中也可以配置 Forwarding 配置。

如果不使用 HTTPS ，那么可以修改 CNAME 的记录，增加如下内容。

```
Type     Name    Value              TTL
CNAME    www     foobar.github.io   1 Hour
```

也就是说，当访问 `www.example.me` 时会自动访问到 `foobar.github.io` 的服务器，但是浏览器上仍然显示的是 `www.example.me` 这个网址。

在 Linux 上可以通过 `dig` 命令查看。

```
$ dig www.example.me +nostats +nocomments +nocmd
;www.example.me.              IN      A
www.example.me.       1778    IN      CNAME   foobar.github.io.
foobar.github.io.     3578    IN      A       185.199.111.153
foobar.github.io.     3578    IN      A       185.199.110.153
foobar.github.io.     3578    IN      A       185.199.108.153
foobar.github.io.     3578    IN      A       185.199.109.153
```

注意，如果此时访问的是 `example.me` 仍然是开始注册网页，如果要让顶级域名生效，就需要配置 A 记录。

### A 记录

在 GoDaddy 中是不允许配置等级域名的 CNAME 的，所以可以通过 A 记录将域名影射到 IP 地址上。

GitHub Pages 当前提供的 IP 地址列表可以通过 [GitHub Docs](https://docs.github.com/cn/free-pro-team@latest/github/working-with-github-pages/managing-a-custom-domain-for-your-github-pages-site) 查看，实际上，也就是上述通过 `dig` 命令查看的结果，在 GoDaddy 中增加如下记录。

```
Type     Name    Value              TTL
A        @       185.199.108.153    600s
A        @       185.199.109.153    600s
A        @       185.199.110.153    600s
A        @       185.199.111.153    600s
```

这里的 IP 地址也就是 GitHub 提供 Pages 服务的 IP ，正常来说只会增加而不会减少或者修改这里的 IP 地址，当然，也不排除会有变更。

这样，就可以通过 `example.me` 直接访问了。

{% include ads_content02.html %}

## 参考

官方的文档可以参考 [Managing A Custom Domain](https://docs.github.com/cn/free-pro-team@latest/github/working-with-github-pages/managing-a-custom-domain-for-your-github-pages-site) 中的介绍，以及 [Troubleshooting](https://docs.github.com/en/free-pro-team@latest/github/working-with-github-pages/troubleshooting-custom-domains-and-github-pages) 中介绍。


<!--
使用Google账号登录GoDaddy，对应域名为gohalo.me，有效期为三年 2020.10.12~2023.10.12

301转发
然而godaddy还支持设置把一个域名301跳转到另外一个域名，所以可以把example.com跳转到www.example.com。
godaddy在你的域名下配置Forwarding，链接输入www.example.com，选择301，保存。
用A或者301都是可选操作，大家可以尝试后再决定。

DNSPod
http://blog.ulinkmega.com/method/2015/06/11/GithubPages-Custom-Domain/
-->

{% highlight text %}
{% endhighlight %}
