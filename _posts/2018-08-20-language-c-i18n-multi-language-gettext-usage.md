---
title: C 多语言支持
layout: post
comments: true
language: chinese
category: [linux,program]
keywords: linux,c,gettext
description: 可以通过 gettext 进行处理，一个在 Linux 上的 i18n 国际化常用的解决方案。	
---

可以通过 gettext 进行处理，一个在 Linux 上的 i18n 国际化常用的解决方案。	

<!-- more -->

## 示例

在进行翻译的时候需要指定翻译哪些字符串，如下是一个简单示例。

{% highlight c %}
#include <stdio.h>
#include <locale.h>
#include <libintl.h>

#define _(string) gettext(string)

int main(void)
{
        //char *info = _("World"); // NOT WORK
        char info[64];

        setlocale(LC_ALL, "");     // OR "zh_CN.utf8"
        bindtextdomain("foobar", "langs");
        textdomain("foobar");

        strncpy(info, _("World"), sizeof(info));
        printf(_("Hello %s!\n"), info);

        return 0;
}
{% endhighlight %}

使用 `gcc -o foobar foobar.c` 生成对应二进制文件，然后通过 `LC_ALL="en_US.utf8" ./foobar` 或者 `LC_ALL="zh_CN.utf8" ./foobar` 进行测试，此时一般会输出是 `Hello World!` 。

对上述的代码比较核心的内容介绍如下。

* `setlocale(LC_ALL, "");` 设置区域，如果第二参数为 `""` 表示设置成系统环境变量指定的值；
* `bindtextdomain("foobar", "langs");` 设置语言文件的查找路径，其中 `langs` 指定的是当前路径下的目录；
* `textdomain("foobar");` 设置当前程序需要的 domain 需要与上述的第一个参数保持一致。

如果程序使用了多个 `.mo` 文件，那么就需要通过 `bindtextdomain()` 指定多次，然后通过 `textdomain()` 选择对应的文件。

在 Linux i18n 中，资源文件是二进制的 `.mo` 文件，使用工具生成，之所以是二进制，估计是为了性能，那么，上述的 `bindtextdomain()` 设置，实际上就会查找 `langs/foobar.mo` 文件。

然后将所有要翻译的字符串写成 `gettext("STR")` 的形式，上述是使用的一个宏，方便进行修改。

## 生成翻译文件

首先生成翻译模板 (Portable Object Template) 文件 `foobar.pot` 。

{% highlight text %}
$ xgettext -k_ foobar.c --add-comments --add-location --default-domain foobar \
        --no-wrap --copyright-holder="Foobar Copyright Message"               \
        --package-name="Foobar Package Name" --package-version="v1.0.0"       \
        --msgid-bugs-address="foobar@mail.com" -o foobar.pot
{% endhighlight %}

此时如果直接通过 `msgfmt` 转换会报错，可以通过如下方式初始化为 Portable Object 文件。

{% highlight text %}
$ msginit --no-translator --locale zh_CN.UTF-8 --output-file foobar.po --input foobar.pot
{% endhighlight %}

然后，修改上述文件中的一些常见字段，包括了编辑第一作者 (FIRST AUTHOR)、翻译者 (Last Translator)、语言工作组 (Language Team) 等信息，大致如下。

{% highlight text %}
msgid ""
msgstr ""
"Project-Id-Version: Foobar Package Name v1.0.0\n"
"Report-Msgid-Bugs-To: foobar@mail.com\n"
"POT-Creation-Date: 2012-06-01 19:33+0800\n"
"PO-Revision-Date: 2012-08-03 14:14+0800\n"
"Last-Translator: Your Name<foobar@email.com>\n"
"Language-Team: Simplified Chinese <foobar@hi.org>\n"
"Language: zh_CN\n"
"MIME-Version: 1.0\n"
"Content-Type: text/plain; charset=UTF-8\n"
"Content-Transfer-Encoding: 8bit\n"

#: foobar.c:16
msgid "World"
msgstr "世界"

#: foobar.c:17
#, c-format
msgid "Hello %s!\n"
msgstr "你好 %1$s!\n"
{% endhighlight %}

然后创建一个子目录，用来保存生成的 `.mo` 文件。

{% highlight text %}
# mkdir -p language/zh_CN/LC_MESSAGES
# msgfmt -o langs/zh_CN/LC_MESSAGES/foobar.mo foobar.po
{% endhighlight %}

如果此时再执行 `foobar` 文件会直接翻译成中文。

## 其它

可以通过 `msgunfmt *.mo -o *.po` 命令反向生成 `po` 文件，如果有新的翻译项，可以直接通过 `msgmerge` 合并新的 `pot` 文件。

<!--
msgmerge messages.pot zh_CN.po -o zh_CN2.po
-->

### 注解

在使用 `gettext` 工具进行分析时，也可以使用一些特殊的注解，通常以 `,` 开头，常见的有：

* `, fuzzy` 翻译可能不正确，需要重新 review 之后再删除；
* `, c-format` 信息会使用到 C 语言的 printf ，例如 `%d` `%s` 等。

## 参考

* 官方相关的介绍 [Overview of GNU gettext](https://www.gnu.org/software/gettext/manual/html_node/Overview.html) 。

<!--
https://upload.wikimedia.org/wikipedia/commons/6/6b/Gettext.svg
GNU gettext工具简介
https://www.atjiang.com/gnu-gettext-intro/
http://wen00072.github.io/blog/2014/05/30/study-on-gettext/
-->



{% highlight text %}
{% endhighlight %}
