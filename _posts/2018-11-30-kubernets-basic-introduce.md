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

## 简介

Kubernetes 的整体架构如下。

![k8s arch]({{ site.url }}/images/docker/kubernetes-high-level-component-archtecture.jpg "k8s arch"){: .pull-center width="80%" }

K8S 包含了两类节点，Master 和 Nodes ，前者用于管理协调集群，包含了 `kube-apiserver` `kube-controller-manager` `kube-scheduler` 三个进程；后者则是真正工作的机器，包含了 `kubelet` `kube-proxy` 两个进程。

## API-Server

{% highlight text %}
--admission-control 权限控制
{% endhighlight %}

## Kubectl

kubectl 是 Kubernetes API 的客户端，可以用来执行所有可能的 Kubernetes 操作。

### 自动补全

一般的 Bash 或者 Zsh 都提供了自动补全的方式，不过需要安装单独的依赖包，默认是安装的，例如 `bash-completion` ，可以通过 `type _init_completion` 命令检查，详细参考 [自动补全]({{ site.production_url }}/post/linux-bash-auto-completion-introduce.html) 。

可以通过 `kubectl completion bash` 生成命令补全内容，参数也可以是 `zsh` ，然后通过如下方式进行配置。

{% highlight text %}
----- 1. 生成配置文件，并添加到~/.bashrc文件中
# kubectl completion bash > /usr/share/bash-completion/bash_completion
# source /usr/share/bash-completion/bash_completion

----- 2. 加载默认配置，添加到默认的加载目录
# kubectl completion bash > /etc/bash_completion.d/kubectl
{% endhighlight %}

在输出的时候，可以通过 `-o custom-columns=XXX` 指定输出内容以及格式。

### kubeconfig

K8S 中通过配置文件保存了集群、用户、命名空间、鉴权等与集群建立连接相关信息，默认保存在 `~/.kube/config` 文件中，也可以通过 `KUBECONFIG` 环境变量，或者 `--kubeconfig` 参数指定。

当通过 `kubectl config view` 命令查看时，有可能是几个配置文件的合并，详见 [Organizing Cluster Access Using kubeconfig Files](https://kubernetes.io/docs/concepts/configuration/organize-cluster-access-kubeconfig/) 中的介绍。

通常一个集群对应了一个上下文，其上下文包括了：

* 集群 (Cluster)，集群API服务器的URL地址；
* 用户 (User)，集群的特定用户的身份验证凭据；
* 命名空间 (Namespace)，连接到集群时使用的命名空间。

也就是说，在切换集群的时候，实际上就是切换上下文，当然，也可以通过 `--cluster` `--user` `--namespace` 指定。

如下是常用的命令。

{% highlight text %}
----- 查看配置，也就是~/.kube/config
# client/bin/kubectl config view

----- 列出所有上下文信息以及当前上下文信息
# client/bin/kubectl config get-contexts
# client/bin/kubectl config current-contexts
{% endhighlight %}

<!--
kubectl config use-context：更改当前上下文
kubectl config set-context：更改上下文的元素
https://learnk8s.io/blog/kubectl-productivity/
-->


## Kube-Proxy

在 K8S 中，Service 是一组 pod 服务的抽象，会提供统一的 IP (Cluster IP)，类似于一组 pod 的 LB，负责将请求分发给对应的 pod 。



而 kube-proxy 就是用来实现 Service ，具体来说，就是实现了内部从 pod 到service和外部的从node port向service的访问。

举个例子，现在有podA，podB，podC和serviceAB。serviceAB是podA，podB的服务抽象(service)。
那么kube-proxy的作用就是可以将pod(不管是podA，podB或者podC)向serviceAB的请求，进行转发到service所代表的一个具体pod(podA或者podB)上。
请求的分配方法一般分配是采用轮询方法进行分配。

另外，kubernetes还提供了一种在node节点上暴露一个端口，从而提供从外部访问service的方式。


假设有如下的配置。

{% highlight text %}
apiVersion: v1
kind: Service
metadata:
  labels:
    name: mysql
    role: service
  name: mysql-service
spec:
  ports:
    - port: 3306
      targetPort: 3306
      nodePort: 30964
  type: NodePort
  selector:
    mysql-service: "true"
{% endhighlight %}

https://www.cnblogs.com/xuxinkun/p/5799986.html

## Label

Label 是 K8S 中一个核心的概念，通过 Key-Value 方式组织，可以附加到任何对象上，例如 Pod、Service、Node 等。

<!--
release: stable
release: canary
env: dev
env: product
tier: frontend
tier: backend
tier: middleware
-->

## 部署

在部署服务时，通过 YMAL 文件进行定义，主要包括了两个配置文件：A) 服务文件，定义 POD 逻辑组及其策略；B) 部署文件，定义应用程序的运行状态，比如什么时间应该运行多少副本。




