---
title: Golang Web 测试
layout: post
comments: true
language: chinese
category: [program]
keywords:
description:
---

除了 Go 语言提供的基础 Testing 框架之外，对于 Web 来说，同样提供了 httptest 的测试工具，以及 SQL 的工具。

<!-- more -->

## HTTPTest

### 示例

如下是实现的一个简单的 HealthCheck 接口。

{% highlight go %}
// FILE: ping.go
package main

import (
        "log"
        "net/http"
)

func PingHandler(w http.ResponseWriter, r *http.Request) {
        w.Header().Set("Content-Type", "application/json")
        w.Write([]byte(`{"alive":true}`))
}

func main() {
        http.HandleFunc("/ping", PingHandler)
        if err := http.ListenAndServe(":12345", nil); err != nil {
                log.Fatal("Listen failed: ", err)
        }
}
{% endhighlight %}

{% highlight go %}
// FILE: ping_test.go
package main

import (
        "net/http"
        "net/http/httptest"
        "testing"
)

func TestPingHandler(t *testing.T) {
        // 用来构建模拟请求数据
        req, err := http.NewRequest("GET", "/ping", nil)
        if err != nil {
                t.Fatal(err)
        }

        // 满足ResponseWriter可以用来记录返回的响应
        rr := httptest.NewRecorder()
        PingHandler(rr, req)

        if s := rr.Code; s != http.StatusOK {
                t.Errorf("Invalid status code, %d != %d", s, http.StatusOK)
        }
        e := `{"alive":true}`
        if rr.Body.String() != e {
                t.Errorf("Invalid body, '%s' != '%s'", rr.Body.String(), e)
        }
}
{% endhighlight %}

然后可以通过 `go test` 进行测试。

<!--
https://www.cnblogs.com/Detector/p/9769840.html
-->


{% highlight go %}
{% endhighlight %}
