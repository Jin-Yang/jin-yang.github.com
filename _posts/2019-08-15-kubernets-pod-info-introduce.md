---
title: K8S Pod 简介
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:
---

K8S 中有各种各样的组件，对于容器来说 K8S 的最小单元由 Pod 进行组成，它封装了一个或多个应用程序的容器、存储资源、网络 IP 以及管理容器的选项。

<!-- more -->

## 简介

Pod 标示的是一个部署单元，可以理解为 K8S 中应用程序的单个实例，它可能由单个容器组成，也可能由少量紧密耦合并共享资源的容器组成。

### 创建 Pod

这里通过创建一个简单的 BusyBox 服务，可以提前通过 `docker pull busybox` 下载镜像。

创建 `pod.yaml` 文件。

{% highlight text %}
apiVersion: v1
kind: Pod                   # 类别信息
metadata:                   # 基本信息
  name: foobar
  labels:
    os: busybox
spec:                       # 容器信息，可以包含多个容器
  containers:
  - name: hello
    image: busybox
    env:
    - name: Test
      value: "123456"
    command: ["sh","-c","while true;do date;sleep 1;done"]
{% endhighlight %}

如下是基本的操作。

{% highlight text %}
----- 创建、删除Pod
# kubectl create -f pod.yaml
# kubectl delete -f pod.yaml

----- 当前状态信息
# kubectl get pods foobar

----- 描述信息
# kubectl describe pod foobar
{% endhighlight %}

<!--
# 替换资源
kubectl replace -f pod.yaml -force
创建pod请求的响应流程
http://f.dataguru.cn/thread-685212-1-1.html
-->

### Deployment

在早期版本使用 Replication Controller 对 Pod 副本数量进行管理，在新的版本中官方推荐使用 Deployment 来代替 RC 。

相比来说，Deployment 拥有更加灵活强大的升级、回滚功能，并且支持滚动更新；在升级 Pod 时，只需要定义 Pod 的最终状态，K8S 会执行必要的操作，而 RC 要自己定义如何操作。

定义 `deploy.yaml` 配置文件。

{% highlight text %}
apiVersion: apps/v1
kind: Deployment
metadata:
  name: foobar
  labels:
    app: busybox
spec:
  replicas: 1                 # 副本数量
  selector:
    matchLabels:
      app: busybox
  template:
    metadata:
      labels:
        app: busybox
    spec:
      containers:
      - name: hello
        image: busybox        # 也可以通过URL/NAME:Tag方式指定
        ports:                # 暴露容器的80端口
        - containerPort: 2000
        command: ["nc", "-lk", "-p", "2000"]
{% endhighlight %}

K8S 里的 Service 是用来访问 Pod 的，由于 Pod 可能被重启，重启之后 IP 就变了，而 Service 具有名字，可以直接通过名字来访问 Service 所代表的 Pod 。

同样，创建如下的 `service.yaml` 文件。

{% highlight text %}
apiVersion: v1
kind: Service
metadata:
  name: foobar
spec:
  selector:
    app: busybox
  type: NodePort
  ports:
  - protocol: TCP
    port: 2000
    nodePort: 30000
{% endhighlight %}


{% highlight text %}
----- 创建Deployment
# kubectl apply -f deploy.yaml --record

----- 查看当前的状态
# kubectl get deployments
{% endhighlight %}

### 常用命令

{% highlight text %}
----- 查看当前所有POD状态信息
# kubectl get pods

----- 确认某个Pod的具体失败原因
# kubectl describe pod busybox-96cdb678c-cm4l7

----- 查看、删除已经部署的容器
# kubectl get deployments
# kubectl delete deployment busybox-service
{% endhighlight %}

<!--
https://www.cnblogs.com/saneri/p/9128980.html
-->

## 部署服务

如果从头开始，可以通过 Dockerfile 创建一个镜像，然后上传到私有镜像仓库中，不过这里直接使用标准的 BusyBox 镜像。


## 亲和性



{% highlight text %}
{% endhighlight %}
