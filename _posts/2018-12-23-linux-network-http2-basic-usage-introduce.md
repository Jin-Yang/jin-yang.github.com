---
title: HTTP2 使用简介
layout: post
comments: true
language: chinese
category: [misc]
keywords: http2,network,hpack
description:
---


<!-- more -->

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

{% highlight text %}
{% endhighlight %}
