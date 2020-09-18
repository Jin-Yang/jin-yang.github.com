---
Date: October 19, 2013
title: Linux 网络命名空间
layout: post
comments: true
language: chinese
category: [linux, network]
---


<!-- more -->





在 IP 网段中有 A B C 三个网段的私有 IP 可以使用，分别是：10.0.0.0 ~ 10.255.255.255、172.16.0.0 ~ 172.31.255.255、192.168.0.0 ~ 192.168.255.255 。

上图是 docker 中的一个网络设置，其使用了一个私有网段，172.40.1.0/24，还可能会使用 10.0.0.0 和 192.168.0.0 这两个私有网段，关键看你的路由表中是否配置了，如果没有配置，就会使用，如果你的路由表配置了所有私有网段，那么 docker 启动时就会出错了。

当启动一个 Docker 容器后，可以使用 ip link/addr show 来查看当前宿主机的网络情况，通常有一个 docker0 以及 vethXXXXX 的虚拟网卡 (给容器用) 。




## 创建一个 network namespace

建立一个基本的网络命名空间。

{% highlight text %}
----- 新建的NET-NS会保存在该目录下，如果没有直接新建目录，非必须
master# mkdir -p /var/run/netns

----- 增加一个network namespace名称为foons
master# ip netns add foons

----- 查看当前所有的网络命名空间，或者查看/var/run/netns目录
master# ip netns list

----- 激活foons中的loopback设备
master# ip netns exec foons ip link set dev lo up

----- 在新的命名空间中执行bash，这样就可以直接查看网络设备信息等
master# ip netns exec foons bash

----- 在新的命名空间查看设备
foons# ip link show
{% endhighlight %}

其中 ip netns exec NS CMD 命令可用于在一个网络命名空间中执行响应的命令。

接下来新建一对新的虚拟网卡，其中一个给搁到上面新建的网络命名空间中。

{% highlight text %}
----- 新建两个虚拟网卡，分别为veth0和foo-veth0
master# ip link add veth0 type veth peer name foo-veth0

----- 查看新建的虚拟网卡
master# ip link show

----- 将其中的foo-veth0放到新建的命名空间foons中
master# ip link set foo-veth0 netns foons

----- 查看foons中的网络设备
master# ip netns exec foons ip link list

----- 把容器里的foo-veth0改名为eth0
master# ip netns exec foons ip link set dev foo-veth0 name eth0
{% endhighlight %}

在网络命名空间中查看，除了上述 ip netns 方法之外，还可以在上述执行 bash 之后，直接在 bash 中查看，同样是执行 ip link show 查看。

接下来激活网络设备。

{% highlight text %}
----- 激活网络命名空间中的网络设备
master# ip netns exec foons ifconfig eth0 172.18.0.20/16 up
master# ifconfig veth0 172.18.0.21/16 up

----- 或者分为两步，同上
master# ip netns exec foons ip addr add 172.18.0.20/16 dev eth0
master# ip netns exec foons ip link set eth0 up
{% endhighlight %}

接下来添加一个网桥，将 foons 的网络设备与真实的网络设备链接起来。

{% highlight text %}
----- 增加一个网桥foobr0
master# brctl addbr foobr0

----- 暂时先关闭网桥
master# brctl stp foobr0 off

----- 为网桥设置IP地址，并启动
master# ifconfig foobr0 172.18.0.1/24 up

----- 将veth0添加到网桥上
master# brctl addif foobr0 veth0

----- 为容器增加一个路由规则，让容器可以访问外面的网桥
master# ip netns exec foons ip route add default via 172.18.0.1
{% endhighlight %}


目前，可以从容器中访问到网桥，但是不能访问到外部网络，如果要访问外部网络，还需要添加 NAT 规则。

{% highlight text %}
----- 地址转换，从而可以访问外部的网络，对于源地址是127.18.0.0/16且目的地址不是172.18.0.0/16的IP进行转换
master# iptables -t nat -A POSTROUTING -s 172.18.0.0/16 ! -d 172.18.0.0/16 -j MASQUERADE

----- 查看当前设置的规则
master# iptables -nL POSTROUTING -t nat --line-number

----- 假设对应的是第2条记录，可以通过如下方式删除记录
master# iptables -D POSTROUTING 2
{% endhighlight %}



iptables -t nat -A POSTROUTING -s 172.18.0.0/16 -o foobr0 -j SNAT --to-source 192.168.1.10













.2 举例新增zabbix端口的映射：
3.2.1 单IP单容器端口扩容：
iptables -t nat -A PREROUTING  -p tcp -m tcp --dport 10050 -j DNAT --to-destination  172.17.0.3:10050
iptables -t nat -A PREROUTING  -p tcp -m tcp --dport 10051 -j DNAT --to-destination  172.17.0.3:10051

3.2.2 单IP多容器端口扩容：
iptables -t nat -A PREROUTING  -p tcp -m tcp --dport 50010 -j DNAT --to-destination  172.17.0.3:10050
iptables -t nat -A PREROUTING  -p tcp -m tcp --dport 50011 -j DNAT --to-destination  172.17.0.3:10051
#另一个容器则可以规划为60010,60011，这样在zabbix监控的时候，就需要指定客户容器的端口连接了。

3.2.3 多IP多容器端口扩容：
iptables -t nat -A PREROUTING -d  10.18.103.2 -p tcp -m tcp --dport 10050 -j DNAT --to-destination 172.17.0.3:10050
iptables -t nat -A PREROUTING -d  10.18.103.2 -p tcp -m tcp --dport 10051 -j DNAT --to-destination 172.17.0.3:10051
#iptables -t nat -A PREROUTING -d  10.18.103.3 -p tcp -m tcp --dport 10050 -j DNAT --to-destination 172.17.0.4:10050
#iptables -t nat -A PREROUTING -d  10.18.103.3 -p tcp -m tcp --dport 10051 -j DNAT --to-destination 172.17.0.4:10051
#这样zabbix连接10.18.103.2，3的正常zabbix端口就可以了。


