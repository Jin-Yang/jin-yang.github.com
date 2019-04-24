---
title: HTTP2 协议使用
layout: post
comments: true
language: chinese
category: [misc]
keywords: http2,network,hpack
description: HTTP2 也就是超文本传输协议第 2 版，保持了与 HTTP1 相同的基本语义，例如 方法语义 (GET PUST PUT DELETE 等)、状态码 (200 404 500 等)、URL 等等，相比来说做了如下的优化。链路复用，通过单个 TCP 连接支持多个通道；头部压缩，解析会更快、更小等等。
---

HTTP2 也就是超文本传输协议第 2 版，基于 TCP 连接的一个上层协议，允许在一个 TCP 连接上多路复用，支持优先级、流量控制、服务器推送、首部HPACK压缩、

保持了与 HTTP1 相同的基本语义，例如 方法语义 (GET PUST PUT DELETE 等)、状态码 (200 404 500 等)、URL 等等，相比来说做了如下的优化。

通过单个 TCP 连接支持多个通道；头部压缩，解析会更快、更小等等。

<!-- more -->

![http2]({{ site.url }}/images/network/http2-introduce.png  "http2"){: .pull-center width="60%" }

## 简介

HTTP2 传输的数据是二进制的，相比 HTTP1.X 的纯文本数据，二进制数据的传输体积更小，更易于解析，纯文本帧在解析的时候还要考虑处理空格、大小写、空行和换行等问题，而二进制帧就不存在这个问题。

![http2 frame]({{ site.url }}/images/network/http2-binary-frame.svg "http2 frame"){: .pull-center width="60%" }

在 HTTP2 中，有如下的三个概念，也就是：流 (Stream)、消息 (Message) 和帧 (Frame)。

* Stream 处于一个连接中的虚拟双向二进制数据流，通过一个唯一的整数进行标识，可以包含一个或者多个 Message 。
* Message 一个完整的请求或者响应，比如请求、响应等，由一个或多个 Frame 组成。
* Frame HTTP2 通讯的最小单位，每个帧包含帧首部帧，表示当前帧所属的流，承载着特定类型的数据，如 HTTP 首部、负荷等等。

![http2 conception]({{ site.url }}/images/network/http2-connection-stream-frame.svg "http2 conception"){: .pull-center width="70%" }

每个 HTTP2 连接上传输的帧都关联到一个 "流"，这是一个逻辑上的概念，作为一个独立的双向帧序列，用于在客户端和服务器端之间进行通讯。

可以包含多个并发的流，任何一端都可以交错地插入帧。

![http2 multiplexing]({{ site.url }}/images/network/http2-multiplexing.svg "http2 multiplexing"){: .pull-center width="70%" }

### 优先级

每个资源都获取一个 Stream ID 来标识连接上的资源，有三个参数用于定义资源优先级：

* 父级数据流 (Parent Stream)：这个数据流是一个“依赖”资源或者应该在之后被传递的数据流。有一个所有数据流共享的虚拟root stream 0。
* 权重(Weight)：1到256之间的数字，用于标识在多个数据流共享连接时分配给此数据流的带宽量。带宽是相对于所有其他活动的数据流的权重分配的，而不是绝对值。
* 独占位(Exclusive bit)：一个标志，表示应该在不与任何其他数据流共享带宽的情况下下载。

浏览器不一定同时知道所有资源，因此服务器能够在新请求到达时重新确定请求的优先级也很关键。

## 示例

简单介绍下两个基本概念：

* h2 基于 TLS 构建的 HTTP2，其中 ALPN 标识符为 0x68 0x32 ，也就是 https 。
* h2c 直接在 TCP 之上构建的 HTTP2，也就是 http 。

### 示例

如果不启用 TLS ，默认只支持 HTTP1.1 ，

{% highlight go %}
package main

import (
	"fmt"
	"golang.org/x/net/http2"
	"html"
	"net/http"
)

func main() {
	var server http.Server

	http2.VerboseLogs = true
	server.Addr = ":8990"

	http2.ConfigureServer(&server, &http2.Server{})

	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		fmt.Fprintf(w, "URL: %q\n", html.EscapeString(r.URL.Path))
		ShowRequestInfoHandler(w, r)
	})

	server.ListenAndServe() // 不启用https则默认只支持http1.x
	//server.ListenAndServeTLS("localhost.cert", "localhost.key")
}

