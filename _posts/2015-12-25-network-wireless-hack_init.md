---
Date: Auguest 05, 2015
title: Linux 无线网络
layout: post
comments: true
language: chinese
category: [linux, network]
---


<!-- more -->

{% highlight text %}
----- 查看网卡功能
# iw list

----- 查看wlp3s0的配置信息
# iw wlp3s0 info
Interface wlp3s0
    ifindex 3                      # 设备序号是3
    type managed                   # 连接类型，单点对AP的连接模式
    wiphy 0

----- 查看当前无线网络信号，包括网络名称(SSID)、信号强度、加密算法等
# iw dev wlp3s0 scan | less

----- 如果没有密码则直接连接
# iw dev wlp3s0 connect [SSID]
----- 如果网络是用WEP加密的，也非常容易
# iw dev wlp3s0 connect [SSID] key 0:[WEP-PASSWORD]

----- 通过DHCP获取本地IP，连接到无线网络
# dhcpcd wlp3s0
{% endhighlight %}

关于无线的配置，可以参考 iw 安装包，以及 [Linux Wireless](http://www.linuxwireless.org/en/users/Documentation/iw) 文档介绍；iw 是 iwconfig 的替换。

<!--
但网络使用的是 WPA 或 WPA2 协议的话，事情就不好办了。这种情况，您就得使用叫做 wpasupplicant 的工具，它默认是没有的。然后需要修改 /etc/wpasupplicant/wpa_supplicant.conf 文件，增加如下行：
network={    ssid="[网络 ssid]"    psk="[密码]"    priority=1}
我建议你在文件的末尾添加它，并确保其他配置都注释掉。要注意 SSID 和密码字串都是大小写敏感的。在技术上您也可以把接入点的名称当做是 SSID，使用 wpa_supplicant 工具的话会有合适的 SSID 来替代这个名字。
一旦配置文件修改完成后，在后台启动此命令：
$ sudo wpa_supplicant -i wlan0 -c /etc/wpa_supplicant/wpa_supplicant.conf
-->






# 常见操作

下面列举一下一些常见的操作。

## 网卡设置为 Monitor Mode

通常一些无线秘密的破解工具，如 aircrack-ng 需要将网卡设置为监听模式。可以直接使用该工具包中的一个 Bash 脚本 airmon-ng 进行设置。

{% highlight text %}
# airmon-ng start wlp3s0
{% endhighlight %}

当然，也可以手动将无线网卡设置为 Monitor Mode 。

{% highlight text %}
----- 0. 查看网卡是否支持Monitor模式，在Supported interface modes中查看
# iw list
Wiphy phy0
    ... ...
    Supported interface modes:
        * ... ...
        * monitor
        * ... ...

----- 1. 关闭无线网卡
# ip link set wlp3s0 down

----- 2.0 获取物理设备号
# ls -l "/sys/class/net/wlp3s0/phy80211" | sed 's/^.*\/\([a-zA-Z0-9_-]*\)$/\1/'
phy0
----- 2.1 其中phy0是如上的输出结果
# iw phy phy0 interface add wlp3s0mon type monitor
----- 2.2 查看设备
# ifconfig wlp3s0mon
wlp3s0mon: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
    ether ac:2b:6e:8b:42:28  txqueuelen 1000  (Ethernet)
    RX packets 0  bytes 0 (0.0 B)
    RX errors 0  dropped 0  overruns 0  frame 0
    TX packets 0  bytes 0 (0.0 B)
    TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

----- 3. 将wlp3s0mon设置为monitor mode
# ip link set wlp3s0mon down
# iw dev wlp3s0mon set type monitor
# ip link set wlp3s0mon up
# iw dev wlp3s0mon set channel 10
# iw dev wlp3s0 set freq 5825              # 也可以修改频率，可选

----- 4. 查看网卡的属性
# ifconfig wlp3s0mon
wlp3s0mon: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
    unspec AC-2B-6E-8B-42-28-F0-00-00-00-00-00-00-00-00-00  txqueuelen 1000  (UNSPEC)
    RX packets 150  bytes 47203 (46.0 KiB)
    RX errors 0  dropped 0  overruns 0  frame 0
    TX packets 0  bytes 0 (0.0 B)
    TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

# iw wlp3s0mon info
Interface wlp3s0mon
    ifindex 4
    wdev 0x2
    addr ac:2b:6e:8b:42:28
    type monitor
    wiphy 0

----- 5. 恢复原无线设备
# ip link set wlp3s0 up
{% endhighlight %}


## 无线密码破解

可以参考 [完全教程 Aircrack-ng破解WEP、WPA-PSK加密利器.pdf](/reference/linux/network/Aircrack_ng_WEP_WPA_PSK.pdf "完全教程 Aircrack-ng破解WEP、WPA-PSK加密利器.pdf")，以及《无线黑客傻瓜书》。


<br><br>
<br><br>
<br><br>
<br><br>
<br><br>

<!--
Ubuntu 14.04中碰到的wlan抓包问题
http://leave001.blog.163.com/blog/static/162691293201432544553241/
完全教程 Aircrack-ng破解WEP、WPA-PSK加密利器
http://netsecurity.51cto.com/art/201105/264844_all.htm
-->















{% highlight c %}
{% endhighlight %}
