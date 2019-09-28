---
title: GoLang 网络编程
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: 
description:
---

Golang 对 TCP 做了很好的抽象，而且提供了很高的性能，其底层的 netpoller 通过非阻塞的 IO 多路复用实现，然后再结合协程的调度，使得用户在编程时实际是阻塞接口。

<!-- more -->

## 简介

### 异常处理

Golang 中的 error 类型是一个内建的 interface 类型。

{% highlight go %}
type error interface {
	Error() string
}
{% endhighlight %}

如果要准确判断不同的错误类型，那么只需要根据类型断言出其错误类型即可。

在标准的网络库中，其错误类型被统一封装为 `net.Error` 类型，其实现在 `net/net.go` 中。

{% highlight go %}
type Error interface {
        error
        Timeout() bool   // Is the error a timeout?
        Temporary() bool // Is the error temporary?
}
{% endhighlight %}

在此基础上，还封装了类似 `net.OpError` `net.ParseError` `net.AddrError` 等类型的错误，而应用程序可以通过 `Temporary()` 接口判断是否为致命的错误。

{% highlight go %}
c, err := l.Accept()
if err != nil {
        if e, ok := err.(net.Error); ok && e.Temporary() {
                logs.Error("accept temporary error: ", e)
                continue
        }

        fmt.Println("accept failed:", err)
        break
}
{% endhighlight %}

### 示例

{% highlight go %}
package main

import (
        "fmt"
        "io"
        "net"
)

func main() {
        l, err := net.Listen("tcp", ":8080")
        if err != nil {
                fmt.Println("listen failed:", err)
                return
        }
        defer l.Close()

        fmt.Println("listening on ':8080'")
        for {
                c, err := l.Accept()
                if err != nil {
                        if e, ok := err.(net.Error); ok && e.Temporary() {
                                logs.Error("accept temporary error: ", e)
                                continue
                        }

                        fmt.Println("accept failed:", err)
                        break
                }

                fmt.Printf("accept client %s(R) -> %s(L)\n", c.RemoteAddr(), c.LocalAddr())
                // handle connections in a new goroutine.
                go handleRequest(c)
        }
}

func handleRequest(conn net.Conn) {
        defer conn.Close()
        for {
                io.Copy(conn, conn) // echo
        }
}
{% endhighlight %}

{% highlight go %}
package main

import (
        "fmt"
        "net"
        "strconv"
        "time"
)

func main() {
        //c, err := net.Dial("tcp", "localhost:8080")
        c, err := net.DialTimeout("tcp", "localhost:8080", 2*time.Second)
        if err != nil {
                fmt.Println("connect failed:", err)
                return
        }
        defer c.Close()
        fmt.Println("Connecting to 'localhost:8080'")

        done := make(chan string)
        go handleWrite(c, done)
        go handleRead(c, done)

        fmt.Println(<-done)
        fmt.Println(<-done)
}

func handleWrite(conn net.Conn, done chan string) {
        for i := 10; i > 0; i-- {
                _, e := conn.Write([]byte("hello " + strconv.Itoa(i) + "\r\n"))
                if e != nil {
                        fmt.Println("send message failed:", e.Error())
                        break
                }
        }
        done <- "Sent"
}

func handleRead(conn net.Conn, done chan string) {
        buf := make([]byte, 1024)

        reqLen, err := conn.Read(buf)
        if err != nil {
                fmt.Println("read message failed:", err)
                return
        }
        fmt.Println(string(buf[:reqLen-1]))
        done <- "Read"
}
{% endhighlight %}

如上代码的 Client 端会通过管道做数据同步，也就是通过两次 `<-done` 等待两个协程退出。

上述的退出方式简单，但是不太灵活，如果要增加协程，那么修改会比较麻烦，建议可以通过 `sync` 包中的 `WaitGroup` 来处理。

<!--
https://colobu.com/2014/12/02/go-socket-programming-TCP/
-->

## 二进制协议

GoLang 提供了一个带缓冲区的 IO 标准库，可以降低对网络或者磁盘的 IO 操作频率。

其中，还提供了一个 Scanner 扫描器，可以将接受到的数据流进行切割。

{% highlight go %}
package main

import (
        "bufio"
        "fmt"
        "strings"
)

func main() {
        input := "foo  bar   baz"

        s := bufio.NewScanner(strings.NewReader(input))
        s.Split(bufio.ScanWords)
        for s.Scan() {
                fmt.Println(s.Text())
        }
}
{% endhighlight %}

注意，如果内存中保存的是字符串或者是 bytes 切片，可以首先考虑使用 `bytes.Split()` 或是 `strings.Split()` 这样的工具集。

最终触发扫描的是在 `Scan()` 函数中，会根据 `Split()` 函数进行切割，该函数会返回三个值。

