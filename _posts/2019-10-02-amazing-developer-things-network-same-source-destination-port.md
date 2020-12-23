---
title: 【拍案惊奇】之网络端口
layout: post
comments: true
language: chinese
category: [misc]
keywords: tcp,端口相同
description: 如果在本地访问时，使用了一个源和目的端口都相同的 TCP 链接，会出现什么情况？
---

如果在本地访问时，使用了一个源和目的端口都相同的 TCP 链接，会出现什么情况？

<!-- more -->

## 简介

在同一台机器上部署的程序，通常会通过 Socket 进行通讯，如果监听的端口刚好在随机分配的端口范围内，那么就有可能出现源和目的端口相同的情况。

这个问题出现的概率很低，但是一旦出现相同的通讯就会发生异常。

### 原理

对于客户端，如果没有调用 `bind()` 设置端口，那么当调用 `connect()` 函数时会为套接字选择一个临时接口，那么，当服务端监听的端口也在这一范围内时，那么就有可能出现客户端和服务端端口相同的情况。

一般只会发生在本地，如果是远程，那么至少其 IP 地址是不同的。

也就是在当前机器上绑定了客户端和服务端都是相同的端口，假设在内核中创建了一个套接字 `sk` ，对应的上述的 IP:Port ，那么，基本处理流程如下：

1. 客户端发送 SYN 请求，服务端接收到 SYN 之后，通过 `__inet_lookup()` 查找内核中的套接字，也就是上述的 `sk` ，状态迁移到 `SYN_RCVD` ；
2. 然后发送 `SYN+ACK` 响应，套接字迁移到 `ESTABLISED` 状态。

注意，此时收到的序号要比 `SYN+ACK` 段的序号大 1 ，相当于收到了重复的段，会再发一个 `D-ACK` 标识收到了重复段，但是不影响状态。

## 测试代码

竟然仍可以发送接收数据。

{% highlight c %}
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(void)
{
        char buff[1024] = "Hello World!";
        int rc, sockfd, i;
        struct sockaddr_in addr;


        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
                fprintf(stderr, "create socket failed, %d:%s.\n", errno, strerror(errno));
                return -1;
        }

        bzero(&addr, sizeof(addr));
        addr.sin_family      = AF_INET;
        addr.sin_port        = htons(9090);
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        rc = bind(sockfd, (const struct sockaddr *)&addr, sizeof(struct sockaddr));
        if (rc < 0) {
                fprintf(stderr, "bind socket failed, %d:%s.\n", errno, strerror(errno));
                close(sockfd);
                return -1;
        }


        rc = connect(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr));
        if (rc < 0) {
                fprintf(stderr, "connect failed, %d:%s.\n", errno, strerror(errno));
                close(sockfd);
                return -1;
        }

#if 0
        for (i = 0; i < 3; i++) {
                rc  = send(sockfd, buff, strlen(buff), 0);

                rc = recv(sockfd, buff, sizeof(buff) - 1, 0);
                buff[rc] = 0;

                printf("received:%s\n", buff);

                //rc = scanf("%3s", sizeof(buff) - 1, buff);
                //rc = scanf("%3s", buff);

                //printf("%d %s\n", rc, buff);
        }
#else
        rc = recv(sockfd, buff, sizeof(buff) - 1, 0);
        if (rc < 0) {
                fprintf(stderr, "connect failed, %d:%s.\n", errno, strerror(errno));
                close(sockfd);
                return -1;
        }
        buff[rc] = 0;
#endif

        close(sockfd);

        return 0;
}
{% endhighlight %}

<!--
https://blog.csdn.net/wojiuguowei/article/details/99442952
-->


{% highlight text %}
{% endhighlight %}
