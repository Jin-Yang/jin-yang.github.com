---
title: 终端音乐播放器 CMUS 详细介绍
layout: post
comments: true
language: chinese
tag: [SoftWare]
keywords: linux,cmus
description: CMUS 是一个可以在终端上使用的音乐播放器，可以运行在多数类 Unix 的系统上，例如 Linux、Unix、Mac 等，而且支持绝大多数格式的音乐文件，这里详细介绍其使用方法。
---

CMUS 是一个可以在终端上使用的音乐播放器，可以运行在多数类 Unix 的系统上，例如 Linux、Unix、Mac 等，而且支持绝大多数格式的音乐文件。

不过目前已经不再维护了，这里详细介绍其使用方法。

<!-- more -->

![cmus osx](/{{ site.imgdir }}/linux/cmus-osx.png "cmus osx")

## 简介

Console Music Player, CMUS 一个终端上的音乐播放软件，资源占用很少，支持 Unicode 也就意味着对中文支持还不错，支持大多数的音乐格式，例如 FLAC、MP3、WAV、AAC 等。

### 输入输出

CMUS 通过插件的方式实现对应的输出输出格式，详细的可以通过 `cmus --plugin` 命令查看当前支持的格式，常见的输入如 MP3、FLAC、WAV、AAC 等等，输出如 ALSA、PulseAudio、libao、sndio (OpenBSD)、WaveOut (Windows) 等。

也就是说，几乎可以支持任意格式的输入。

## 操作界面

通过数字可以选择对应的界面。

### 1. Artist/Album

默认的是 Artist/Album 界面，包含了所有已经添加的音乐文件列表，也就是所谓的 library ，类似如上图片的内容。

{% highlight text %}
+---------------------------------------------------------------------+
| Artist / Album             Track                            Library |
|                          |                                          |
|                          |                                          |
|                          |                                          |
|                          |                                          |
|                          |                                          |
|                          |                                          |
|                          |                                          |
|                                                                     |
| . 00:00 - 00:00 vol: 100                     all from library | C   |
|                                                                     |
+---------------------------------------------------------------------+
{% endhighlight %}

这里会按照专辑、歌手排列，例如属于同一专辑内容会显示在右侧。

### 2. Library

这里会显示所有已经添加的音乐文件，可以用来播放整个列表。

{% highlight text %}
+---------------------------------------------------------------------+
| Library ~/.config/cmus/lib.pl - 31 tracks sorted by artist album di |
| Flying Lizards         . Money (That's What I Want)           02:31 |
| Jason Woofenden        . VoR Theme                       2009 01:20 |
| Keali'i Reichel      06. Wanting Memories                1994 04:28 |
| Molly Lewis            . Tom Cruise Crazy                     03:13 |
| NonMemory              . pista1                          2009 03:18 |
| NonMemory            01. pista1                    2009-04-21 04:13 |
| Ray Charles          06. Halleluja I Love Her So              02:33 |
|                                                                     |
| . 00:00 - 2:16:25 vol: 100                   all from library | C   |
|                                                                     |
+---------------------------------------------------------------------+
{% endhighlight %}

通过上下方向键选择歌曲，然后回车开始播放，如下是一些常见的快捷键：

* `c` 暂停、取消暂停。
* 左右方向键用来快退、快进，每次间隔 10 秒。
* `<` 和 `>` 也是快退、快进，每次跳转是 1 分钟。
* `z` 播放前一首，`b` 播放下一首。

另外，可以选择播放列表的方式，当前的信息会在右下角显示，默认是 `all from library | C` ，也就是播放所有的 library 中的歌曲，顺序播放。

播放列表也可以选择具体的歌手、专辑，可以通过 `m` 按键在不同的播放列表中进行切换。而播放顺序主要有如下的几种：

* `[C]ontinue` 默认，顺序播放，到最后一首后结束，可以通过 `Shift-C` 切换。
* `[R]epeat` 循环播放，播放到最后一首后从头重新开始播放，可以通过 `r` 切换。
* `[S]huffle` 随机播放，通过 `s` 切换。

<!--
[F]ollow
    If this is on, cmus will select the currently playing track on track change.
Press *f* to toggle this option.
-->

### 3 播放列表

与 library 类似，不过是可以根据自己的兴趣定制，通过 `y` 将当前歌曲添加到播放列表中，可以通过 `p` `P` 调整顺序，使用 `shift-D` 从列表中删除。

### 4 使用队列 (Queue)

这是一个先进先出的队列，用来临时保存播放列表，通常在听某一首歌的时候想一首指定的下首歌，可以在任意界面中通过 `e` 按键将当前歌曲添加到队列中。

在当前歌曲播放完后会播放下首，同时会将当前歌曲从队列中删除，而且当前不会受随机播放模式的影响。

可以通过 `4` 切换到队列页面，同样可以使用 `p` 和 `P` 调整顺序，通过 `shift-D` 从列表中删除。

## 常见操作

### 添加音乐

通过 `5` 切换到文件浏览页面，通过上下方向键、回车键 (Enter)、删除键 (Backspace) 切换目录，最终找到所需的音乐文件或者目录。通过按键 `a` 会添加文件/目录到 library 中，添加完或会自动切换到下一行，所以可以通过多次 `a` 添加文件。

注意，如果添加的是目录，而且目录下有很多文件时，添加会比较慢。

添加完之后，可以通过 `:save` 命令保存已经添加的文件信息，当然，在退出的时候也会保存。

### 总结

CMUS 分了好几个界面，可以通过 `1~7` 数字进行切换。

| 操作         | 按键/命令                |
|:-------------|:-------------------------|
| 导入音乐     | :add /file/path 或 5     |
| 暂停/继续    | c                        |
| 重新播放     | x                        |
| 播放下首     | b                        |
| 播放上首     | z                        |
| 清空列表     | :clear                   |
| 保存播放列表 | :save /path/to/playlist  |
| 加载播放列表 | :save /path/to/playlist  |
| 退出         | q                        |


{% highlight text %}
{% endhighlight %}
