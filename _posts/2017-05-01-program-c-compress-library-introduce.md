---
title: 压缩库使用
layout: post
comments: true
language: chinese
category: [misc, linux]
keywords:
description:
---


<!-- more -->

## 简介

zlib 库已经提供了基本的压缩、解压缩算法，详细的版本可以参考 [zlib.net](http://zlib.net/) 中的介绍。

<!--
#include <zlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
        int rc;
        char *buff;
        uLong tlen, clen;
        char text[] = "zlib compress and uncompress test";

        tlen = strlen(text) + 1;
        clen = compressBound(tlen);

        buff = (char *)malloc(clen);
        if (buff == NULL) {
                fprintf(stderr, "create compress buffer failed, out of memory.\n");
                return -1;
        }

        rc = compress(buff, &clen, text, tlen);
        if (rc != Z_OK) {
                fprintf(stderr, "compress failed, rc %d.\n", rc);
                return -1;
        }

        rc = uncompress(text, &tlen, buff, clen);
        if (rc != Z_OK) {
                fprintf(stderr, "uncompress failed, rc %d.\n", rc);
                return -1;
        }

        printf("%s\n", text);
        free(buff);

        return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <zlib.h>

#include <errno.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define CHUNK 4096

int compress_file(int src, int dst)
{
        z_stream stream;
        int rc, flush, nbytes;
        uint8_t inb[CHUNK], outb[CHUNK];

        memset(&stream, 0, sizeof(z_stream));
        rc = deflateInit(&stream, 9);
        if (rc != Z_OK) {
                fprintf(stderr, "deflate init failed, rc %d.\n", rc);
                return -1;
        }

        do {
                flush = Z_NO_FLUSH;
                rc = read(src, inb, sizeof(inb));
                if (rc < 0) {
                        fprintf(stderr, "read from source failed, %d:%s.\n",
                                errno, strerror(errno));
                        deflateEnd(&stream);
                        return -1;
                } else if (rc == 0) {
                        flush = Z_FINISH;
                }
                stream.avail_in = rc;
                stream.next_in = inb;

                do {
                        stream.avail_out = CHUNK;
                        stream.next_out = outb;
                        deflate(&stream, flush);

                        nbytes = CHUNK - stream.avail_out;
                        rc = write(dst, outb, nbytes);
                        if (rc != nbytes) {
                                fprintf(stderr, "write to destination failed rc %d, %d:%s.\n",
                                                rc, errno, strerror(errno));
                                deflateEnd(&stream);
                                return -1;
                        }
                } while (stream.avail_out == 0);
        } while (flush != Z_FINISH);

        deflateEnd(&stream);
        return 0;
}

int decompress_file(int src, int dst)
{
        z_stream stream;
        int rc, nbytes, nwrite;
        uint8_t inb[CHUNK], outb[CHUNK];


        memset(&stream, 0, sizeof(z_stream));
        rc = inflateInit(&stream);
        if (rc != Z_OK) {
                fprintf(stderr, "inflate init failed, rc %d.\n", rc);
                return -1;
        }

        do {
                rc = read(src, inb, sizeof(inb));
                if (rc < 0) {
                        fprintf(stderr, "read from source failed, %d:%s.\n",
                                errno, strerror(errno));
                        deflateEnd(&stream);
                        return -1;
                } else if (rc == 0) {
                        break;
                }
                stream.avail_in = rc;
                stream.next_in = inb;

                do {
                        stream.avail_out = CHUNK;
                        stream.next_out = outb;

                        rc = inflate(&stream, Z_NO_FLUSH);
                        if (rc == Z_NEED_DICT || rc == Z_DATA_ERROR || rc == Z_MEM_ERROR) {
                                fprintf(stderr, "inflate failed, rc %d.\n", rc);
                                inflateEnd(&stream);
                                return -1;
                        }

                        nbytes = CHUNK - stream.avail_out;
                        nwrite = write(dst, outb, nbytes);
                        if (nwrite < 0) {
                                fprintf(stderr, "write file failed, rc %d.\n", nwrite);
                                inflateEnd(&stream);
                                return -1;
                        }
                } while (stream.avail_out == 0);
        } while (rc != Z_STREAM_END);

        inflateEnd(&stream);
        return 0;
}

int main(void)
{
        int fds, fdd;

        fds = open("main.c", O_RDONLY);
        fdd = open("main.c.zip", O_CREAT | O_WRONLY, 0644);
        compress_file(fds, fdd);
        close(fds);
        close(fdd);

        fds = open("main.c.zip", O_RDONLY);
        fdd = open("main.c.in", O_CREAT | O_WRONLY, 0644);
        decompress_file(fds, fdd);
}

压缩库

zlib
[QuickLZ](http://www.quicklz.com/) 号称压缩速度最快的压缩库。
[LZO](http://www.oberhumer.com/opensource/lzo/) 高性能的压缩库，同时提供了一个很小的 miniLZO 实现；
Snappy 降低压缩率而提高压缩速度，有较高健壮性，由 Google 开源；

除了上面四个比较常用的库，还包括了：A) 采用 LZ77 压缩的 FastLZ 库；B) 针对内存使用优化的 LZF 库。
-->

{% highlight text %}
{% endhighlight %}