<!--
## MiniKube

需要安装 [kubectl](https://github.com/kubernetes/kubernetes/blob/master/CHANGELOG.md) ，选择对应的版本。

https://storage.googleapis.com/kubernetes-release/release/v1.15.4/bin/linux/amd64/kubectl

https://docker.mirrors.ustc.edu.cn/

https://github.com/AliyunContainerService/minikube/releases
http://kubernetes.oss-cn-hangzhou.aliyuncs.com/minikube/releases/v1.3.1/minikube-linux-amd64



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

-->

<!-- kubeadm config images pull http://www.ruanyifeng.com/blog/2018/02/docker-tutorial.html https://www.mdslq.cn/archives/5e6f338.html cubectl https://github.com/kubernetes/kubernetes/blob/master/CHANGELOG.md https://yq.aliyun.com/articles/221687 https://www.jianshu.com/p/18441c7434a6 https://ehlxr.me/2018/01/12/kubernetes-minikube-installation/ https://blog.csdn.net/qq_26188449/article/details/77543093
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




<!--
1. Cluster

cluster是计算、存储和网络资源的集合，kubernetes利用这些资源运行各种基于容器的应用。
2. Master

Master 是Cluster的大脑，它的主要职能就是负责调度，决定应用放在哪里运行。master运行linux操作系统，可以是物理机或者虚拟机。为了实现高可用，可以运行多个Master。
3. Node

Node 的职责是运行容器应用。Node由Master管理，Node负责监控并汇报容器的状态，同时根据Master的要求管理容器的生命周期。Node运行在linux系统上，可以是物理机或者虚拟机。
4. Pod

Pod是kubernetes的最小工作单元。每个pod可以包含一个或者多个容器。Pod中的容器会作为一个整体被Master调度到一个Node上运行。kubernetes 以Pod为最小单位进行调度、扩展、共享资源、管理生命周期；pod中的所有容器都共享一个网络namespace，所有的容器可以共享存储。

    pod有两种使用方式：

    运行单一容器：

    one-container-per-Pod 是kubernetes最常见的模型，这种情况下，只是将单个容器简单封装成pod。即使只有一个容器，kubernetes管理的也是pod而不是直接管理容器。
    运行多个容器:

    运行在同一个pod的的多个容器必须联系紧密，而且直接共享资源。

5. Controller

kubernetes通常不会直接去创建pod，而是通过Controller去管理pod的，Controller中定义了Pod的部署特性，比如有几个副本、什么样的Node上运行等。为了满足不同的业务场景，kubernetes提供了多种Controller，包括Deployment、ReplicaSet、DeamonSet、StatefuleSet、Job等，我们逐一讨论。

    Deployment :是最常用的的Controller，deployment可以管理pod的多个副本，并确保pod按照预期的状态来运行。
    ReplicaSet : 实现了pod的多副本管理。使用Deployment时会自动创建Replicaset。
    DeamonSet: 用于每个Node最多只能运行一个Pod副本的场景。
    StatefuleSet:能够保证Pod的每个副本在整个生命周期中名称是不变的，而其它Controller是提供这个功能。当某个Pod发生故障需要删除并且重新启动时，Pod的名称会发生变化，同时StatefuleSet会保证副本按照固定的顺序启动、更新或者删除。
    Job 用于运行就删除的应用，而其他Controller　中的ｐｏｄ通常是持续运行的。

6. Service

Deployment 可以部署多个副本，每个Pod都有自己的Ip，那么外界如何访问这些副本呢？Kubernetes Service 定义了外界访问一组特定Pod的方式。Service 有自己的IP和端口，Service为pod提供了负载均衡。K8s运行容器Pod与访问容器Pod这两项任务分别由Controller和Service执行。
7. Namespace

如果有多个用户或者项目组共同使用k8s 集群，如果将他们创建的Pod等资源分开呢，就是通过Namespace进行隔离。


















Master 用来管理整个集群；
Node 可以是物理机或者虚拟机，实际运行的机器；
Pod 与业务逻辑相关的逻辑概念，可以包含多个容器、磁盘、网络空间等。

另外，还可以将 Service 作为一组 POD 的抽象。

Master 用来管理所有的 Nodes，会自动调度每个 Node 上的可用资源；每个 Node 可以运行多个 Pods 。

这就意味着，Node 上至少需要运行：A) 负责和 Master 进行通讯；B) 管理 Nodes 上的容器 。上述的两个功能通过 kubelet 实现，另外还有 kube-proxy 反应 K8S 的网络服务的网络代理。

