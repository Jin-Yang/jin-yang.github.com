---
title: KVM 虚拟平台
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---

全虚拟化最简单通俗的理解是，在一台机器上可以运行不同的平台 (例如Linux、Windows等)，不过其对应的仍然是 x86_64 架构。另外，如果允许在机器上可以运行不同的架构(例如x86_64、PowerPC、ARM等)，那么这种的虚拟化方式称为硬件仿真。

这里简单介绍下 CentOS 中 KVM 的使用方式。

<!-- more -->

## 简介

Kernel-based Virtual Machine, KVM 是一款基于 Linux 内核的虚拟机，安装了 `kvm.ko` 内核模块后，直接将 Linux 内核改变成了一个 `Hypervisor`，此时宿主机上仍然可以运行进程，同时可以创建虚拟机，不过每个虚机就是运行在 Linux 内核之上的一个系统进程，可以随时使用 `kill` 命令杀死。

最初，Linux 内核只有用户、内核态，在引入了 KVM 模块之后，同时引入了 `guest` 模式，此模式只负责非 `IO` 类操作或者说是非特权指令操作，如算术运行。

在 Guest 模式中，可以直接运行到物理 CPU 的 `ring 3` 上，但一旦有 IO 调用或者需要特权指令，就需要从用户态到内核态的调用。

![kvm qemu]({{ site.url }}/images/linux/kvm-qemu-libvirtd-arch.jpg "kvm qemu"){: .pull-center width="90%" }

### KVM

KVM 是 Linux 的内核模块，需要 CPU 的支持，采用硬件辅助虚拟化技术 Intel-VT 或者 AMD-V，内存的相关如 Intel-EPT 和 AMD-RVI 技术，然后 Guest OS 的 CPU 指令不用再经过 Qemu 转译，直接运行，大大提高了速度。

KVM 通过 `/dev/kvm` 暴露接口，用户态程序可以通过 `ioctl()` 函数来访问这个接口。

注意，KVM 内核模块本身只提供了 CPU 和内存的虚拟化，所以它必须结合 QEMU 才能构成一个完成的虚拟化技术。

### QEMU-KVM

如上，但 KVM 不能模拟其它设备，通过 QEMU 模拟 IO 设备，包括网卡、磁盘等，这样 KVM 加上 QEMU 之后就能实现真正意义上服务器虚拟化。

### libvirt

目前使用最为广泛的对 KVM 虚拟机进行管理的工具和 API，通过 libvirtd 的后台程序，可以被本地的 virsh 调用。

### 总结

各个模块调用流程如下。

![kvm qemu]({{ site.url }}/images/linux/kvm-qemu-libvirtd-process.jpg "kvm qemu"){: .pull-center width="90%" }

在运行的时候，包括了内核态和用户态，内核模块主要负责虚拟机的创建、虚拟内存分配、VCPU 寄存器的读写以及 VCPU 的运行；而 QEMU 用于模拟虚拟机的用户空间组件，提供 IO 设备模型，访问外设的途径。

KVM 所使用的方法是通过简单地加载内核模块而将 Linux 内核转换为一个系统管理程序，该模块会导出了一个名为 `/dev/kvm` 的设备，可以启用内核的 `guest` 模式。

通过 `/dev/kvm` 设备，可以使 VM 的地址空间独立于内核以及其 VM 的地址空间。

## 安装

可以通过如下方式安装配置 CentOS 虚拟机。

### CPU 支持

KVM 是基于 x86 虚拟化扩展 (Intel VT 或者 AMD-V) 技术的虚拟机软件，所以查看 CPU 是否支持 VT 技术，就可以判断是否支持 KVM 。

如果结果中有 vmx(Intel) 或 svm(AMD) 字样，就说明 CPU 支持。

{% highlight text %}
$ egrep -q 'vmx|svm' /proc/cpuinfo && echo yes || echo no
{% endhighlight %}

为了避免不必要的权限错误，可以暂时关闭 SELinux，也就是将 `/etc/sysconfig/selinux` 配置文件中的 `SELinux=enforcing` 修改为 `SELinux=disabled` 。

