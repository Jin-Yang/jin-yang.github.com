---
title: K8S 使用简介
layout: post
comments: true
language: chinese
category: [program,linux]
keywords: c,coverage
description:
---


<!-- more -->

## MiniKube

需要安装 [kubectl](https://github.com/kubernetes/kubernetes/blob/master/CHANGELOG.md) ，选择对应的版本。



<!--
https://storage.googleapis.com/kubernetes-release/release/v1.15.4/bin/linux/amd64/kubectl


https://docker.mirrors.ustc.edu.cn/

https://github.com/AliyunContainerService/minikube/releases
http://kubernetes.oss-cn-hangzhou.aliyuncs.com/minikube/releases/v1.3.1/minikube-linux-amd64
-->



{% highlight text %}
----- 删除现有虚机以及目录缓存文件
# minikube delete
# ~/.minikube
{% endhighlight %}


如果使用 `--vm-driver=none` 参数，实际上会在本地生成一些配置文件。

{% highlight text %}
# minikube start --vm-driver=none --registry-mirror=https://docker.mirrors.ustc.edu.cn/
{% endhighlight %}

此时会通过 `sudo` 启动 docker 服务，所以需要确保 CentOS 中安装了 sudo 包。

Downloading kubeadm v1.15.2
Downloading kubelet v1.15.2



添加阿里云的仓库地址，然后安装 ``  即可。

{% highlight text %}
# cat <<EOF > /etc/yum.repos.d/kubernetes.repo
[kubernetes]
name=Kubernetes
baseurl=https://mirrors.aliyun.com/kubernetes/yum/repos/kubernetes-el7-x86_64/
enabled=1
gpgcheck=0
repo_gpgcheck=0
EOF
{% endhighlight %}

修改系统参数的配置。

{% highlight text %}
----- 临时禁用selinux，可以修改/etc/sysconfig/selinux永久关闭
# setenforce 0
# sed -i 's/SELINUX=permissive/SELINUX=disabled/' /etc/sysconfig/selinux

----- 临时关闭SWAP，如需永久关闭注释/etc/fstab文件里swap相关行
# swapoff -a

----- 配置转发规则
# cat <<EOF >  /etc/sysctl.d/k8s.conf
net.bridge.bridge-nf-call-ip6tables = 1
net.bridge.bridge-nf-call-iptables = 1
EOF
# sysctl --system


systemctl start docker
systemctl start kubelet
{% endhighlight %}


### kubeadm

这是 Kubernetes 的集群安装工具，在通过 init 命令初始化时，会通过 `k8s.gcr.io` 镜像仓库初始化，不过国内无法访问。

{% highlight text %}
----- 查看需要那些镜像
# kubeadm config images list
k8s.gcr.io/kube-apiserver:v1.16.0
k8s.gcr.io/kube-controller-manager:v1.16.0
k8s.gcr.io/kube-scheduler:v1.16.0
k8s.gcr.io/kube-proxy:v1.16.0
k8s.gcr.io/pause:3.1
k8s.gcr.io/etcd:3.3.15-0
k8s.gcr.io/coredns:1.6.2


kubeadm init --image-repository="gcr.azk8s.cn/google_containers"

http://mirror.azure.cn/help/gcr-proxy-cache.html
{% endhighlight %}


<!--
kubeadm config images pull

http://www.ruanyifeng.com/blog/2018/02/docker-tutorial.html
https://www.mdslq.cn/archives/5e6f338.html



cubectl
https://github.com/kubernetes/kubernetes/blob/master/CHANGELOG.md
https://yq.aliyun.com/articles/221687
https://www.jianshu.com/p/18441c7434a6
https://ehlxr.me/2018/01/12/kubernetes-minikube-installation/
https://blog.csdn.net/qq_26188449/article/details/77543093

通过cobra进行命令行的处理
https://github.com/spf13/cobra
https://www.cnblogs.com/sparkdev/p/10856077.html

源码解析
https://juejin.im/post/5b29c0d5e51d45588821399a
http://www.10tiao.com/html/356/201706/2247484527/1.html

runStart() 在start时实际调用的函数

/post/golang-basic-package-introduce.html
从 v1.5 开始开始引入 vendor 包模式，如果项目目录下有 vendor 目录，那么 go 工具链会优先使用 vendor 内的包进行编译、测试等。

实际上，这之后第三方的包管理思路都是通过这种方式来实现，比如说由社区维护准官方包管理工具 dep ，不过官方不认可。

在 v1.11 中加入了 Go Module 作为官方包管理形式，在 v1.11 和 v1.12 版本中 gomod 不能直接使用，可以执行 `go env` 命令查看是否有 GOMOD 判断是否已开启。

如果没有开启，可以通过设置环境变量 `export GO111MODULE=on` 开启。

----- 查看所有依赖
go list -u -m all

当使用 modules 时，会完全忽略原有的 vendor 机制。

## sync 扩展

官方的 sync 包，提供了基础的 Map、Mutex、WaitGroup、Pool 等功能的支持。

在基础的 sync 包的基础上，官方还提供了一个高效的扩展包 golang.org/x/sync，包括了 errgroup、semaphore、singleflight、syncmap 等工具。

这里简单介绍其使用方法，以及部分实现原理。

Shell的变量替换
https://www.cnblogs.com/fhefh/archive/2011/04/22/2024750.html

这里使用的是 Go 1.13 版本。

假设将官方的库安装到 `/opt/golang` 目录下，常用的三方库保存在 `/opt/golang/vendor` 目录下，在 `/etc/profile` 文件中添加如下内容。

export GOPATH=/opt/golang/vendor
export GOROOT=/opt/golang
pathmunge "${GOROOT}/bin"
pathmunge "${GOPATH}/bin"

这样，可以确保所有的 Go 版本保存在 `$GOROOT` 中，通用三方包保存在 `$GOPATH/src` 目录下。

go install github.com/jstemmer/gotags
https://github.com/jstemmer/gotags/releases

#!/bin/bash

#REPO_PATH="foobar.com/foobar"
REPO_PATH="foobar"

project_build() {
        out="bin"
        go build foobar
}

pathmunge() {
        if [[ -z "${GOPATH}" ]]; then
                GOPATH=$1
                return
        fi

        case ":${GOPATH}:" in
        *:"$1":*)
                ;;
        *)
                if [[ "$2" = "after" ]] ; then
                        GOPATH=${GOPATH}:$1
                else
                        GOPATH=$1:${GOPATH}
                fi
        esac
}

