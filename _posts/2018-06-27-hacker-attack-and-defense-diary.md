---
title: 【转】黑客攻防日记
layout: post
comments: true
language: chinese
category: [misc]
keywords: hacker
description: 一篇介绍如何进行密码交互的文章。
---

一篇介绍如何进行密码交互的文章。

<!-- more -->

### 小黑的日记 2010-6-22 晴

我最近发现了一个网站，是个博客平台，很火，大家都到那里去写注册账号，写日志。

我也好奇地去看了看，不过，我主要是想看看有什么漏洞没有，哈哈。

我发现他这个网站只用了HTTP协议，没用HTTPS，换句话说，所有的数据都是明文传输的，包括用户名和密码， 我觉得机会来了。

我尊敬的大哥给了我几台服务器的信息，他说通过这几台服务器能够偷窥，不，是监视发往博客平台服务器的HTTP数据，还能通过这几台服务器当个中间人，拦截修改请求和响应， 这让我喜出望外。

既然数据都是明文的，我就轻松地拿到了很多人的用户名和密码。更有意思的是，这些人的用户名和密码在很多平台都是一样的，这下次发财了！

我把用户名和密码都献给了大哥。

### 张大胖的日记 2010-6-23  阴

最近收到了不少投诉，都是说密码泄露的问题，我觉得不可能啊，因为我根本就没有明文存储密码！

有些人还不相信，我百口难辨。

在服务器的数据库里，我存的都是hash过的密码，为了防止黑客破解， 每个密码还都加了盐(salt)以后才做的存储。 密码怎么可能泄露？

![secret save]({{ site.url }}/images/linux/secret-save-01.png "secret save"){: .pull-center }

![secret save]({{ site.url }}/images/linux/secret-save-02.png "secret save"){: .pull-center }

想来想去，我觉得还是从浏览器到服务器这一段出了问题，肯定是有人在偷窥， 老子一定要加密！