func ShowRequestInfoHandler(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "text/plain")

	fmt.Fprintf(w, "Method: %s\n", r.Method)
	fmt.Fprintf(w, "Protocol: %s\n", r.Proto)
	fmt.Fprintf(w, "Host: %s\n", r.Host)
	fmt.Fprintf(w, "RemoteAddr: %s\n", r.RemoteAddr)
	fmt.Fprintf(w, "RequestURI: %q\n", r.RequestURI)
	fmt.Fprintf(w, "URL: %#v\n", r.URL)
	fmt.Fprintf(w, "Body.ContentLength: %d (-1 means unknown)\n", r.ContentLength)
	fmt.Fprintf(w, "Close: %v (relevant for HTTP/1 only)\n", r.Close)
	fmt.Fprintf(w, "TLS: %#v\n", r.TLS)
	fmt.Fprintf(w, "\nHeaders:\n")

	r.Header.Write(w)
}
{% endhighlight %}

{% highlight go %}
package main

import (
	"crypto/tls"
	"fmt"
	"io/ioutil"
	"log"
	"net"
	"net/http"

	"golang.org/x/net/http2"
)

func main() {
	url := "http://localhost:8990/"
	client(url)
}

func client(url string) {
	log.SetFlags(log.Llongfile)
	tr := &http2.Transport{ // 服务端退化成了 http1.x
		AllowHTTP: true, // 充许非加密的链接
		//TLSClientConfig: &tls.Config{InsecureSkipVerify: true},
		DialTLS: func(netw, addr string, cfg *tls.Config) (net.Conn, error) {
			return net.Dial(netw, addr)
		},
	}

	httpClient := http.Client{Transport: tr}

	resp, err := httpClient.Get(url)
	if err != nil {
		log.Fatal(err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		fmt.Println("resp StatusCode:", resp.StatusCode)
		return
	}

	body, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		log.Fatal(err)
	}

	fmt.Println("resp.Body:\n", string(body))
}
{% endhighlight %}

通过 `tcpdump -X -nnn -i lo tcp port 8990 and host 127.0.0.1` 可以发现，服务器向客户端发了一个 http1.1 的包，并且服务端还关闭了连接。

此时服务端需要考虑使用更低一层的 http2 库实现，也就是使用 ServCon 直接替换掉 `net/http` 中的 serv 函数，例如。

{% highlight go %}
package main

import (
	"fmt"
	"golang.org/x/net/http2"
	"net/http"

	"net"
	"time"
)

type serverHandler struct {
}

func (sh *serverHandler) ServeHTTP(w http.ResponseWriter, req *http.Request) {
	fmt.Println(req)
	w.Header().Set("server", "h2test")
	w.Write([]byte("this is a http2 test sever"))
}

func main() {
	server := &http.Server{
		Addr:         ":8990",
		Handler:      &serverHandler{},
		ReadTimeout:  5 * time.Second,
		WriteTimeout: 5 * time.Second,
	}
	//http2.Server.ServeConn()
	s2 := &http2.Server{
		IdleTimeout: 1 * time.Minute,
	}
	http2.ConfigureServer(server, s2)
	l, _ := net.Listen("tcp", ":8990")
	defer l.Close()

	fmt.Println("Start server...")
	for {
		rwc, err := l.Accept()
		if err != nil {
			fmt.Println("accept err:", err)
			continue
		}
		go s2.ServeConn(rwc, &http2.ServeConnOpts{BaseConfig: server})

	}
	//http.ListenAndServe(":8888",&serverHandler{})
}
{% endhighlight %}

<!--
https://blog.csdn.net/xcl168/article/details/53869911
-->

整个协议建立连接过程如下。

![start up process]({{ site.url }}/images/network/http2-start-up-process.png "start up"){: .pull-center }

<!--
详见
http://www.blogjava.net/yongboy/archive/2015/03/18/423570.html
-->

## 帧 Frame

HTTP2 的最小数据单位是帧，所有帧以 9 字节的帧头并跟着 `0~16,383` 字节的数据。

{% highlight text %}
+-----------------------------------------------+
|                 Length (24)                   |
+---------------+---------------+---------------+
|   Type (8)    |   Flags (8)   |
+-+-------------+---------------+-------------------------------+
|R|                 Stream Identifier (31)                      |
+=+=============================================================+
|                   Frame Payload (0...)                      ...
+---------------------------------------------------------------+
{% endhighlight %}

