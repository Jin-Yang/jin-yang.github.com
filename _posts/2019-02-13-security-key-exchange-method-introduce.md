---
title: 密钥交换算法
layout: post
comments: true
language: chinese
category: [misc]
keywords: 加密算法
description:
---


<!-- more -->




## DH

Diffie-Hellman, DH 密钥交换是最早的密钥交换算法之一，使得通信双方能在非安全的信道中安全的交换密钥，用于加密后续的通信消息。

改算法由 Whitfield Diffie 和 Martin Hellman 于 1976 提出，之后被应用于安全领域，常见的如 HTTPs 使用的就是 DH 作为交换算法。

与很多加密算法一样，DH 加密算法也是基于一数学难题：离散对数问题。

简单的说，对于公式 `A=g^a(mod p)` 来说，当已知 `g` `a` `p` 时求 `A` 很容易，但是，如果从 `A` `g` `p` 来求 `a` 会很难。

秘钥交换的流程大致如下：

1. 假设 Bob 和 Alice 共同约定一个素数 `p` 以及根 `g` ，假设 `p=13` 以及 `g=6`，这两个值是公开的；
2. Bob 和 Alice 各生成了一个随机的秘密整数 `B=4` 和 `A=5`，该整数范围是 `[1, p-2]` ，然后通过公式 `g^a(mod p)` 进行计算，分别得到 `6^5 mod 13=2` 以及 `6^4 mod 13=9`；
3. 各自将上述生成的值进行交换，并同样通过 `g^a(mod p)` 计算一个加密使用的 Key 。

最后实际计算得到的是 `(g^A mod p)^B mod p` 以及 `(g^B mod p)^A mod p` ，两者都可以简化为 `g^(A*B) mod p` 。

注意，如上所示，所谓的 DH 密钥交换，实际上并没有交换任何 "密钥"，而是通过交换数据最终双方能够生成同样的密钥。

假设作为一名攻击者，截获双方的数据并尝试进行破解，拿上例来说，尝试对 Bob 数据进行破解，其中已知或者可以截获的有 ：

1. 最终要获取到 s 。已知 `s=A^b%p` 其中 A 和 p 已知，那么要想获取到密钥 s 必须知道 b 。
2. 而根据 `B=g^b%p` 其中 `B` `g` `p` 已知，但离散对数问题说明了 b 很难被破解。

图片不错
https://www.practicalnetworking.net/series/cryptography/diffie-hellman/
https://l2x.gitbooks.io/understanding-cryptography/docs/chapter-3/diffie-hellman%E5%AF%86%E9%92%A5%E4%BA%A4%E6%8D%A2.html



{% highlight text %}
{% endhighlight %}
