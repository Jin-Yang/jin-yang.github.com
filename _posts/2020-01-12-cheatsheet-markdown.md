---
title: Markdown 语法速查表
layout: post
comments: true
tag: [CheatSheet]
language: chinese
keywords: markdown,标记,速查表
description: Markdown 是一种很简单的标记语法，可以直接使用普通的文本编辑器编写，在渲染之后具有一定的格式，主要特点就是简单、方便书写。
---

Markdown 是一种很简单的标记语法，可以直接使用普通的文本编辑器编写，在渲染之后具有一定的格式，主要特点就是简单、方便书写。

同时，还有一些衍生版本，用于扩展一些常见的功能，常见的有 Markdown Extra、MultiMarkdown、Maruku 等。

这里简单整理基础语法的速查表，详细语法可以参考 [Markdown Syntax](https://daringfireball.net/projects/markdown/syntax) 中的介绍。

<!-- more -->

## 标题 (Section Headings)

标题等级通过 `#` 数量标识，注意在 `#` 和标题之间需要添加空格。

```
# TEXT         标题1
## TEXT        标题2
### TEXT       标题3
#### TEXT      标题4
##### TEXT     标题5
```

正文段落与段落之间通过空行分割，如果只是换行，可以在行结束添加两个空格。

## 分割线 (Horizontal Rules)

单行且要求至少三个符号，中间可以有空格。注意，有些编辑器在渲染时可能冲突，因为不限长度，可以多写一些字符。

```
***
* * *
---
- - -
___
_ _ _
```

## 字体 (Bold/Italics)

只支持粗体和斜体。

```
__Bold__        粗体
**Bold**        粗体
_Italics_       斜体
*Italics*       斜体
```

## 列表 (Lists)

### 无序列表 (Bulleted Lists)

一般以 `*` `+` `-` 开始，然后紧跟一个空格，如果要嵌套，需要在开始符号之前至少使用两个空格，或者 `Tab` 。

```
* Bullet 01       一级无序列表
* Bullet 02       一级无序列表
  - Bullet 2A     二级无序列表
  - Bullet 2B     二级无序列表
```

### 有序列表 (Numbered Lists)

以数字加 `.` 开始，然后紧跟一个空格，如果要嵌套，需要至少四个空格，或者 `Tab` ，序号会从头开始。另外，只要是数字即可，没有必要按照序号填写。

```
1. Numbered 01        一级有序列表
2. Numbered 02        一级有序列表
3. Numbered 03        一级有序列表
    3. Numbered 3A    二级有序列表
    3. Numbered 3B    二级有序列表
```

## 链接 (Links)

包括了显示的内容以及对应的链接。

```
[Google](https://www.google.com)
[Google](https://www.google.com "Google Site")
```
## 图片 (Images)

与链接类似，如果有多处使用相同图片，可以将具体引用放置到最后统一管理。

```
![Alt Text](/path/to/image.png)
![Alt Text](/path/to/image.png "Title")

![Alt Text](img1)
[img1]: /path/to/image.png "Title"
```

## 引用 (Block Quotes)

通常用于引用一段文正。

```
> hi                     因为中间没空行，会在相同引用中，显示为hi block quote
block quote

> hi                     嵌套引用
>
>> embeded block quote
```

## 代码 (Code)

包括了行内嵌以及代码块。

### 内嵌代码 (Inline Code)

如果内嵌的代码中同时使用了反引号 `` ` `` ，可以使用如下的方式，实际上，只需要保证连续反引号大于引用数即可。

```
Use `<div>` tags.

Use command ``echo `uname -a` `` to get kernel info.
```

### 代码块 (Code Block)

以连续三个反引号 ```` ``` ```` 开始，支持标记语言用于语法高亮显示。

{% highlight text %}
``` python
def helloworld():
	print("Hello World")
```
{% endhighlight %}

## 表格 (Table)

可以设置表格头，以及表中每行的对齐方式，其中 `:---` 左对齐，`:---:` 居中对齐，`---:` 右对齐。

```
| header #1 | Header #2 | Header #3 |
|:----------|:---------:|----------:|
| Row1 Col1 | Row1 Col2 | Row1 Col3 |
| Row2 Col1 | Row2 Col2 | Row2 Col3 |
| Row3 Col1 | Row3 Col2 | Row3 Col3 |
```

## 转义符 (Escapable Characters)

可以在如下的字符前添加 `\` 用来转移，实际上就是上述标记使用的符号。

```
\      Backslash    反斜杠
`      Backtick     反引号
*      Asterisk     星号
_      Underscore   下划线
{}     Curly Braces 大括号
[]     Brackets     中括号
()     Parenthesis  括号
#      Pound        井号
+      Plus         加号
-      Hyphen       减好
.      Period       英文句点
!      Exclamation  感叹号
```

## 注脚 (Footnotes)

标记引用的内容出处。

```
This is a footnotes [^1] for text.

[^1]: foot notes example
```