project_setup_gopath() {
        DIR=$(dirname "$0")
        CDIR=$(cd "${DIR}" && pwd)
        cd "${CDIR}"

        PRG_GOPATH="${CDIR}/gopath"
        if [[ -d "${PRG_GOPATH}" ]]; then
                rm -rf "${PRG_GOPATH:?}/"
        fi
        mkdir -p "${PRG_GOPATH}"

        pathmunge "${PRG_GOPATH}"
        echo "Current GOPATH=${GOPATH}"
        ln -s "${CDIR}/vendor" "${PRG_GOPATH}/src"
        if [[ ! -L "${CDIR}/vendor/${REPO_PATH}" ]]; then
                ln -s "${CDIR}" "${CDIR}/vendor/${REPO_PATH}"
        fi
}

ETCD_SETUP_GOPATH=1

if [[ "${ETCD_SETUP_GOPATH}" == "1" ]]; then
        project_setup_gopath
fi

# only build when called directly, not sourced
if echo "$0" | grep "build$" >/dev/null; then
        project_build
fi
https://n3xtchen.github.io/n3xtchen/go/2018/10/30/go-mod-local-pacakge
http://www.r9it.com/20190611/go-mod-use-dev-package.html
https://www.cnblogs.com/apocelipes/p/10295096.html
https://allenwind.github.io/2017/09/16/Golang%E5%AE%9E%E7%8E%B0%E4%BF%A1%E5%8F%B7%E9%87%8F/
https://yangxikun.com/golang/2017/03/07/golang-singleflight.html
https://segmentfault.com/a/1190000018464029
https://zhuanlan.zhihu.com/p/44585993
https://studygolang.com/articles/22525
https://github.com/golang/sync/tree/master/syncmap
https://blog.csdn.net/mrbuffoon/article/details/85263480
https://gocn.vip/question/161
https://zhuanlan.zhihu.com/p/64983626
https://blog.csdn.net/jiankunking/article/details/78818953
https://medium.com/@deckarep/gos-extended-concurrency-semaphores-part-1-5eeabfa351ce
-->


{% highlight text %}
{% endhighlight %}
