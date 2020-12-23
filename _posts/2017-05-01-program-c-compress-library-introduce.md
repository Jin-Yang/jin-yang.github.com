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

如下实际上就是 zlib 算法，可以直接使用 golang 的 zlib 库。

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


https://blog.csdn.net/CleverCode/article/details/46691645
https://blog.csdn.net/cordova/article/details/52928432
https://juejin.im/post/5dc2422ff265da4d365f2fff
https://zhangbohun.github.io/2019/04/01/%E6%97%A0%E6%8D%9F%E5%8E%8B%E7%BC%A9%E7%AE%97%E6%B3%95%E7%90%86%E8%AE%BA%E5%AD%A6%E4%B9%A0%E6%80%BB%E7%BB%93/

常用的压缩算法包括了 RLE、Huffman、LZ，目前大部分压缩算法的改进都是基于 LZ  算法，然后在不同的压缩率以及解压缩速度上的平衡。

LZO压缩库
http://www.oberhumer.com/opensource/lzo/
压缩速率比较，以及FB实现的zstd压缩库
https://facebook.github.io/zstd/

https://github.com/lz4/lz4

在 [zlib how](http://www.zlib.net/zlib_how.html) 中有相关示例的介绍，其压缩过程为 `deflateInit()` `deflate()` `deflateEnd()`，对应的解压过程 `inflateInit()` `inflate()` `inflateEnd()` 。

在通过 `deflateInit()` 进行初始化的时候，可以指定 `-1` 到 `9` 的压缩级别，级别越高对应的压缩率越高，但是速度会变慢，其中 `0` 实际没有压缩，`-1` 使用系统默认值。
https://github.com/google/snappy
https://github.com/andikleen/snappy-c

https://www.jianshu.com/p/dc76bcfef553

有可能会依赖不同 cmake 版本，可以通过如下方式安装不同版本的 cmake 。

tar -zxf /opt/bak/cmake-3.13.2.tar.gz -C /opt/app/
cd /opt/app/cmake-3.13.2/
./bootstrap --prefix=/opt/app/cmake-3.13.2/
gmake
gmake install
export CMake_HOME=/opt/cmake-3.13.2/
export PATH=$PATH:$CMake_HOME/bin

## snappy

mkdir build && cd build && cmake .. && make

mkdir example && cd example
g++ -o example example.cc -I.. -I../build -L../build -lsnappy

#include <string>
#include <snappy.h>
#include <iostream>
using namespace std;

int main(void)
{
        string output;
        string input = "Hello World";

        for (int i = 0; i < 5; ++i)
                input += input;
        snappy::Compress(input.data(), input.size(), &output);
        cout << "input size:" << input.size() << " output size:" << output.size() << endl;

        string output_uncom;
        snappy::Uncompress(output.data(), output.size(), &output_uncom);
        if (input == output_uncom)
                cout << "Equal" << endl;
        else
                cout << "ERROR: not equal" << endl;
        return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <snappy-c.h>

int main(void)
{
        int rc;
        char *comp, *uncomp;
        size_t inlen, outlen;
        const char *str = "With great power there must also come great responsibility.";

        inlen = strlen(str) + 1;
        outlen = snappy_max_compressed_length(inlen);
        comp = (char *)malloc(outlen);
        if (comp == NULL) {
                fprintf(stderr, "OOM\n");
                return -1;
        }
        rc = snappy_compress(str, inlen, comp, &outlen);
        if (rc != SNAPPY_OK) {
                fprintf(stderr, "compress failed, rc %d.\n", rc);
                free(comp);
                return -1;
        }
        fprintf(stdout, "compress input length %dB output length %dB.\n", inlen, outlen);

        rc = snappy_uncompressed_length(comp, outlen, &inlen);
        if (rc != SNAPPY_OK) {
                fprintf(stderr, "get uncompress length failed, rc %d.\n", rc);
                free(comp);
                return -1;
        }
        uncomp = (char *)malloc(inlen);
        if (uncomp == NULL) {
                fprintf(stderr, "OOM\n");
                return -1;
        }
        rc = snappy_uncompress(comp, outlen, uncomp, &inlen);
        if (rc != SNAPPY_OK) {
                fprintf(stderr, "uncompress failed, rc %d.\n", rc);
                free(uncomp);
                free(comp);
                return -1;
        }
        fprintf(stdout, "uncompress input length %dB output length %dB.\n", outlen, inlen);
        if (strcmp(str, uncomp) == 0)
                fprintf(stdout, "equal\n");
        else
                fprintf(stdout, "unequal\n");
        free(uncomp);
        free(comp);
        return 0;
}


g++ -o example example.c -I.. -I../build -L../build -lsnappy

-->

{% highlight text %}
{% endhighlight %}
