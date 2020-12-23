---
title: Linux Socket 端口使用
layout: post
comments: true
language: chinese
category: [linux,program,misc]
keywords: linux,socket,port,range
description: Linux 使用的 TCP/IP 协议栈会使用四元组，在客户端与服务端建立链接的时候一般无需指定端口，可以直接使用系统默认指定的端口。 当然，也可以指定固定的端口，不过此时可能会导致端口冲突。
---

Linux 使用的 TCP/IP 协议栈会使用四元组，在客户端与服务端建立链接的时候一般无需指定端口，可以直接使用系统默认指定的端口。

当然，也可以指定固定的端口，不过此时可能会导致端口冲突。

<!-- more -->

## 示例

如下是通过 nc 进行的简单测试。

{% highlight text %}
----- 建立一个8080的监听端口
$ nc -k -l 127.0.0.1 8080
----- 客户端直接连接到服务端
$ nc 127.0.0.1 8080
----- 查看客户端使用的端口
# netstat -atunp | grep 8080
tcp        0      0 127.0.0.1:8080          0.0.0.0:*               LISTEN      28436/nc
tcp        0      0 127.0.0.1:8080          127.0.0.1:55596         ESTABLISHED 28436/nc
tcp        0      0 127.0.0.1:55596         127.0.0.1:8080          ESTABLISHED 48830/nc
{% endhighlight %}

也即是说，当前客户端使用的是 `55596` 这个端口，如果现在再次使用 `nc -l 127.0.0.1 55596` 就会报错，其内容为 `Ncat: bind to 127.0.0.1:55596: Address already in use. QUITTING.` 。

这也就意味着，只要是一个端口被占用，不管是客户端还是服务端，其它进程是无法再次使用的。

那么可以在连接到服务时指定一个端口，也是通过 `bind()` 进行绑定，当然，前提仍然是改端口没有被使用，可以参考如下的示例。

{% highlight c %}
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

int main(void)
{
        int rc, sock;
        const char *ipaddr = "127.0.0.1";
        struct sockaddr_in svr, cli;

        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
                fprintf(stderr, "Create socket failed, %s.\n", strerror(errno));
                return -1;
        }

        memset(&cli, 0, sizeof(cli));
        cli.sin_family = AF_INET;
        cli.sin_addr.s_addr = htonl(INADDR_ANY);
        cli.sin_port = htons(55596);
        if (bind(sock, (struct sockaddr *)&cli, sizeof(cli)) < 0) {
                fprintf(stderr, "Bind client socket failed, %s.\n", strerror(errno));
                close(sock);
                return -1;
        }

        memset(&svr, 0, sizeof(svr));
        svr.sin_family = AF_INET;
        svr.sin_port = htons(8080);
        rc = inet_pton(AF_INET, ipaddr, &svr.sin_addr);
        if (rc <= 0) {
                if (rc == 0)
                        fprintf(stderr, "Not in presentation format.\n");
                else
                        fprintf(stderr, "Inet_pton() failed, %s.\n", strerror(errno));
                close(sock);
                return -1;
        }

        if(connect(sock, (struct sockaddr *)&svr, sizeof(svr)) < 0) {
                fprintf(stderr, "Connect to server failed, %s.\n", strerror(errno));
                close(sock);
                return -1;
        }

        sleep(10000);
        write(sock, "123\n", 4); /* try best */

        close(sock);

        return 0;
}
{% endhighlight %}

通常来说，用户不会关心客户端的端口是什么，所以默认不会 `bind()` ，而是由内核默认分配。

## 内核配置

实际内核提供了参数用于指定本地客户端分配端口的范围，可以参考如下的设置。

{% highlight text %}
----- 查看当前的配置
$ cat /proc/sys/net/ipv4/ip_local_port_range 
$ sysctl net.ipv4.ip_local_port_range

----- 可以临时修改
$ echo "1024 65535" > /proc/sys/net/ipv4/ip_local_port_range
$ sysctl -w net.ipv4.ip_local_port_range="1024 65535"

----- 修改配置文件中的默认配置
$ cat /etc/sysctl.conf | grep 'ip_local_port_range'
net.ipv4.ip_local_port_range = 32768 59000

----- 重新加载，使配置修改生效
$ sysctl -p /etc/sysctl.conf
{% endhighlight %}

详细可以参考 `man 5 sysctl.conf` 或者 `man sysctl` 中的介绍。

注意，上述的配置时，一般建议使用奇数 + 偶数的配置方式。

{% highlight text %}
{% endhighlight %}