### 安装软件包

安装如下基础软件包，其中 CentOS7 需要安装 `bridge-utils` 包。

{% highlight text %}
# yum -y install qemu-kvm libvirt virt-install
{% endhighlight %}

其中各个包的使用用途如下。

* `qemu-kvm` 主要的 KVM 程序包；
* `virt-install` 基于 libvirt 服务的虚拟机创建命令；
* `libvirt` 工具包，提供 libvirt 服务；
* `libvirt-client` 为虚拟客户机提供的 C 语言工具包；
* `virt-top` 虚拟机统计命令，可以显示当前虚拟机的资源使用情况；
* `bridge-utils` 创建和管理桥接设备的工具；

<!--
* `python-virtinst` 创建虚拟机所需要的命令行工具和程序库；
virt-manager GUI虚拟机管理工具
virt-viewer GUI连接程序，连接到已配置好的虚拟机
-->

确认内核是否加载了 KVM 模块，如果没有，可以使用 `modprobe kvm` 来加载。

{% highlight text %}
# lsmod |grep kvm
kvm_intel             170181  0 
kvm                   554609  1 kvm_intel
irqbypass              13503  1 kvm
{% endhighlight %}

然后启动 `libvirtd` 后台服务。

{% highlight text %}
# systemctl start libvirtd
{% endhighlight %}

最后检查环境是否准备好。

{% highlight text %}
# virt-host-validate
{% endhighlight %}

### 安装 CentOS 7

从官方下载相关的安装包，然后执行如下命令。

{% highlight text %}
# virt-install --virt-type=kvm --name=centos7 --vcpus=2 --memory=4096 \
    --location=CentOS-7-x86_64-Minimal-1908.iso                       \
    --disk path=centos7.qcow2,size=40,format=qcow2                    \
    --network bridge=virbr0 --graphics none --extra-args='console=ttyS0' --force
{% endhighlight %}

常用参数如下。

{% highlight text %}
-n NAME, --name=NAME
    虚拟机名称，需全局惟一
-r MEMORY, --ram=MEMORY
    内存大小，单位为MB
