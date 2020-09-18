---
Date: October 19, 2013
title: Nginx 负载均衡
layout: post
comments: true
language: chinese
category: [linux, network]
---

Nginx 是一个高性能的 HTTP 服务器和反向代理服务器，在此我们将其作为一个负载均衡服务器。

<!-- more -->

# 简介

Nginx 支持的三种负载均衡策略

轮询：将请求依次轮询发给每个服务器。

最少链接：将请求发送给持有最少活动链接的服务器。

ip哈希：通过哈希函数决定请求发送给哪个服务器。

权重：服务器的权重越高，处理请求的概率越大。


轮询负载均衡

在nginx.conf配置文件中添加如下配置，此配置有三台服务器提供支付服务。
复制代码

http {
        upstream CashServers {
                    server CashServers1.com;
                            server CashServers2.com;
                                    server CashServers3.com;
                                        }

                                            server {
                                                        listen 80;

                                                                location / {
                                                                                proxy_pass http://CashServers;
                                                                                        }
                                                                                            }
}

复制代码



 需要注意以下几点
 1.缺省配置就是轮询策略;
 2.nginx负载均衡支持http和https协议，只需要修改 proxy_pass后协议即可;
 3.nginx支持FastCGI, uwsgi, SCGI,memcached的负载均衡,只需将 proxy_pass改为fastcgi_pass, uwsgi_pass, scgi_pass,memcached_pass即可。
 4.此策略适合服务器配置相当，无状态且短平快的服务使用。
 四、最少链接负载均衡

 复制代码

 http {
         upstream CashServers {
                   least_conn;
                           server CashServers1.com;
                                   server CashServers2.com;
                                           server CashServers3.com;
                                               }

                                                   server {
                                                               listen 80;

                                                                       location / {
                                                                                       proxy_pass http://CashServers;
                                                                                               }
                                                                                                   }
 }

 复制代码



  需要注意以下几点
  1.最少链接负载均衡通过least_conn指令定义;
  2.此负载均衡策略适合请求处理时间长短不一造成服务器过载的情况;
  五、ip哈希负载均衡

  复制代码

  http {
          upstream CashServers {
                    ip_hash;
                            server CashServers1.com;
                                    server CashServers2.com;
                                            server CashServers3.com;
                                                }

                                                    server {
                                                                listen 80;

                                                                        location / {
                                                                                        proxy_pass http://CashServers;
                                                                                                }
                                                                                                    }
  }

  复制代码



   需要注意以下几点
   1.ip哈希负载均衡使用ip_hash指令定义;
   2.nginx使用请求客户端的ip地址进行哈希计算，确保使用同一个服务器响应请求;
   3.此策略适合有状态服务，比如session;
   六、权重负载均衡

   复制代码

   http {
           upstream CashServers {
                       server CashServers1.com weight=3;
                               server CashServers2.com weight=2;
                                       server CashServers3.com weight=1;
                                           }

                                               server {
                                                           listen 80;
                                                                   location / {
                                                                                   proxy_pass http://CashServers;
                                                                                           }
                                                                                               }
   }

   复制代码



    需要注意以下几点

    1. 权重负载均衡需要使用weight指令定义;

    2. 权重越高分配到需要处理的请求越多;

    3.此策略可以与最少链接负载和ip哈希策略结合使用;

    4.此策略比较适合服务器的硬件配置差别比较大的情况;


    七、健康检测

    nginx内置了针对服务器的健康检测机制，如果特定服务器请求失败，则nginx即可进行标记待下次就不会请求分配给它。max_fails定义失败指定次数后进行标记服务器不可用。
# 参考


http://freeloda.blog.51cto.com/2033581/1288553

{% highlight text %}
{% endhighlight %}
