# Understanding the heap by debugging it

[Painless intro to the Linux userland heap](https://sensepost.com/blog/2017/painless-intro-to-the-linux-userland-heap/):
> The best source of knowledge with regards to the implementation of the heap is itself, the source code.


通过手动构造的样例动态调试malloc，free函数进行源码分析来理解堆、堆管理。

* 仓库中所给的glibc版本：2.31。没有增删行,行可以和[glibc-2.31源代码](https://elixir.free-electrons.com/glibc/glibc-2.31/source/malloc/malloc.c)对应上。有中文注释。


## 如何使用:

**注意：**样例仅支持64位linux操作系统!

直接输入：

```
make
```

目录下会生成对应的动态链接，带调试符号的目标文件。通过用gdb调试这个样例程序进入库源码动态调试理解堆管理。

## 分析源代码&调试相关问题：

* 如何用gdb调试glibc源代码:[Zhang1933-如何用gdb调试glibc源码](https://zhang1933.github.io/2022/06/07/%E5%A6%82%E4%BD%95%E7%94%A8gdb%E8%B0%83%E8%AF%95glibc%E6%BA%90%E7%A0%81.html)

* gdb的一个方便查看堆的插件: [pwndbg](https://github.com/pwndbg/pwndbg)

* 生成 compilation database 用于在看源代码时的跳转查找函数&宏定义:[rizsotto/Bear](https://github.com/rizsotto/Bear)

## Putting it all together

### malloc分配策略总结：

Glibc 2.26以上版本，tcache 默认情况下开启:[Glibc Enables A Per-Thread Cache For Malloc - Big Performance Win](https://www.phoronix.com/scan.php?page=news_item&px=glibc-malloc-thread-cache)。这里分析时用默认情况。

chunk分配步骤总述：
    1. 如果请求的chunk大小不超过tcache bins 中的最大chunk且tcache bins中对应大小的bin中有空chunk,直接返回。
    2. 否则继续执行。判断请求的chunk大小不超过fastbins中最大chunk大小，且请求大小对应的fastbin有空chunk。如果判断为真,直接返回。
    3. 否则继续执行。判断请求的chunk大小是否不超过samllbins的最大大小且对应的smallbins有空余chunk。补充tcachebin并返回。(优先考虑smallbins因为smallbins不像unsorted bins需要遍历。)
    4. 否则继续执行。如果fastbins非空，调用malloc_consolidate函数合并fastbin上的物理相邻的chunk。如果准备合并的chunk物理上下一个chunk是top chunk，则与top chunk合并。否则放到unsorted bin里。(tcache bins中的chunk不会参与合并。)
    5. 继续执行。在unsorted bin 里找一下。
        5.1 
    6. 否则继续执行。如果不是在samllbins大小范围中，将就一下。用最佳匹配找到largebin下标对应的chunk,切割chunk。
    7. 否则继续执行。下标+1，找下一个bin。

### free 释放策略总结：

chunk释放策略总述：
    1. 如果传入的指针为null，什么都不做，直接返回。
    2. 否则继续执行。通过chunk头大小字段中的一个标志位判断是否是用mmap分配的chunk。如果是通过munmap_chunk函数释放，返回。
    3. 否则继续执行。拿到chunk头，做一些合理性检查,如果检查失败，报错,终止程序。
    4. 否则继续执行。如果准备释放的chunk大小有对应的tcache bin(chunk 太大就没有对应的bin),且bin还没有满。插到对应tcache bin 链表头中,返回。
    5. 否则继续执行。如果chunk大小小于等于fastbin的最大大小，将chunk插入到对应的fastbin，返回。
    6. 否则继续执行。判断是否是通过mmap分配的。即判断chunk其大小没有达到用mmap分配的阈值。 
        6.1. 如果是mmap分配的,通过munmap_chunk函数释放chunk。 返回。
    7. 否则继续执行。尝试合并物理上相邻的chunk，如果该chunk的下一个chunk是top chunk，则合并到top chunk中去。否则合并到unsorted chunk中...
        7.1. 接着执行。如果合并后的chunk的大小不超过FASTBIN_CONSOLIDATION_THRESHOLD,返回。
    8. 否则继续执行。如果有fastbin，调用malloc_consolidate函数做分配步骤4相同的操作。判断top chunk 的大小是否超过top chunk所设置的最大值。如果是执行堆修建，按页返回一些申请的空间给系统,直到top chunk的大小满足所设定的一个值。返回。

合并时，合并到unsorte chunk中的情况。

## CTF WarGame配套实践：

*  [shellphish/how2heap](https://github.com/shellphish/how2heap)

