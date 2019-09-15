---
title: Golang bytes 模块
layout: post
comments: true
language: chinese
category: [program]
keywords:
description:
---

Golang 中的 bytes 包使用频率较高，主要提供了字节相关的操作，通常作为缓冲区使用。

<!-- more -->

## 简介

### String VS. Bytes

String 是固定的一系列字节，创建之后不能被修改；而 Bytes 可以动态调整。所以，如果要修改字符串的内容，需要转换为 Bytes 。

两者之间可以相互转换，例如：

{% highlight go %}
s := "abc"
b := []byte(s)
s := string(b)
{% endhighlight %}

在两者相互转换时，会先完成内存申请，然后将对应的内容进行拷贝。

<!--
## 扩容
https://halfrost.com/go_slice/
-->


## bytes

`bytes.Buffer` 是一个字节类型的缓冲器，可变长，实际上内部指向的是一个 Slice 对象。

如下是一些常见的操作。

{% highlight go %}
//----- 分配对象可以动态扩容，如果大小是固定可以指定打消
var b bytes.Buffer
b := new(bytes.Buffer)
b := bytes.NewBufferString("hello")
b := bytes.NewBuffer([]byte("hello"))

b.Write([]byte("Hello"))
b.WriteString("Hello")

b.Bytes() // 获取对应的Slice
{% endhighlight %}

在该包中，提供了一个如下的结构体。

{% highlight go %}
type Buffer struct {
	buf       []byte            // contents are the bytes buf[off : len(buf)]
	off       int               // read at &buf[off], write at &buf[len(buf)]
	bootstrap [64]byte          // memory to hold first slice; helps small buffers avoid allocation.
	lastRead  readOp            // last read operation, so that Unread* can work correctly.
}
{% endhighlight %}

数据会保存在 `buf []byte` 对象中，那么无非就是当内存空间不足的时候如何进行扩容。

## 序列化

二进制文件读写方式，可以通过标准库提供的 `encoding/binary` 库。

{% highlight go %}
package main

import (
        "bytes"
        "encoding/binary"
        "log"
        "os"
        "unsafe"
)

const (
        T_WRITE = 0x1234
)

type Package struct {
        Magic  [4]byte
        Type   uint16
        Length uint32
}

func main() {
        file, err := os.OpenFile("foobar.bin", os.O_CREATE|os.O_RDWR|os.O_TRUNC, 0644)
        if err != nil {
                log.Fatal(err)
        }
        defer file.Close()

        // Write
        data := &Package{
                Magic:  [4]byte{'A', 'B', 'C', 'D'},
                Type:   T_WRITE,
                Length: 10,
        }

        buff := new(bytes.Buffer)
        binary.Write(buff, binary.LittleEndian, data)

        nwritten, err := file.Write(buff.Bytes())
        if err != nil {
                log.Fatal(err)
        }
        log.Printf("Write %d bytes\n", nwritten)

        // Read
        if _, err := file.Seek(0, 0); err != nil {
                log.Fatal(err)
        }
        rdata := make([]byte, unsafe.Sizeof(Package{}))

        pkg := Package{}
        nread, err := file.Read(rdata)
        if err != nil {
                log.Fatal(err)
        }
        log.Printf("Read %d bytes\n", nread)
        binary.Read(bytes.NewBuffer(rdata), binary.LittleEndian, &pkg)
        log.Printf("Got data %v\n", pkg)
}
{% endhighlight %}


如果是多个二进制文件，那么就可以通过 `binary.LittleEndian.Uint16` 进行转换。

## 参考

<!--
https://shohi.github.io/tech/2017/06/19/go-read-write-txt-and-bin-file




## 文件读写

除了正常文本文件的读写，还涉及到了二进制的操作。

如下是简单的文件读写，采用的都是文本格式。

package main

import (
        "log"
        "os"
)

func main() {
        var size int64

        //file, err := os.Open("foobar.txt")
        file, err := os.OpenFile("foobar.txt", os.O_CREATE|os.O_RDWR|os.O_TRUNC, 0644)
        if err != nil {
                log.Fatal(err)
        }
        defer file.Close()

        data := []byte("hello world")
        nwritten, err := file.Write(data)
        if err != nil {
                log.Fatal(err)
        }
        log.Printf("Write %d bytes\n", nwritten)

        if _, err := file.Seek(0, 0); err != nil {
                log.Fatal(err)
        }

        if stats, err := file.Stat(); err == nil {
                size = stats.Size()
        }

        if size == 0 {
                log.Println("File maybe empty.")
                return
        }

        bytes := make([]byte, size)
        nread, err := file.Read(bytes)
        if err != nil {
                log.Fatal(err)
        }
        log.Printf("Got %d bytes data: %s\n", nread, bytes)
}
-->

{% highlight go %}
{% endhighlight %}
