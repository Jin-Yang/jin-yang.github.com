---
title: 给 Jekyll Blog 添加 AdSense 广告
layout: post
language: chinese
category: [misc]
keywords: jekyll,blog,adsense,广告
description: 通过 Google 提供的 AdSense 平台，可以在 Jekyll 提供的 Blog 中添加广告，并以此来赚取收益。这里详细介绍如何在 AdSense 上注册，然后在页面上添加广告，并进行优化。
---

通过 Google 提供的 AdSense 平台，可以在 Jekyll 提供的 Blog 中添加广告，并以此来赚取收益。

这里详细介绍如何在 AdSense 上注册，然后在页面上添加广告，并进行优化。

<!-- more -->

## 注册

注册时忘记了截图，自行脑补吧，这里简单说明下注意事项。

注册时需要如实填写信息，尤其是国家信息，**一旦选择之后就不能再修改**，如果要修改，只能删除账号，然后用新的邮箱重新再进行注册一个 AdSenses 账号。

另外，地址信息也需要如实填写 (允许以后进行修改)，后面取钱的时候会通过该地址使用邮件 (对，就是纸制邮件) 对地址进行验证，里面会包含一个 PIN 码，

注册完成后，会出现一段代码，将这段代码添加到网站的 `<head>` 段中，然后，会出现如下的界面，需要等待审核，一般一到两天就可以审核完成，慢的话可能需要两周左右，如果不满足，需要修改后重新提交。

![jekyll adsense reviewing phase]({{ site.url }}/images/linux/jekyll-adsense-reviewing-phase.png "jekyll adsense reviewing phase")

如上，除了 Feedback 之外，其它的都是灰色的，只有审核通过之后才可以使用。

<!--
## 初审通过

初审通过后
-->

## 其它

### ads.txt

这个官方称为授权数字卖家(ads.txt)，是 Google 推出的用来提高广告透明度的计划，用户在可以从 AdSense 上下载对应的 `ads.txt` 文件，然后保存在网站的根目录下即可。

格式需要满足 IAB Tech Lab 的规范，一般格式如下。

```
google.com,pub-XXXXXXXXXXXXXXXX,DIRECT,f08c47fec0942fa0
```

总共包含了四个字段，其含义为：

1. 广告投放系统的域名，对于 Google 卖家账号，始终为 `google.com` ；
2. 发布商帐号ID，根上面域名关联的标识符，Google 的账号可以在 AdSense 系统中查看；
3. 帐号或关系的类型，不区分大小写，一般是 `DIRECT` 或者 `RESELLER` ；
4. 专属标识符，与字段一对应，Google 如上。

当添加完之后，可以通过 `https://yourdomain.com/ads.txt` 直接访问，Google 一般每隔一段时间扫描一次，大概两到三天后会取消提示。

### seller.json

同样遵循 IAB Tech Lab 的规范，用来公开卖家信息，提高广告生态系统的透明度。


{% highlight text %}
{% endhighlight %}