--vcpus=VCPUS[,maxvcpus=MAX][,sockets=#][,cores=#][,threads=#]
    CPU个数及相关配置
--cpu=CPU
    CPU模式及特性，可以使用qemu-kvm -cpu ?来获取支持模式
--os-type=DISTRO_TYPE
    操作系统类型，如linux、unix或windows等

-c CDROM, --cdrom=CDROM
    光盘安装介质
-l LOCATION, --location=LOCATION
    安装源URL，支持FTP、HTTP及NFS等，如ftp://172.16.0.1/pub
--pxe
    基于PXE完成安装
--livecd
    把光盘当作LiveCD


--disk=DISKOPTS
    存储设备及其属性；格式为--disk /some/storage/path,opt1=val1,opt2=val2
    常用选项有：
        device 设备类型，如cdrom、disk或floppy等，默认为disk；
        bus    磁盘总结类型，其值可以为ide、scsi、usb、virtio或xen；
        perms  访问权限，如rw、ro或sh(共享的可读写)，默认为rw；
        size   新建磁盘映像的大小，单位为GB；
        cache  缓存模型，其值有none、writethrouth(缓存读)及writeback(缓存读写)；
        format 磁盘映像格式，如raw、qcow2、vmdk等；
        sparse 磁盘映像使用稀疏格式，即不立即分配指定大小的空间；


-w NETWORK, --network=NETWORK,opt1=val1,opt2=val2
    将虚拟机连入宿主机的网络中，其中NETWORK可以为
    bridge=BRIDGE 连接至名为"BRIDEG"的桥设备
    network=NAME  连接至名为"NAME"的网络


-v, --hvm
    当物理机同时支持完全虚拟化和半虚拟化时，指定使用完全虚拟化
-p, --paravirt
    使用半虚拟化
--virt-type
    使用的hypervisor，如kvm、qemu、xen等，可选列表通过virsh capabilities查看

--graphics TYPE,opt1=val1,opt2=val2
    图形显示相关配置，不会配置硬件，仅指定虚拟机启动后对其进行访问的接口
    TYPE 显示类型，可以为vnc、sdl、spice或none等，默认为vnc:port
{% endhighlight %}

### 网络配置

修改网络配置，其中 IP 需要与上述配置的网桥地址相同。

{% highlight text %}
TYPE=Ethernet
BOOTPROTO=static
IPADDR=192.168.120.200
PREFIX=24
GATEWAY=192.168.120.1
DEFROUTE=yes
PEERDNS=yes
PEERROUTES=yes
IPV4_FAILURE_FATAL=no
IPV6INIT=yes
IPV6_AUTOCONF=yes
IPV6_DEFROUTE=yes
IPV6_PEERDNS=yes
IPV6_PEERROUTES=yes
IPV6_FAILURE_FATAL=no
NAME=eth0
UUID=adfa3b7d-bf60-47e6-8482-871dee686fb5
DEVICE=eth0
ONBOOT=yes
{% endhighlight %}

然后，通过 `ifup eth0` 命令激活网卡。

## 常用命令

每个虚拟机会在 `/etc/libvirt/qemu` 目录下生成一个配置文件，包括了 CPU、磁盘的配置信息等，可以直接进行修改，或者通过 `virsh edit XXX` 修改。

{% highlight text %}
----- 确认各个版本号
# kvm --version
# virt-install --version
# virsh --version

----- 查看虚拟机列表，需要root执行，通过--all查看所有，--inactive未运行虚拟机
# virsh list
  Id    Name                           State
 ----------------------------------------------------
  7     centos7                        running

----- 连接到虚拟机
# virsh console centos7

----- 虚拟机信息
# virsh dominfo <VIRT-MACHINE>

----- 虚拟机的分区信息
# virt-df <VIRT-MACHINE>


----- 删除虚拟机
# virsh shutdown centos7      # 关机
# virsh destroy centos7       # 强制关机
# virsh undefine centos7      # 只删除配置文件，磁盘文件未删除

----- 其它操作
# virsh start centos7         # 开机
# virsh reboot centos7        # 重启
# virsh suspend centos7       # 挂起
# virsh resume centos7        # 恢复
{% endhighlight %}




## 源码解析

其中 KVM 的源代码已经包含在了 Linux 的内核树中，可以直接从 [www.kernel.org](https://www.kernel.org/) 下载代码即可。

涉及到 KVM 的主要有两个目录，公共实现相关 `virt/kvm` 和平台相关 `arch/x86/kvm`，其中后者的 `Makefile` 列表中包含了那些源码文件会编译成内核模块。

内核代码中的 `$(CONFIG_KVM)` 是预编译条件，如果在 `Linux` 内核配置时配置了该选项，则 `$(CONFIG_KVM)` 会通过预编译替换成 `y`，对应的文件也会被编译进内核中。

从上述 `kvm.ko` 所包含的对象可以发现，主要包含了 `IO` 操作、`MMU` 内存操作、`IRQ` 终端操作、`APIC` 高级电源管理和 `Timer` 定时器操作等，其实这也是 KVM 所要处理的主要工作。

另外，单独通过一个 KVM 内核模块并不能实现真正的虚拟机，还需要通过 QEMU 来模拟大量外设才行。

<!--
###

QEMU 通过 `ioctl()` 向设备驱动来发送创建、运行虚拟机命令，设备驱动 kvm 就会来解析命令 (对应 `kvm_dev_ioctl()[kvm_main.c]`)，包括了创建、启动、停止等。

很不错的源码介绍，KVM源代码分析1:基本工作原理
http://blog.51cto.com/koumm/1288795
http://abcdxyzk.github.io/blog/2015/07/29/kvm-src1/
-->


## 其它

### kvmtools

如上所述，KVM 只是一个内核模块，要构建一个完整的虚机需要同时模拟外设，其中使用比较多的是 QEMU 。除此之外，还有一个较小的工具 [KVMTool](https://github.com/kvmtool/kvmtool)，用来实现轻量级的虚拟机。

代码比较少，不过质量较高，麻雀虽小，但五脏俱全，但是其只支持 Linux 客户机。


## 网络


{% highlight text %}
----- 当前活跃的网络
# virsh net-list
 Name                 State      Autostart     Persistent
----------------------------------------------------------
 default              active     yes           yes

----- 该网络详细配置
# virsh net-dumpxml default
<network>
  <name>default</name>
  <uuid>19a8f652-9c33-4de0-b1e3-97ec8cd8d6eb</uuid>
  <forward mode='nat'>
    <nat>
      <port start='1024' end='65535'/>
    </nat>
  </forward>
  <bridge name='virbr0' stp='on' delay='0'/>
  <mac address='52:54:00:63:39:92'/>
  <ip address='192.168.122.1' netmask='255.255.255.0'>
    <dhcp>
      <range start='192.168.122.2' end='192.168.122.254'/>
    </dhcp>
  </ip>
</network>

----- 编辑修改default网络的配置
# virsh net-edit default

----- 将default网络干掉，再重新定义
# virsh net-undefine default

----- 重新创建default.xml文件，指定某个mac地址对应的ip，以及ip段
# cat default.xml
<?xml version="1.0" encoding="utf-8"?>
<network>
  <name>default</name>
  <uuid>dc69ff61-6445-4376-b940-8714a3922bf7</uuid>
  <forward mode="nat"/>
  <bridge name="virbr0" stp="on" delay="0"/>
  <mac address="52:54:00:81:14:18"/>
  <ip address="192.168.122.1" netmask="255.255.255.0">
    <dhcp>
      <range start="192.168.122.2" end="192.168.122.254"/>
      <host mac="00:25:90:eb:4b:bb" name="guest1" ip="192.168.5.13"/>
      <host mac="00:25:90:eb:34:2c" name="guest2" ip="192.168.7.206"/>
      <host mac="00:25:90:eb:e5:de" name="guest3" ip="192.168.7.207"/>
      <host mac="00:25:90:eb:7e:11" name="guest4" ip="192.168.7.208"/>
      <host mac="00:25:90:eb:b2:11" name="guest5" ip="192.168.7.209"/>
    </dhcp>
  </ip>
</network>
# virsh net-define default.xml

----- 使其生效
# virsh net-start default
{% endhighlight %}

其它配置可以参考 [Network XML Format](https://libvirt.org/formatnetwork.html) 。


## 问题排查

### 已经在使用

报错信息为 `Guest Is already in use` ，一般是在安装完虚拟机之后的报错。

和 XEN 的半虚拟化不同，对于 XEN 只需要删除配置文件即可，而 KVM 需要执行如下的命令。

{% highlight text %}
# virsh undefine <VIRT-NAME>
{% endhighlight %}

### 权限问题

默认使用 qemu 用户创建相关文件，所以需要确保目录权限，可以修改 `/etc/libvirt/qemu.conf` 配置文件的如下内容。

{% highlight text %}
user = "root"
group = "root"
{% endhighlight %}

然后重启 `systemctl restart libvirtd` 。

### 环境检查

{% highlight text %}
----- 检查虚拟机环境是否准备好
# virt-host-validate
{% endhighlight %}

可能会出现如下的错误 `IOMMU appears to be disabled in kernel` ，此时需要通过如下方式进行设置。修改配置 `/etc/default/grub` 中 `GRUB_CMDLINE_LINUX` 选项，添加 `intel_iommu=on` 内容。

接着通过 `grub2-mkconfig -o /boot/grub2/grub.cfg` 更新 grub2 ，然后重启。


<!--
https://github.com/jaywcjlove/handbook/blob/master/CentOS/CentOS7%E5%AE%89%E8%A3%85KVM%E8%99%9A%E6%8B%9F%E6%9C%BA%E8%AF%A6%E8%A7%A3.md
-->

{% highlight text %}
{% endhighlight %}