{% highlight go %}
func Split(data []byte, atEOF bool) (advance int, token []byte, err error)
{% endhighlight %}

分别用来表示，已经处理的字节数，通过 `Text()` 返回的数据，异常信息。

注意，如果 `token` 为 nil 那么实际只是单纯的继续读取并处理函数。

例如，常见的场景有：

* 需要更多的值 `0, nil, nil`
* 如果发生了异常，可以返回 `0, nil, errors.New("message")` ，具体的错误信息通过 `Err()` 接口获取。

另外，默认的最大 Buffer 是 `bufio.MaxScanTokenSize` 一般为 64K ，如果超过了这个范围，则会报 `token too long` 的异常。

如果大小不满足，可以在创建的时候指定。

### 示例

{% highlight go %}
package main

import (
        "bufio"
        "bytes"
        "encoding/binary"
        "errors"
        "fmt"
        "io"
        "log"
        "os"
        "time"
)

type Header struct {
        Magic  [4]byte
        Type   uint16
        Length uint32
}

type Package struct {
        Header
        TagLen  int16
        Tag     []byte
        Message []byte
}

const (
        PKG_S_HDR   = 10
        PKG_S_EXTRA = 2
)

func (p *Package) Pack(w io.Writer) error {
        if err := binary.Write(w, binary.LittleEndian, &p.Header); err != nil {
                return err
        }

        if err := binary.Write(w, binary.LittleEndian, &p.TagLen); err != nil {
                return err
        }

        if err := binary.Write(w, binary.LittleEndian, &p.Tag); err != nil {
                return err
        }

        if err := binary.Write(w, binary.LittleEndian, &p.Message); err != nil {
                return err
        }
        return nil
}


func (p *Package) UnPack(w io.Reader) error {
        if err := binary.Read(w, binary.LittleEndian, &p.Header); err != nil {
                return err
        }

        if err := binary.Read(w, binary.LittleEndian, &p.TagLen); err != nil {
                return err
        }

        p.Tag = make([]byte, p.TagLen)
        if err := binary.Read(w, binary.LittleEndian, &p.Tag); err != nil {
                return err
        }

        p.Message = make([]byte, int(p.Length)-int(p.TagLen)-PKG_S_EXTRA)
        if err := binary.Read(w, binary.LittleEndian, &p.Message); err != nil {
                return err
        }
        return nil
}

func (p *Package) DumpToFile(buff bytes.Buffer, f string) error {
        file, err := os.OpenFile(f, os.O_CREATE|os.O_RDWR|os.O_TRUNC, 0644)
        if err != nil {
                return err
        }
        defer file.Close()

        nwritten, err := file.Write(buff.Bytes())
        if err != nil {
                return err
        }
        log.Printf("Body length %d bytes, total write %d bytes\n", p.Length, nwritten)

        return nil
}

func (p *Package) String() string {
        return fmt.Sprintf("[Magic]%s [Length]%d [Type]0x%x [Tag]%s [Message]%s",
                p.Magic, p.Length, p.Type, p.Tag, p.Message)
}

func split(data []byte, atEOF bool) (advance int, token []byte, err error) {
        var hdr Header

        if atEOF {
                return 0, nil, io.EOF
        }

        if len(data) < PKG_S_HDR {
                return 0, nil, nil // need more data
        }

        if !bytes.Equal(data[:4], []byte{'A', 'G', 'V', '1'}) {
                return 0, nil, errors.New("invalid header")
        }

        binary.Read(bytes.NewBuffer(data), binary.LittleEndian, &hdr)
        length := int(PKG_S_HDR + hdr.Length)

        if len(data) < length {
                return 0, nil, nil
        }

        return length, data[:length], nil
}

func main() {
        buff := new(bytes.Buffer)
        pack := &Package{
                Header: Header{
                        Magic: [4]byte{'A', 'G', 'V', '1'},
                        Type:  0x1234,
                },
                TagLen:  4,
                Tag:     []byte("demo"),
                Message: []byte(("Current time:" + time.Now().Format("2006-01-02 15:04:05"))),
        }
        pack.Length = uint32(pack.TagLen) + uint32(len(pack.Message)) + PKG_S_EXTRA

        for i := 0; i < 3; i++ {
                if err := pack.Pack(buff); err != nil {
                        log.Fatal(err)
                }
        }

        scan := bufio.NewScanner(buff)
        scan.Split(split)

        npkg := Package{}
        for scan.Scan() {
                if err := npkg.UnPack(bytes.NewBuffer(scan.Bytes())); err != nil {
                        log.Fatal(err)
                }
                log.Println(npkg.String())
        }

        if err := scan.Err(); err != nil {
                log.Fatal(err)
        }
}
{% endhighlight %}

{% highlight go %}
{% endhighlight %}