而 Master 节点包含了：A) API Server 对外提供接口 kube-apiserver；B) Scheduler 调度器 kube-scheduler；C) 集群管理 kube-controller-manager 。

10250 API 接口，可以访问获取 Node 资源及其状态；
4194 cAdvisor 获取 Node 节点的环境信息以及容器的运行状态；
10255 只读接口，无需鉴权；
10248 健康检查接口。

kubectl config view

API访问获取Token
kubectl get secrets
kubectl describe secret NAME

kubectl get - 列出所有的资源
kubectl describe - 列出某个资源的详细信息
kubectl logs - 输出pod中容器的日志
kubectl exec - 在pod中的某个容器里面执行命令

## 源码解析

### kubelet

该组件运行在 Node 节点上，维持运行中的 Pods 以及提供 Kuberntes 运行时环境，主要完成以下使命：

１．监视分配给该Node节点的pods
２．挂载pod所需要的volumes
３．下载pod的secret
４．通过容器管理工具 (例如 docker、rkt) 来运行 Pod 中的容器；

５．周期的执行pod中为容器定义的liveness探针
６．上报pod的状态给系统的其他组件
７．上报Node的状态

在 K8S 中通过协程调度的时候，经常会通过 `wait.Until()` 封装启动。

cmd/kubelet/app/server.go 真正的入口函数是Run()

run()
 |-buildKubeletClientConfig()
 |-RunKubelet() 真正运行
 | |-startKubelet()
 |   |-Run() 通过wait.Until()进入主循环 pkg/kubelet/kubelet.go
 |     |-syncNodeStatus() 周期性的同步数据，以供调度使用
 |     |-syncLoop() 处理Pod的增删改查
 |       |-syncLoopIteration() 会对接收到的消息进行处理转发
 |         |-HandlePodAdditions() 创建POD
 |   |-ListenAndServe() 提供简单的API接口
 |   |-ListenAndServeReadOnly() 只读的接口
 |-InstallHandler() 如果需要则安装healthz服务

在调用 `RunKubelet()` 函数之前，大部分的都是在创建和初始化 `kubeDeps` 这个对象，在该对象中保存了 kubelet 的核心配置以及组件，它最终会传递到 `RunKubelet()` 函数中。

其实际是为了实现 Dependency Injection 机制，可以通过修改进行一些 mock 测试，其中 `kubeDeps` 包含的组件很多，下面简单列举一些：





CAdvisorInterface：提供 cAdvisor 接口功能的组件，用来获取监控信息
DockerClient：docker 客户端，用来和 docker 交互
KubeClient：apiserver 客户端，用来和 api server 通信
Mounter：执行 mount 相关操作
NetworkPlugins：网络插件，执行网络设置工作
VolumePlugins：volume 插件，执行 volume 设置工作

在上述的代码中，可以看到，它会从以下管道中获取相关的消息：

* configCh 读取配置事件的管道，包括了文件、URL、APIServer 汇聚后的事件；
* syncCh 定时器管道，每次隔一段时间去同步最新保存的 POD 状态；
* houseKeepingCh 一些维护相关的工作，例如 POD 清理；
* plegCh 当 POD 状态发生变化时，进行相关的处理；

