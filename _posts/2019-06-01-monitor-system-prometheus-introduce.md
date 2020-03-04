---
title: Prometheus 监控系统
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:
---


<!-- more -->




## Prometheus

Prometheus 是由 SoundCloud 开发的一套监控系统，包括了基础的监控框架，报警系统以及时序列数据库(TSDB)。

* 支持多维度数据模型；
* 灵活的查询语言，类 SQL 语言；

不依赖分布式存储，单个服务器节点是自主的。
通过基于HTTP的pull方式采集时序数据。
可以通过中间网关进行时序列数据推送。
通过服务发现或者静态配置来发现目标服务对象。
支持多种多样的图表和界面展示，比如Grafana等。

> Heapster 已经被废弃，其中 A) CPU、内存、HPA 指标改为使用 Metircs-Server 进行采集；B) 基础监控使用 Prometheus ，包括 Kubelet 会将部分指标暴露；C) 事件监控集成到 Eventrouter 。

数据的采集是通过 HTTP 协议周期性的拉取数据，所以只要暴露标准的 HTTP 接口即可 (被称为 exporter)，集成的成本很低，而且有很多开源的 exporter 直接使用。

## 安装

假设使用的安装目录为 `/opt/promethues` ，在该目录下安装 `Prometheus`、`AlertManager` 等工具。

直接从 [Prometheus Download](https://prometheus.io/download/) 上下载二进制包，然后复制到安装目录即可。

# prometheus --version
# prometheus --config.file=prometheus.yml

此时会启动一个默认的界面，可以直接访问安装机器的 9090 端口，如下是一些常见的 API 接口：

* `/metrics` 显示自身运行状态，默认会自动采集；
* `/targets` 采集的目标URL，也就是exporter的地址；

### 配置文件

也就是启动的时候通过 `--config.file` 参数指定的文件。

global:
  scrape_interval:     15s # 默认的采集间隔
  external_labels:
    monitor: 'codelab-monitor'
scrape_configs:            # 抓取对象的配置
  - job_name: 'prometheus' # 会在如下配置的每条记录添加{'job_name':'prometheus'}标签
    scrape_interval: 5s    # 覆盖默认的采集间隔
	static_configs:        # 也就是需要抓取的路径
    - targets: ['localhost:9090']
	  labels:
	    group: 'self'

详细配置可以参考 [Prometheus Configuration](https://prometheus.io/docs/prometheus/latest/configuration/configuration/) 。

### 系统配置

运行的时候会尽量是以非 root 用户运行。

# groupadd -g 3434 -o -r prometheus
# useradd -u 3434 -o -r -g prometheus -m -d /opt/prometheus/prometheus -s /sbin/nologin prometheus

新建 systemd 的相关配置，这样就不需要在管理日志等。

# cat /etc/systemd/system/prometheus.service
[Unit]
Description=Prometheus Monitor System
After=network.target
[Service]
Type=simple
User=prometheus
ExecStart=/opt/prometheus/prometheus/prometheus --config.file=/opt/prometheus/prometheus/prometheus.yml --storage.tsdb.path=/opt/prometheus/prometheus/data
Restart=on-failure
[Install]
WantedBy=multi-user.target
# systemctl status prometheus
# systemctl start prometheus



### 系统指标

主要是通过 `node_exporter` 实现，同样可以新建一个 systemd 的配置文件。

# cat /etc/systemd/system/node_exporter.service
[Unit]
Description=Prometheus Node Exporter
After=network.target
[Service]
Type=simple
User=prometheus
ExecStart=/opt/prometheus/node_exporter/node_exporter
Restart=on-failure
[Install]
WantedBy=multi-user.target

systemctl restart node_exporter

然后，在上述的配置文件中添加如下的内容。

  - job_name: 'linux'
    scrape_interval: 5s
    static_configs:
    - targets: ['localhost:9100']
	  labels:
	    group: 'host'

## PushGateway

## Grafana

### 安装

直接从 [Grafana Download](https://grafana.com/grafana/download) 可以查看下载路径以及安装方式。

# systemctl start grafana-server.service

然后通过 `3000` 端口访问即可，该端口通过 `/etc/grafana/grafana.ini` 中的 `http_port` 配置。

1. 通过 admin/admin 登录，此时需要修改为更安全的密码；
2. 配置数据源，选择 Prometheus 并配置访问地址；
3. 可以从 Prometheus 中直接导入默认的模板；

## PromQL

Prometheus Query Language, PromQL

查询结果主要有 3 种类型：

* 瞬时数据 (Instant Vector)，只包含一组时序，每个时序只有一个点，例如 `http_requests_total` ；
* 区间数据 (Range Vector)，包含一组时序，每个时序有多个点，例如 `http_requests_total[5m]` ；
* 纯量数据 (Scalar)，纯量只有一个数字，没有时序，例如 `count(http_requests_total)`。

其查询的条件也很简单，因为时序数据通过名字和一组标签构成，在过滤的时候也是通过标签进行过滤。

http_requests_total{code="200"}
http_requests_total{code!="200"}
http_requests_total{code=~"2.."}
http_requests_total{code!~"2.."}

https://prometheus.io/docs/prometheus/latest/querying/basics/
https://prometheus.io/docs/prometheus/latest/querying/examples/

## 参考

https://grafana.com/

https://github.com/prometheus/prometheus
https://prometheus.io/



{% highlight text %}
{% endhighlight %}
