---
layout: default
title: Projects
---
<style type="text/css"><!-- p {text-indent: 2em;} li > p {text-indent: 0em;} .comment { font-size: 0.8em; font-style:italic; } --></style>

# 项目导航

---------------------------

## DevOps

![logo]({{ site.url }}/images/devops/devops-process-logo.png "logo"){: .pull-center width="70%" }

简单来说，DevOps 是 Development 和 Operations 的组合词，是一种重视软件开发人员 (Dev) 和IT运维技术人员 (Ops) 之间沟通合作的文化或惯例。透过自动化"软件交付"和"架构变更"的流程，来使得构建、测试、发布软件能够更加地快捷、频繁和可靠。

DevOps 的三大原则：

* 基础设施即代码 (Infrastructure as Code)<br>
将重复的事情使用自动化脚本或软件来实现，例如 Docker(容器化)、Jenkins(持续集成)、Puppet(基础架构构建)等。

* 持续交付 (Continuous Delivery)<br>
在生产环境发布可靠的软件并交付给用户使用，而持续部署则不一定交付给用户使用。其中涉及到两个时间，需要尽可能减小这两个时间：Time to Repair, TTR 修复时间，Time To Marketing, TTM 产品上线时间。

* 协同工作 (Culture of Collaboration)<br>
开发者和运维人员必须定期进行密切的合作，通常是通过几种建议方式：A) 自动化，以减少不必要的协作；B) 小范围，每次修改的内容不宜过多，减少发布的风险；C) 统一信息集散地，使用Wiki可以共享信息。

<!-- Mobius -->

包含的内容有：

* [基础平台简介](/projects/devops/introduce.html)，对整体基础平台的介绍。

### 实现

这里介绍整体的规范、通用服务的搭建。

* [通用规范](/projects/devops/platform-common.html) 设置一些通用的规范，包括错误码、命名规则等。
* [基础服务](/projects/devops/platform-common-service.html) 一些通用服务的搭建，例如 FTP、HTTP、DB 等。

#### Agents

部署在主机侧，使用 C 语言实现，按照功能划分为如下几类，基本、监控、日志、安全、网络等，详细见如下的介绍，其中所有的 Agent 统一由 BootAgent 进行管理。

如下是实现的细节。

* [BootAgent](/projects/devops/platform-agent-bootagent.html) 装机时安装，负责安装、升级、监控、重启其它 Agent 。
* [BasicAgent](/projects/devops/platform-agent-basicagent.html) 基本命令执行(同步、异步、查询)、安装部署、服务健康检查等。
* [MonitorAgent](/projects/devops/platform-agent-monitoragent.html) 采集监控数据，包括系统类、中间组件、业务指标等。
* [SecureAgent](/projects/devops/platform-agent-secureagent.html) 安全相关，例如入侵检测。

##### 其它

上述 Agent 的实现有很多公共库支撑，如下是其简单的介绍。

* [Agent 通用库](/projects/devops/platform-agent-common-library.html) 记录一些通用库的实现、API 接口，以及一个示例 MiniAgent。

<!--
* LogAgent 按照固定格式采集日志数据
-->

#### Servers

根据不同的功能进行划分。

* [Cargo](/projects/devops/platform-server-cargo.html) 基础的通用服务，包括了单点登陆系统、用户信息、Portal等。
* [Aspire](/projects/devops/platform-server-aspire.html) BootAgent 的服务端，用于 Agent 的安装部署、升级、状态统计等。

### FAQ






<!--
基于libev的websocket库
https://github.com/zhaojh329/libuwsc
ELF加密
https://github.com/droberson/ELFcrypt
加密协议
https://github.com/WickrInc/wickr-crypto-c
-->

