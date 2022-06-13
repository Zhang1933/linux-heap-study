# Understanding the malloc by debugging it.

linux glibc malloc 源码分析。用教学样例调试malloc，free函数来理解堆,malloc的堆管理。

仓库中所给的glibc版本：2.31。没有增删行,行可以和[glibc-2.31源代码](https://elixir.free-electrons.com/glibc/glibc-2.31/source/malloc/malloc.c)对应上。有中文注释。

## 如何使用:

进入`debug-src`目录,直接输入：

```
make
```

然后所在本目录下会生成对应的动态链接，带调试符号的目标文件。通过用gdb调试这个样例程序来理解malloc。

## 看代码&调试相关问题：

* 如何用gdb调试glibc源代码:[Zhang1933-如何用gdb调试glibc源码](https://zhang1933.github.io/2022/06/07/%E5%A6%82%E4%BD%95%E7%94%A8gdb%E8%B0%83%E8%AF%95glibc%E6%BA%90%E7%A0%81.html)

* gdb的一个方便查看堆的插件: [pwndbg](https://github.com/pwndbg/pwndbg)

* 生成 compilation database 用于在看源代码时的跳转查找定义:[rizsotto/Bear](https://github.com/rizsotto/Bear)

## 一个堆的CTF WarGame：

* 配套实践： [shellphish/how2heap](https://github.com/shellphish/how2heap)