### PING

PING：Type=0x6，发送端测量最小的RTT时间，检测连接是否可用


PING 帧定义了以下的Flags:ACK (0x1) ： 位1表示PING帧是一个PING响应。1）PING帧可以被任何终端任何时刻发送。2）PING帧必须在载体中包含一个8字节长度的数据。收到不含ACK的PING帧必须发送一个有ACK的PING响应，带相同的载荷。PING响应应设置比其他帧更高的优先级。流ID为0，不和任何流关联；


### SETTINGS

1）设置帧由两个终端在连接开始时发送，连接生存期的任意时间发送。2）设置帧的参数将替换参数中现有值，不能识别的忽略。3）设置帧总是应用于连接，而不是一个单独的流。

流ID必须为0；SETTINGS 帧定义了以下的 Flags：ACK (0x1) ： 位1，表示设置帧被接收端接收并应用。

如果设置了ACK，设置帧的载体必须为空。


## 流 Stream

流是一个逻辑上的概念，代表 HTTP/2 连接中在客户端和服务器之间交换的独立双向帧序列，每个帧的 Stream Identifier 字段指明了它属于哪个流，流有以下特性:

单个 h2 连接可以包含多个并发的流，两端之间可以交叉发送不同流的帧
流可以由客户端或服务器来单方面地建立和使用，或者共享
流可以由任一方关闭
帧在流上发送的顺序非常重要，最后接收方会把相同 Stream Identifier (同一个流) 的帧重新组装成完整消息报文





<!--
SETTINGS  ======>

<<<<< SETTINGS 连接配置参数帧 Type=0x4 用于设置参数

消息体可以包含多个参数，每个包含一个 16 位标识以及一个 32 位的值。

HTTP2的优先级介绍
https://juejin.im/post/5c1d9b8ae51d4559746922de
https://blog.csdn.net/liujiyong7/article/details/64478317



https://http2.github.io/

http2交互流程
http://www.blogjava.net/yongboy/archive/2015/03/18/423570.html
https://www.jianshu.com/p/40378501d1fc

golang http2
https://juejin.im/entry/5b3359966fb9a00e4e47c35e
curl http2支持
https://www.sysgeek.cn/curl-with-http2-support/
通过wireshark分析
https://imququ.com/post/http2-traffic-in-wireshark.html
HTTP2实现
一个不错的库，curl在使用
https://github.com/nghttp2/nghttp2
很小，基本上就一个文件
https://github.com/douglascaetano/http2
依赖一个小型的HPACK库和S2N安全库
https://github.com/64/hh
https://github.com/awslabs/s2n/
https://github.com/pyos/libcno
https://github.com/gregory144/prism-web-server
https://github.com/Niades/http2/tree/dev/src
https://github.com/pepsi7959/http2
https://github.com/NickNaso/nhttp2

https://github.com/h2o/h2o  *** 直接参考这个吧
https://github.com/nghttp2/nghttp2 功能比较详细就是有些复杂
https://github.com/h2o/picohttpparser
https://www.slideshare.net/kazuho/h2o-20141103pptx

https://github.com/ngtcp2/ngtcp2
https://github.com/douglascaetano/http2
https://www.cnblogs.com/yingsmirk/p/5248506.html
https://ye11ow.gitbooks.io/http2-explained/content/part6.html
https://www.jianshu.com/p/47d02f10757f
https://github.com/zqjflash/http2-protocol
https://blog.csdn.net/zqjflash/article/details/50179235


这里的blog有很多不错的关于HTTP2协议的介绍
https://imququ.com/post/content-encoding-header-in-http.html
-->


## 参考

* HTTP2 的协议，也就是 RFC7540 的详细介绍 [Hypertext Transfer Protocol Version 2](https://tools.ietf.org/rfc/rfc7540.txt) 。
* 关于 HTTP2 的所有版本实现可以参考 [HTTP2 Implementations](https://github.com/http2/http2-spec/wiki/Implementations) 。
* [High Performance Browser Networking HTTP2](https://hpbn.co/http2/) 或者 [本地文档](/reference/linux/http2_protocol.html)。

{% highlight text %}
{% endhighlight %}
