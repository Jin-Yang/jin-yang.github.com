---
title: 通讯协议 WebSocket 协议详解
layout: post
comments: true
language: chinese
tag: [Program, Network, Protocol]
keywords: websocket,protocol,http,tcp
description: WebSocket 是一个基于 TCP 的全双工协议，这样允许用户在页面上可以更实时的响应用户的请求，相比之前的 AJAX 也更加灵活，这里详细介绍协议的使用。
---

对于 HTTP 协议，通信只能由客户端发起，服务端无法主动向客户端推送信息，通过轮询方式就很消耗资源。

为了达到实时响应以及良好的用户体验，通常通过 AJAX 实现异步无刷新请求功能，不过只能在浏览器异步请求服务器，也就是拉取；而推送一般通过类似 Comet 的技术，略显复杂。

而 WebSocket 是一种全新的协议，将 TCP 的 Socket 应用在了 web page 上，从而使通信双方建立起一个保持在活动状态连接通道，并且属于全双工工作模式。

<!-- more -->

## 简介

WebSocket 协议的标准为 [RFC6455](https://tools.ietf.org/html/rfc6455) ，相比其它协议 (例如 HTTP) 来说，要简单很多。该协议会借用 HTTP 协议的 `101 switch protocol` 来达到协议转换的，从 HTTP 协议切换成 WebSocket 通信协议，而且没有同源的限制，底层基于 TCP 协议，客户端可以与任意服务器通信，协议标识符是 ws (如果加密，则为wss)。

该协议是基于 Frame 而非 Stream ，也就是说，数据的传输不是像传统的流式读写一样按字节发送，而是采用一帧一帧的 Frame，并且每个 Frame 都定义了严格的数据结构，因此所有的信息就在这个 Frame 载体中。

默认情况下，WebSocket 协议使用 80 端口建立普通连接，加密的 TLS 连接默认使用 443 端口，与 HTTP 相同。

### 控制帧 VS. 数据帧

控制帧用来标识 WebSocket 的状态信息、控制分片、关闭连接等等。如果有 Payload 数据那么必须要小于 125 字节，而且不允许被分片，目前的话，包括了 Opcode <!--为0x0（持续的帧）--> `0x08` 关闭连接、`0x09` Ping 帧、`0x0A` Pong 帧。

而数据帧是用来真正发送数据，包括了 Opcode 为是 `0x01` UTF-8 文本和 `0x02` 二进制数据两种。

### 服务器握手响应

首先需要浏览器主动发起一个 HTTP 请求，每一个响应头之间用 `\r\n` 间隔，最后再放一个 `\r\n` 空行，请求内容大致如下：

{% highlight text %}
GET HTTP/1.1
Host: localhost:9999
Connection: Upgrade
Upgrade: websocket
Sec-WebSocket-Version: 13
Sec-WebSocket-Key: UjxPJpGjxC4JH5+0znrYBg==
Sec-WebSocket-Protocol: chat, superchat
{% endhighlight %}

其中包含了几个关键的字段：

* Upgrade 内容为 websocket，该字段标示改变 HTTP 协议版本或换用其他协议，这里是换用了 websocket 协议。
* Sec-WebSocket-Key 一个随机的经过 Base64 编码的字符串 (16字节)，像密钥一样用于服务器和客户端的握手过程。

当服务器接收到来自客户端的 upgrade 请求，就将请求头中的 `Sec-WebSocket-Key` 字段提取出来，追加一个固定的 `258EAFA5-E914-47DA-95CA-C5AB0DC85B11` 字符串，并进行 `SHA-1` 加密，然后再次经过 Base64 编码生成一个新的 key，作为响应头中的 `Sec-WebSocket-Accept` 字段的内容返回给浏览器。

浏览器接收到来自服务器的响应，便会解析响应中的 `Sec-WebSocket-Accept` 字段，与自己加密编码后的串进行匹配，一旦匹配成功，便可以准备建立 WebSocket 链接了。

然后服务器响应类似如下的 HTTP 报文。

{% highlight text %}
HTTP/1.1 101 Switching Protocols
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Accept: 4O33ZinyFxWKCaxf7T4yCA==
{% endhighlight %}

另外，客户端向服务器发起握手请求的 header 中，可以通过 `Sec-WebSocket-Protocol` 字段指定一个子协议，服务器在建立连接的响应头中包含同样的字段，内容就是选择的子协议之一。

还可以使用 `Sec-WebSocket-Extensions` 头部进行扩展。

### 协议

接下来的所有数据传输就是通过 WebSocket 协议了，传输数据都以 Frame 形式实现，类似于 TCP/UDP 协议中的报文段 Segment ，也就是将数据进行了分割。

{% highlight text %}
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-------+-+-------------+-------------------------------+
 |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
 |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
 |N|V|V|V|       |S|             |   (if payload len==126/127)   |
 | |1|2|3|       |K|             |                               |
 +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
 |     Extended payload length continued, if payload len == 127  |
 + - - - - - - - - - - - - - - - +-------------------------------+
 |                               |Masking-key, if MASK set to 1  |
 +-------------------------------+-------------------------------+
 | Masking-key (continued)       |          Payload Data         |
 +-------------------------------- - - - - - - - - - - - - - - - +
 :                     Payload Data continued ...                :
 + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
 |                     Payload Data continued ...                |
 +---------------------------------------------------------------+
{% endhighlight %}

如下简单介绍各个字段的含义。

#### Flag

目前只包含了一个 FIN 标志位，标识是否为最后一个报文，一般只有数据报文分片之后，该位才可能是 1 。

#### OpCode

操作码，该值决定了数据报文的格式，如果非合法的操作码，那么应该断开连接，操作码包括了：

* `%x00` 数据的延续帧，表示本次数据传输采用了分片，当前收到的是其中一个分片，可能是中间分片也可能是最后一片。
* `%x01` 文本帧，采用的是 UTF-8 编码。
* `%x02` 二进制帧，具体含义由应用层定义。
* `0x08` 连接断开。
* `%x09` PING 操作。
* `%x0A` PONG 操作。

其中 `0x00` 到 `0x07` 是数据帧，`0x03` 到 `0x07` 目前保留，暂时没有用到；而 `0x08` 到 `0x0A` 为控制帧，其 Payload 长度限制为 125 字节，而且不允许数据分片。

#### Payload Length

根据第二个字节决定了长度对应不同的大小。

* `0~125` 直接对应了具体的长度。
* `126` 接下来 2 字节无符号整数才是该帧的长度
* `127` 接下来 8 字节无符号整数才是该帧的长度，高位必须为 0 。

也就是说，数据 PayLoad 的长度是通过一个变长的表示方式表示。

#### Masking-key

当 mask 字段的值为 1 时，payload-data 字段的数据需要经这个掩码进行解密；如果是 0 ，那么数据的解析就不涉及到掩码，直接使用就行。

注意，如果消息是从客户端发送到服务器的，那么 mask 一定是 1，而且 Masking-key 一定是一个 32bit 的值。

至于原因下面再详细介绍。

## 数据传输

当建立好 WebSocket 连接之后，后续的操作基本都是基于数据帧的传递，也就是说 opcode 为 `0x00~0x02`，不过此时可能会发生数据分片，需要通过 FIN 的值来判断是否结束，当为 1 时已经结束，可以对消息进行处理了，否则需要继续等待。

其中 opcode 为 `0x01` 表示消息是文本，`0x02` 为二进制，而 `0x00` 表示为延续帧，也就是完整消息的数据帧还没有接收完，如下是一个简单示例。

```
Client: FIN=1, opcode=0x1, msg="Hello"
Server: (process complete message immediately) Hi.
Client: FIN=0, opcode=0x1, msg="and a"
Server: (listening, new message containing text started)
Client: FIN=0, opcode=0x0, msg="happy new"
Server: (listening, payload concatenated to previous message)
Client: FIN=1, opcode=0x0, msg="year!"
Server: (process complete message) Happy new year to you too!
```

也就是客户端发送了 `Hello` ，然后服务端立即响应 `Hi` ，接着服务端发送 `and a happy new year!` ，最后服务端收到完整消息后响应 `Happy new year to you too!` 。

<!--
理论上来说，每个帧 (Frame) 的大小是没有限制的，但是发送的数据有不能太大，否则无法高效的利用网络带宽 (由于MSS)，如果要传输大片数据就需要使用分片。
-->

## 关闭连接

正常关闭 WebSocket 连接就是发送 opcode 为 `0x08` 的报文，可以包含具体关闭连接的原因，一旦一端接收到了 Close 报文，那么就开始了 Close HandShake ，并进入到 Closing 状态。

关闭帧前两个字节为错误码，详见如下，按照大端发送；接着，可选择发送关闭连接的具体原因 (UTF-8 编码)。

当收到 Close 帧之后，如果之前没有向对方发送过 Close 帧，那么就需要发送一个错误码相同的帧，一般来说会立即发送错误帧，当然，也可以将当前正在发送的帧发送完成后再发送。

Close 消息发送完之后，服务端会立即关闭 Socket 连接，而客户端一般来说需要等待服务端关闭连接，但也可以主动关闭，例如发送 Close 帧之后一直没有收到连接关闭信息。

### 异常码

详细可以参考 [RFC6455 7.4. Status Codes](https://tools.ietf.org/html/rfc6455#section-7.4) 中的介绍，简单整理如下。

| 代号 | 描述              | 使用场景                                                   |
|:----:|:------------------|:-----------------------------------------------------------|
| 1000 | 正常关闭          | 应用完成数据处理，会话正常关闭                             |
| 1001 | 离开              | 应用离开且不期望后续连接的尝试而关闭连接时                 |
| 1002 | 协议错误          | 因协议错误而关闭连接时                                     |
| 1003 | 不可接受的数据类型| 非二进制或文本类型时                                       |
| 1007 | 无效数据          | 文本格式错误，如编码错误                                   |
| 1008 | 消息违反政策      | 当应用程序由于其他代号不包含的原因时                       |
| 1009 | 消息过大          | 当接收的消息太大，应用程序无法处理时（帧的载荷最大为64字节）|
| 1010 | 需要拓展          |                                                        |
| 1011 | 意外情况          |                                                        |

其中 `4000 ~ 4999` 可以由应用层使用。

## 掩码

简单来说，就是当报文中的 mask 字段的值为 1 时，payload 字段的数据需要通过这个掩码进行计算，而且这个 mask 是 32bit 的值，每次由客户端发送给服务端的时候，都需要通过 Mask 计算，而服务端发送给客户端时是不需要计算的。

当发送数据的时候，每次都需要获取一个最新的 mask ，然后将数据通过如下方式计算。

```
for (i = 0; i < len; i++) {
	data[offset + i] ^= mask[i % 4];
}
```

当到达服务器之后，同样以上述的方式计算即可。

### 原因

这里主要是考虑到网络安全而设置，但显然无法做到加密，因为算法公开的而且很容易解密，实际上，主要是为了防止早期版本协议中存在的代理缓存污染攻击，详细可以参考 [RFC6455 10.3. Attacks On Infrastructure (Masking)](https://tools.ietf.org/html/rfc6455#section-10.3) 中的介绍。

实际上这种方案是无法完全避免的，只是缓解。

<!--
https://github.com/abbshr/abbshr.github.io/issues/22
https://github.com/abbshr/abbshr.github.io/issues/47
https://security.stackexchange.com/questions/36930/how-does-websocket-frame-masking-protect-against-cache-poisoning

https://www.cnblogs.com/chyingp/p/websocket-deep-in.html

一般形式的攻击是跟被攻击者控制的服务器建立连接，并构造一个类似WebSocket握手一样的UPGRADE请求，随后通过UPGRADE建立的连接发送看起来就像GET请求的frame去获取一个已知资源（在攻击场景中可能是一个点击跟踪脚本或广告服务网络中的资源）。

之后远程服务器会返回某些东西，就像对于这个伪造GET请求的响应，并且这个响应会被很多广泛部署的网络中间设备缓存，从而达到了污染缓存服务器的目的。对于这个攻击的产生的效应，可能一个用户被诱导访问受攻击者操控的服务器，攻击者就有可能污染这个用户以及其他共享相同缓存服务用户的缓存服务器，并跨域执行恶意脚本，破坏web安全模型。
应对措施——掩码

为了避免面这种针对中间设备的攻击，以非HTTP标准的frame作为用户数据的前缀是没有说服力的，因为不太可能彻底发现并检测每个非标准的frame是否能够被非HTTP标准的中间设施识别并略过，也不清楚这些frame数据是否对中间设施的行为产生错误的影响。

对此，WebSocket的防御措施是mask所有从客户端发往服务器的数据，这样恶意脚本（攻击者）就没法获知网络链路上传输的数据是以何种形式呈现的，所以他没法构造可以被中间设施误解为HTTP请求的frame。

这就是掩码存在的原因。
继续安全性探究——如何选择掩码？

本来到这里就该结束了， 但是协议很负责的深入说明了掩码选择上的要求~

客户端必须为发送的每一个frame选择新的掩码，要求是这个掩码无法被提供数据的终端应用（即客户端）预测。

算法的选择上，为了保证随机性，可以借助密码学中的随机数生成器生成每个掩码。

倘若使用相同的掩码会有什么后果呢？

假设每次发送frame使用了相同的掩码或下一个掩码如何选择被猜出的话，攻击者就可以发送经过mask后类似HTTP请求的frame（做法很简单：攻击者以希望在网络链路上显示的形式构造数据，然后用下一个掩码mask再发出去）。

至于如何用掩码mask原始数据，在前面的 学习WebSocket协议—从顶层到底层的实现原理（修订版） 中已经说过了——按位做循环异或运算.

除此之外，另一个要求是一旦传输开始，客户端必须不准再修改传输的内容，否则攻击者将会发送一个用已知数据（如全0）初始化的frame，并通过第一部分数据的回执（经过mask的数据）计算本次使用的掩码，然后修改将要发送的frame使之mask后表现的是一个HTTP请求，原理同前面所讲，不再赘述。
什么数据需要Mask？

上面所描述的安全模型重点关注的是客户端发送类HTTP请求的frame给服务器，所以仅仅需要mask从客户端到服务器的数据，反之则没有mask，但是为了完成请求，前提是客户端必须能够伪造请求。因此，并不强制要求mask双向通信，只要保证一方的数据是经过mask的即可。
遗留问题

尽管掩码提供了保护，但不符合规定的HTTP代理服务器仍是那些“不使用掩码的客户端-服务器”攻击对象！
总结

所有内容归结为一句话：为防止攻击者获知网络链路中传输的原始数据，提供不可预测的掩码至关重要。
-->













<!---
其中 JavaScript 的示例程序如下：

{% highlight javascript %}
// 与服务端建立链接
var ws = new WebSocket("wss://echo.websocket.org");

// 建立链接后的回调函数
ws.onopen = function(event) {
	console.log("Connection open ...");
	ws.send("Hello WebSockets!");
};

// 收到服务器数据后的回调函数
ws.onmessage = function(event) {
	// 数据格式可以是文本
	if(typeof event.data === String) {
		console.log("Received Message: " + event.data);
	}

	// 也可能是二进制数据
	if(event.data instanceof ArrayBuffer){
		var buffer = event.data;
		console.log("Received arraybuffer");
	}

	ws.close();
};

// 链接关闭后的回调函数
ws.onclose = function(event) {
	console.log("Connection closed.");
};

// 错误异常处理
socket.onerror = function(event) {
	// handle error event
};
{% endhighlight %}

其中每个实例中有 readyState 属性，用于返回实例对象的当前状态，共有四种。

* CONNECTING：值为0，表示正在连接。
* OPEN：值为1，表示连接成功，可以通信了。
* CLOSING：值为2，表示连接正在关闭。
* CLOSED：值为3，表示连接已经关闭，或者打开连接失败。

对于回调函数，在如上的示例中只指定了一个，也可以通过如下代码添加多个回调函数：

{% highlight javascript %}
ws.addEventListener('open', function(event) {
	ws.send('Hello Server!');
});
{% endhighlight %}

如上收到数据后判断类型时，除了上述动态判断收到的数据类型，也可以使用 binaryType 属性，显式指定收到的二进制数据类型。

{% highlight javascript %}
// 收到的是 blob 数据
ws.binaryType = "blob";
ws.onmessage = function(e) {
	console.log(e.data.size);
};

// 收到的是 ArrayBuffer 数据
ws.binaryType = "arraybuffer";
ws.onmessage = function(e) {
	console.log(e.data.byteLength);
};
{% endhighlight %}
-->

## FAQ

#### PING/PONG 能否携带数据？

可以携带一部分数据，但是作为控制帧，协议规定消息体的长度不应该超过 125 字节，这也就导致实际可以携带的消息有限，所以不建议使用。

## 参考

类似 CGI ，通过标准输入输出作为 WebSocket 的输入输出，很特别但是实用的服务器 [http://websocketd.com/](http://websocketd.com/) ，一个通过前端显示 vmstats 的工程实现，详见 [Github web-vmstats](https://github.com/joewalnes/web-vmstats) 。

基于 [libwebsockets](https://github.com/warmcat/libwebsockets) 实现的工程，包括：数据展示 [JitterTrap](https://github.com/acooks/jittertrap)、共享终端 [ttyd](https://github.com/tsl0922/ttyd) 。

<!--
[WebSocket API](https://developer.mozilla.org/en-US/docs/Web/API/WebSocket) 。

http://www.antyme.com/2016/12/14/WebSocket%E5%8D%8F%E8%AE%AE-SocketRocket%E6%BA%90%E7%A0%81%E5%AD%A6%E4%B9%A0/

https://github.com/uNetworking/uWebSockets
https://github.com/tatsuhiro-t/wslay
https://github.com/mortzdk/Websocket
https://github.com/payden/libwebsock
https://github.com/payden/libwsclient

http://senlinzhan.github.io/2017/02/10/Linux%E7%9A%84TCP-CORK/
https://github.com/nodejs/http-parser
http://blog.mingsixue.com/debug/Fiddler-online-map-localfile.html
-->



{% highlight text %}
{% endhighlight %}