这里简单分析，从 APIServer 添加 POD 的场景，也就是 `handler.HandlePodAdditions(u.Pods)` 的处理。

## kubectl
## Controller Manager

集群内部的管理控制中心，负责集群内的 Node、Pod、Endpoint、Namespace、ServiceAccount、ResourceQuota 的管理，当某个 Node 意外宕机时，Controller Manager 会及时发现并执行自动化修复流程，确保集群始终处于预期的工作状态。

通过 kube-controller-manager 和 cloud-controller-manager 两个进程组成，最终通过 APIServer 监控整个集群的状态，并确保集群处于预期的工作状态。

丢包总结
https://cizixs.com/2018/01/13/linux-udp-packet-drop-debug/

## kube-scheduler
http://tang.love/2018/07/24/learning-kubernetes-source-code/
cmd/kube-scheduler/scheduler.go
进程入口。

main()   scheduler.go
 |-NewSchedulerCommand() server.go
 |-Run()
   |-NewSchedulerConfig()  一般会走到source.Policy分支
   | |-NewConfigFactory()
   | |-CreateFromConfig()
   |   |-CreateFromKeys()
   |-NewFromConfig()

scheduleOne()
 |-Scheduler.config.NextPod() 这里是一个函数指针可以定制，默认是getNextPod()
 | |-podQueue.Pop()
 |-Scheduler.schedule()
   |-Scheduler.config.Algorithm.Schedule() <<<1>>> 默认在core/generic_scheduler.go中
     |-findNodesThatFit() 1.1 开始预选
	 | |-podFitsOnNode() 完成所有配置的策略对Node的检查，最大会有16个并发同时检查
	 | |-extender.Filter() 如果设置了扩展，则进行过滤
	 |-PrioritizeNodes() 开始打分
	 |-selectHost() 如果有多个则选择最优的Node

在 algorithm/scheduler_interface.go 中定义接口，而默认的实现在 core/generic_scheduler.go 中，真正的选择处理逻辑。

默认的 Predicate 和 Priorities 策略保存在 pkg/scheduler/algorithm/{predicates,priorities} 中。


在 pkg/scheduler/algorithmprovider/defaults/defaults.go 中设置默认的规则

registerAlgorithmProvider()
 |-defaultPredicates()
 |-defaultPriorities()


# 调度器

关于调度器的策略可以参考 [Scheduler Algorithm in Kubernetes](https://github.com/kubernetes/community/blob/master/contributors/devel/scheduler_algorithm.md) 。

简单来说，也就是 kube-scheduler 进程，在 k8s 集群上新建 pod 并放到合适的 node 上去，这一过程称为绑定 (bind)。

具体而言，监听 apiserver 的 /api/pod/，当发现集群中有未调度的 pod (PodSpec.NodeName为空) 时，会查询集群各 node 的信息，经过 Predicates(过滤)、Priorities(优选器)，得到最适合该 pod 运行的 node 后，再向 apiserver 发送请求，将该容器绑定到选中的 node 上。

https://juejin.im/entry/5b70dc46e51d45666f798909
https://yeasy.gitbooks.io/docker_practice/repository/registry.html




https://arkingc.github.io/2017/07/03/2017-07-03-docker-filesystem-devicemapper/
https://www.jianshu.com/p/6ec8c9ecc5c2
https://cloud.tencent.com/developer/article/1097449
https://cizixs.com/2017/03/30/kubernetes-introduction-service-and-kube-proxy/
https://xuxinkun.github.io/2016/07/18/flannel-docker/
https://blog.csdn.net/u010305706/article/details/52208462
https://jimmysong.io/kubernetes-handbook/concepts/flannel.html

http://blog.xbblfz.site/2018/08/24/kubectl%E6%BA%90%E7%A0%81%E5%88%86%E6%9E%90/
https://blog.csdn.net/qq_32261399/article/details/82019707
https://imroc.io/posts/kubernetes/kubernetes-source-code-reading-notes-kube-apiserver-code-main-line/

https://cizixs.com/2017/03/27/kubernetes-introduction-controller-manager/






kubelet
socat
conntrack-tools
libnetfilter_cthelper
libnetfilter_cttimeout
libnetfilter_queue
kubernetes-cni
-->




{% highlight text %}
{% endhighlight %}
