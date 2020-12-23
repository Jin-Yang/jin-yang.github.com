---
title: 给 Jekyll Blog 添加 AdSense 广告
layout: post
comments: true
language: chinese
tag: [Blog,Others]
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

![jekyll adsense reviewing phase](/{{ site.imgdir }}/linux/jekyll-adsense-reviewing-phase.png "jekyll adsense reviewing phase")

如上，除了 Feedback 之外，其它的都是灰色的，只有审核通过之后其它的功能才可以使用，快的话几天就可以搞定，慢可能就需要两周左右了。

## 添加广告

初审通过后，就可以按照操作进入到添加广告的页面了。

如下是一个内容广告的示例，直接添加到需要放置广告的地方即可。

{% highlight text %}
<script async src="https://pagead2.googlesyndication.com/pagead/js/adsbygoogle.js"></script>
<ins class="adsbygoogle"
     style="display:block;"
     data-ad-layout="in-article"
     data-ad-format="fluid"
     data-ad-client="ca-pub-5925708324262028"
     data-ad-slot="8157274195"></ins>
<script>
     (adsbygoogle = window.adsbygoogle || []).push({});
</script>
{% endhighlight %}

可以根据需求修改上述的 `style` 内容，例如 `style="display:block; width:90%; margin:0 auto; opacity:80%;` ，也就是宽度为 `90%`、中间对齐、降低透明度，也可以通过如下方式固定高度或者宽度。

{% highlight text %}
min-width:758px;max-width:1080px;width:100%;height:90px;
min-height:90px;max-height:120px;height:100%;
{% endhighlight %}

另外，在移动端打开页面时，默认广告的宽度会占据整个移动设备的屏幕宽度，而非占用的 `<div>` 元素，可以添加 `data-full-width-responsive="false"` 。

<!--
注意，`<ins>` 是要每个需要广告的地方都要添加的，如果在网页上添加了多个广告，那么就只需要保留一份 `<script>` 中的内容即可，而且可以将最后添加广告的语句在加载完页面后执行，例如。

{% highlight text %}
<script type="text/javascript">
window.onload = function() {
  (adsbygoogle = window.adsbygoogle || []).push({});
}
</script>
{% endhighlight %}
-->

{% include ads_content01.html %}

## 优化建议

影响到广告收入的因素，核心包括了三个：点击率、展示量、广告单价，其中常见的有几个指标：

* Click-Through-Rate, CTR 点击通过率，计算方式为广告的实际点击次数除以广告的展示次数。
* Reveal Per Mille, RPM 每千次广告展示的收入。
* Cost Per Click, CPC 每次点击付费，当某位浏览者点击你网站显示的 Google 广告时，Google 将向你支付的金额。

提高 CTR 无非就是从广告内容、广告位置出发，据说有人统计 CTR 的大致展示位置排序有：`相关内容 > 文章内嵌 > 文章尾部 > 标题下方 > 标题上方 > 侧栏` 。


<!--
https://zhuanlan.zhihu.com/p/69210606
屏蔽广告
https://justyy.com/archives/676
高价关键词
https://www.moidea.info/archives/seo-and-adsense-high-price-keyword-research-analysis-tool.html
http://drupals.cn/node/73
-->

## 其它

IAB 发布了很多个标准，其目的就是为了建立一个良好的广告生态系统，在 2017 的是否发布到了 ads.txt 规范，而在 2019 发布了 Sellers.json 和  OpenRTB Supply Chain Object ，三者的目的相同，只是方法略有区别。

> Interactive Advertising Bureau, IAB 是一个非盈利的广告业务组织，负责制定行业标准，进行研究并为在线广告行业提供法律支持。

再详细点，现在的广告大部分都是在  在程序化广告上下游生态环境中，为防止广告库存在中间商层面可能发生的作弊行为，出现了一些辅助的校验手段和机制，帮助终端买方在购买决策前，能够对广告库存的源头、中间各级供应链进行校验。

![google adsense IAB ads.txt seller.json info](/{{ site.imgdir }}/linux/google-adsense-iab-ads.txt-seller.json-info.jpg "google adsense IAB ads.txt seller.json info")

如上是其大致的工作流程，不过详细的细节以及工作原理还不是太清楚，所以这里仅简单介绍下如何进行配置。

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

同样遵循 IAB Tech Lab 的规范，用来公开卖家信息，提高广告生态系统的透明度，其内容类似如下。

{% highlight text %}
{
  "contact_email": "Your E-Mail Address Info", 
  "contact_address": "Your Address Info", 
  "version": "1.0", 
  "sellers": [{
    "seller_id":"pub-5925708324262028",
    "is_confidential":0,
    "name":"Your Name",
    "seller_type":"PUBLISHER",
    "domain":"gohalo.me"
  }]
}
{% endhighlight %}

在 Google 的帮助文档上有介绍如何填写 `sellers` 中的字段，以及注意事项，例如 `seller_id` 就是你申请的 Adsense 的账号 ID ，而 `domain` 不要使用 `www.domain.com` 类似的，而是使用 `domain.com` 等等，这里就u不再一一介绍了。

### 注意事项

如果直接使用类似 `foobar.github.io` 的域名，目前是无法申请的，需要自己注册一个域名。

另外，根据 Google Adsense 的条例是不能私自点击广告的，如果被检查出来的话可能会被禁止使用，但是在调试时可能会被误点，在 Chrome 浏览器中可以安装 Google Publisher Toolbar 插件。


<!--
https://davidsekar.com/website-monetization/google-adsense-error-no-slot-size-for-availablewidth
https://www.cnblogs.com/cocowool/p/google_adsense_no_slog_size_error.html
-->

{% highlight text %}
{% endhighlight %}
