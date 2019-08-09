---
title: CentOS 内核升级
layout: post
comments: true
language: chinese
category: [linux]
keywords: linux,centos,kernel,升级
description: CentOS 使用 yum 管理包以及升级内核，不过为了保持稳定，一般官方仓库提供的内核版本往往小于 Linux 的正式版本。这里简单介绍如何使用三方仓库进行升级。
---

CentOS 使用 yum 管理包以及升级内核，不过为了保持稳定，一般官方仓库提供的内核版本往往小于 Linux 的正式版本。

这里简单介绍如何使用三方仓库进行升级。

<!-- more -->

## 简介

这里使用 elrepo 仓库，详细可以参考 [www.elrepo.org](http://www.elrepo.org/) 中的相关介绍。

通过如下方式安装对应的仓库。

{% highlight text %}
# rpm --import https://www.elrepo.org/RPM-GPG-KEY-elrepo.org
# yum install https://www.elrepo.org/elrepo-release-7.0-4.el7.elrepo.noarch.rpm
{% endhighlight %}

然后，查看当前支持的内核包。

{% highlight text %}
# yum --disablerepo="*" --enablerepo="elrepo-kernel" list available
{% endhighlight %}

一般会有两类：A) `ml mainline` 最新的主线稳定内核；B) `lt longterm` 长期维护版。

然后选择需要的版本进行安装。

{% highlight text %}
# yum --enablerepo=elrepo-kernel install kernel-lt
{% endhighlight %}

## Invalid Signature

一般是在安装了三方内核可能会出现这一问题，详细处理方式可参考 [SecureBootKey](http://www.elrepo.org/)。

{% highlight text %}
error: /vmlinuz-xxx-xxx has invalid signature.
error: you need to load the kernel first

Press any key to continue...
{% endhighlight %}

不过，感觉 elrepo 仓库中的内核都没有签名，一般通过 hexdump 之后，在末尾会有签名的信息，一般是证书的信息。

所以，还是将 `Secure Boot` 关闭掉吧。

### Secure Boot

如果使用了 Unified Extensible Firmware Interface, UEFI (用于替换传统的 BIOS)，那么 Secure Boot 会默认开启，就可能会导致启动失败，最简单的方式是直接关闭。

其目的是为防止软件的恶意入侵，采用公私钥认证方式，在 UEFI 出厂时会内置一部分公钥，那么加载操作系统、硬件驱动程序都必须通过公钥的认证。

但是，UEFI 并没有规定那些公钥是合法的、由谁颁发，这都由硬件厂商所决定，例如 Windows 就强制要求包含其公钥。

### 处理方式

简单来说，就是需要将公钥添加到 Machine Owner Key, MOK 列表中，可以通过如下方式下载证书文件。

{% highlight text %}
$ wget https://elrepo.org/SECURE-BOOT-KEY-elrepo.org.der
{% endhighlight %}

不过，实际上在安装 `elrepo-release` 时，已经在 `/etc/pki/elrepo` 中包含了证书。

然后直接导入即可。

{% highlight text %}
# mokutil --import /etc/pki/elrepo/SECURE-BOOT-KEY-elrepo.org.der
{% endhighlight %}

### 其它

注意，很多的 mokutil 操作是需要输入密码的，是用于在 UEFI 执行真正删除时进行确认。

如下是关于 MOK 的常用操作方式。

{% highlight text %}
----- 查看是否开启
# mokutil --sb-state

----- 所有已经注册的密钥
# mokutil --list-enrolled

----- 删除证书，以及查看接着会删除的证书，需要密码
# mokutil --delete /etc/pki/elrepo/SECURE-BOOT-KEY-elrepo.org.der
# mokutil --list-delete
{% endhighlight %}

<!--
# keyctl list %:.system_keyring
-->

如果没有了证书，那么可以如下的方式删除。

{% highlight text %}
----- 导出所有证书，不过只有序号
# mokutil --export

----- 确认所需要删除证书对应的序号
# mokutil --list-enrolled

----- 选择对应的证书删除
# mokutil --delete MOK-0001.der
{% endhighlight %}

## grub

直接修改默认配置 `/etc/default/grub` ，将 `GRUB_DEFAULT` 修改为对应的序号 (从0开始)，也可以设置为 `saved`，此时会加载上次的选择。

启动加载时显示的顺序可以修改 `/etc/grub2-efi.cfg` 或者 `/etc/grub2.cfg` ，前者是对于 UEFI 的支持，后者是老的 BIOS 的配置。

{% highlight text %}
{% endhighlight %}