我想用RSA这种非对称的加密方式（码农翻身注： RSA的详细解释戳 [这里](https://mp.weixin.qq.com/s?__biz=MzAxOTc0NzExNg==&mid=2665513779&idx=1&sn=a1de58690ad4f95111e013254a026ca2&chksm=80d67b70b7a1f26697fa1626b3e9830dbdf4857d7a9528d22662f2e43af149265c4fd1b60024&scene=21#wechat_redirect)）：

服务器生成public key 和 private key ， 浏览器用JavaScript使用public key 对密码加密， 然后服务器端用private key 进行解密。 这样，密码在传输过程中就不会被窃取了。

![secret save]({{ site.url }}/images/linux/secret-save-03.png "secret save"){: .pull-center }

### 小黑的日记  2010-6-24 多云

我突然发现，好多密码都被加密了！

我让大哥看了看，大哥说没有私钥，我是无法解密的， 不过他建议我当个中间人，也生成一对儿public key 和private key ， 对博客网站冒充是浏览器， 对浏览器冒充是博客网站。

我把我的public key 发给浏览器，浏览器把加密后的数据发给我，我用我的private key 解密， 就拿到了明文密码。

然后我还得用博客网站的public key 把密码加密，发给博客网站，让它浑然不知。

这件事难度不小，真是让人兴奋。

### 张大胖的日记， 2010-6-25 阴

MD, 密码还是泄露，气死老子了。

幸亏Bill提示我说可能有中间人， 我怎么忘了我和Bill 建立HTTPS的时候已经考虑到中间人这种情况了？

（码农翻身注： 《[一个故事讲完HTTPS](https://mp.weixin.qq.com/s?__biz=MzAxOTc0NzExNg==&mid=2665513779&idx=1&sn=a1de58690ad4f95111e013254a026ca2&chksm=80d67b70b7a1f26697fa1626b3e9830dbdf4857d7a9528d22662f2e43af149265c4fd1b60024&scene=21#wechat_redirect)》）

中间人难于防范，还得搞数字证书证明身份，如果我把这一套都搞好，岂不是又实现了一遍HTTPS？ 重复发明轮子！

要不我也上HTTPS ，一劳永逸地解决问题？ 但是我这是个小网站，搞个正规的、浏览器不会提示安全风险的证书也不便宜吧？

真是烦人！

### 小黑的日记， 2010-6-26 晴

当中间人真是爽！

### 张大胖的日记， 2010-6-27 小雨

老子决定不再折腾HTTPS了！

Bill建议我可以对浏览器发过来的密码加密， 其实也不是加密，而是做个Hash， Hash过的数据是不可逆的，不能恢复原始的明文密码。

浏览器端：

hash(password,salt) -> hash_password

然后浏览器把（username, hash_password） 发给服务器端。

服务器端：

从数据库获得之前保存的hash_password

和浏览器传递过来的hash_password比较，看看是否相等。

从此以后，网络上传输的都是所谓hash_password，再也看不到明文密码了， 让那帮偷窥的孙子们哭去吧！

ps : 在浏览器中进行hash的时候，有一个salt参数， 这个salt从哪里来？ 肯定是从服务器端获取的啊。

### 小黑的日记， 2010-6-28 晴

大哥猜对了，那家伙果然对密码做了hash，然后再发到服务器端，现在我难于获取明文密码了。

不过，那家伙还是留了一个大漏洞，既然我还能监听到 user_name, hash_password， 我就重新发送一下到服务器端，还是成功登陆这个博客系统了！ 哈哈！

张大胖的日记， 2010-6-29 中雨

焦头烂额！

这个浏览器端的hash操作没能发挥作用。今天研究了半天，才发现那些黑客可以重放攻击。

Bill说主要的原因还是salt固定导致的， 我决定再增加一点难度，增加一点动态的东西：验证码（captcha）！

用户登录的时候，发个验证码(captcha)到浏览器，这个验证码每次都不一样。

浏览器端：

第一次Hash:

hash(password,salt) -> hash_password1

第二次Hash:

hash(hash_password1,captcha)  -> hash_password2

然后浏览器把（username, hash_password2，captcha） 发给服务器端。

注意：hash_password1并不会发送给服务器， 黑客们窥探不到。

服务器端：

验证captcha 是否正确

使用username从数据库获得hash_password

hash(hashed_password,captcha)  --> hash_password3


比较hash_password2和hash_password3，看看是否相等。

如果相等，登录成功，否则，登录失败。

hash_password2是使用一次性的验证码生成的， 即使被那帮孙子截获，他也没法展开重放攻击，因为验证码已经失效了。


### 小黑的日记， 2010-6-30 阴天

那家伙越来越聪明了嘛，增加了验证码，老子的重放攻击也不管用了。

不过大哥真是威武，他告诉我要另辟蹊径，想办法攻击博客系统的数据库，只要把数据库拿到了，用暴力的方法破解它！

今晚就开始行动！

### 张大胖的日记， 2010-6-30 多云

Bill今天来了，他告诉我一件重要事情，不管你前端怎么加密，后端的安全措施一点都不能少！

我决定，把用户在浏览器中输入的密码做两次hash, 一次在浏览器端做，另外一次在服务器端做，把最终的结果作为密码存到数据库中。

浏览器端：

front_hash(password,salt) ->front_hash_password

发送(username,front_hash_password)到服务器端

服务器端：

back_hash(front_hash_password,salt) -> backend_hash_password

和数据库保存的hash_password做比较， 看看是否相等。

这么做的结果就是， 如果那帮孙子真的把我的数据库给偷走了，他们也很难通过撞库的方式破解其中的密码。

因为他们通常会把常用密码建立一个字典，然后通过hash算法生成hash值，如果这个hash值和待破解数据库的值相等，不就知道明文是什么了吗？

但是我的数据库中的密码是通过front_hash_password 作为明文密码，再次hash计算出来的。而那个front_hash_password本身就是个散列值，毫无规则可言。假如它是个32位的16进制字符串， 那将会有16^32 中组合，这是个天文数字， 这样字典也就难以建立了。

如果黑客把密码也按照我的方式，把常用的密码做两次hash呢？

这也不怕，Bill说可以把浏览器端的hash算法设置得复杂一点，增加破解的难度。 当然复杂的算法比较耗时， 但是Bill说了： 浏览器是分布的，完全可以充分利用他们的计算能力嘛！

Bill真是个聪明的家伙。

为了最终的安全，我想我还是上HTTPS吧！

### 小黑的日记， 2010-7-1 阴

终于把博客系统的数据库给拖下来了，可是怎么都破解不了。

大哥研究了一下说，这个网站可能用了复杂的Hash算法, 破解起来太耗时间了，放弃吧。

更加悲催的是，我发现这个网站开始使用HTTPS了，真的难以下手了。

我还是去看世界杯去吧。

<!--
https://mp.weixin.qq.com/s?__biz=MzAxOTc0NzExNg==&mid=2665514752&idx=1&sn=f6e3c65d85d92c7988d167d4d3401be4&chksm=80d67f43b7a1f655a79163eaa08156fd604957a61622930d8bf0bae3caf3ddd4808cf8538e82&scene=0#rd
-->




{% highlight text %}
{% endhighlight %}
