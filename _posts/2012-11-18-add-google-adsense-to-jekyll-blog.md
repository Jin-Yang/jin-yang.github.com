---
title: 给 Jekyll Blog 添加 AdSense 广告
layout: post
language: chinese
category: [misc]
keywords: jekyll,blog,adsense,广告
description:
---


<!-- more -->

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


<!--
https://www.google.com/adsense/cancel-account?token=AEHiP_ChxsHGXi0hoSSA3a-vFYxSvWxrz_5snBJzGQ6I68c9Lve3AIDqjwybQIdt_LJjWh9QbKNKwYi5ZK7r1bH-s2U8WspVVA&pid=pub-4155595889991177
-->



{% highlight text %}
{% endhighlight %}
