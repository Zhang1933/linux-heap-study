# Understanding the heap by debugging it

> The best source of knowledge with regards to the implementation of the heap is itself, the source code.


通过手动构造的样例动态调试malloc，free,calloc,realloc函数进行源码分析来理解堆、堆管理。

* 仓库中所给的glibc版本：2.31。没有增删行,行可以和[glibc-2.31源代码](https://elixir.free-electrons.com/glibc/glibc-2.31/source/malloc/malloc.c)对应上。仓库中的[malloc.c源码](https://github.com/Zhang1933/linux-heap-study/blob/main/glibc-2.31/malloc/malloc.c)有许多注释&分析。

## 如何使用:

**注意：** 调试样例仅支持64位linux操作系统!

编译调试程序：

```bash
make
```

目录下会生成对应的动态链接的，带调试符号的目标文件。通过用gdb动态调试这个样例程序理解堆管理。也可以调试[shellphish/how2heap](https://github.com/shellphish/how2heap)上的样例。

## 分析源代码&调试相关问题：

* 如何用gdb调试glibc源代码:[Zhang1933-如何用gdb调试glibc源码](https://zhang1933.github.io/2022/06/07/%E5%A6%82%E4%BD%95%E7%94%A8gdb%E8%B0%83%E8%AF%95glibc%E6%BA%90%E7%A0%81.html)

* gdb的一个方便查看堆的插件: [pwndbg](https://github.com/pwndbg/pwndbg)

* 生成 compilation database 用于在看源代码时的跳转查找函数&宏定义:[rizsotto/Bear](https://github.com/rizsotto/Bear)

## Putting it all together

### malloc分配流程框架：

Glibc 2.26以上版本，tcache 默认情况下开启:[Glibc Enables A Per-Thread Cache For Malloc - Big Performance Win](https://www.phoronix.com/scan.php?page=news_item&px=glibc-malloc-thread-cache)。这里说明默认情况。

malloc 入口点：`__libc_malloc`函数。

标注了与[malloc.c](https://elixir.free-electrons.com/glibc/glibc-2.31/source/malloc/malloc.c)源码中对应的判断行,复合判断有可能在下一个if。

chunk分配步骤框架(不包括初始化)。

```cpp
if(请求的chunk大小不超过tcache bins中chunk的最大大小&&大小对应的bin中有空chunk){// 3047
    return 对应的tcache bin链表中取出的chunk;
}
if(请求的大小不超过fastbins中chunk的最大大小&&对应的fastbins中的有空chunk){ // 3577
    从大小对应的fastbin中取一个chunk;
    if(请求的chunk大小对应的tcache bin没有装满){ // 3600
        尽量用fastbin中余下chunk装满tcache bin;
    }
    return 取出的chunk;
}
if(申请的chunk的大小不超过smallbins中chunk的最大大小){ // 3635
    if(其大小对应的smallbin不为空){ // 3640
        从chunk大小对应的samllbin中取一个chunk;
        if(请求的chunk大小对应的tcache bin没有装满){ // 3656
            尽量用samllbin中余下chunk装满对应的tcache bin;
        }
        return 取出的chunk;
    }
}
else{ //3695
    计算申请的chunk在largebins中对应大小的下标;
    if(有fastchunks){ // 3695
        调用malloc_consolidate函数。此函数合并内存上上连续的fastbin中的chunk，并放到unsorted bin中;
    }
}
for(;;){ // 3725

    while(unsorted bin不为空){ //3728
        if(请求的chunk大小不超过samllbins的最大大小&&unsorted bin中只有一个chunk&&unsorted bin中唯一的chunk==last remainder&&切割下unsorted bin中唯一的chunk之后其大小不小于chunk的最小大小){ // 3756
            从unsorted中唯一的chunk切割出请求的大小; 
            last remainder更新为剩下部分的chunk地址;
            余下空间的加入unsorted bin;
            return 切下的chunk; 
        }
        从unosrted bin中取出头部chunk;
        if(unsorted bin 中的第一个chunk恰好等于所申请的chunk大小){ // 3792
             if(请求的chunk大小不超过tcachebins的最大大小&&对应的tcache bin没有装满){ // 3800
                 将unsorted bin第一个chunk放在对应的tcachebin中;
                 标记return_cached=1;
                 continue;
             }
             else{ // 3807
                 return unsorted bin 中的第一个chunk;
             }
        }
        if(unsorted bin 中的第一个chunk的大小不超过smallbins的最大大小){ // 3821 
            将unsorted bin 中的第一个chunk放入其大小对应的samllbin;
        }
        else{ // 3827
            将unsorted bin 中的第一个chunk从小到大放入其大小对应的largebin; 
        }
        尝试次数+1;
        if(达到最大的尝试次数){ // 3900
            break;
        }
    }
    if(return_cached标记为1){ // 3906
        return 其对应大小的tcachebin中摘的一个;
    }
    if(超过samllbins的最大大小){ // 3917
        if(对应的largebin非空&&bin中最大的chunk大于请求的大小){ // 3922
            搜索链表最佳匹配找到一个chunk;
            从largebin链表中取出那个chunk,尝试切割;
            if(测试切割后余下的部分小于最小){ // 3942
                那么不分割;
            }
            else{ // 3949
                切割找到的chunk; 
                剩下的部分装到unsorted bin中;
            }
            return 找到的chunk(有可能经过切割);
        }
    }
    for (;; ){ // 3996
        while(比请求的chunk大一点的bins为空){ // 3999
            if(bins都检查完了){ // 4003
                goto:use_top;
            }
        }
        if(找到的bin是一个空的){ // 4021
            将表示bin为空的位置为0;
            继续循环，再考虑下一个bin;
        }
        else{ // 4031
            将找到的bin中的第一个块从bin中取出去; 
            if(切割后其大小小于最小chunk){ // 4044
                不切割;
            }
            else{  // 4052
                切割,将余下的chunk放到unsorted bin中;
            }
            返回 找到的chunk(可能进行切割);
        }
    }
use_top: // 4087
    if(切下所请求chunk大小的topchunk后,topchunk的大小不小于最小chunk大小){ // 4109
        从topchunk中切下需要申请的chunk;
        return 切下的chunk;
    }
    else if(fastbin中来了新的chunk){ //4126
        调用malloc_consolidate函数;        
        恢复对应bin的下标;
    }
    else{ // 4139
        调用sysmalloc函数，此函数按页向系统要空间，返回用户所申请的chunk地址;
        return sysmalloc返回的地址;
    }
}

```


### free 释放流程框架：

free入口点：`__libc_free`函数。

chunk释放流程框架：

```cpp
if(所传入的指针为null){ // 3099
    返回;
}
if(chunk是用mmap分配){// 3104
    用munmap_chunk函数释放chunk;
    return;
}
if(准备释放的chunk大小有对应的tcache bin&&其bin还没有满){ // 4184
    将chunk插到与大小对应tcache bin 链表头中;
    return;
}
if(chunk大小不超过fastbins中chunk的最大大小){ // 4220
    将chunk插入到与大小对应的fastbin链表头中;
}
else if(chunk不是通过mmap方式分配的){ // 4295
    if(该chunk头中前一个chunk在使用的标志位为0){ // 4327
        合并内存中上一个chunk;
        将上一个chunk从bin上摘下来(chunk中有前驱后继信息);
    }
    if(内存中下一个chunk不是top chunk){ // 4336
        if(内存中下下一个chunk的前一个chunk在使用标志为0){// 4341
            将下一个chunk从bin中取出来;
            合并内存中下一个chunk;
        }
         // 4344
        将物理上下一个chunk的PREV_INUSE标志为置为0;
        将申请释放（或者合并后）的chunk放到unsorted bin中;
        配置该chunk的头部与尾部;
    }
    else{ // 4378
        将chunk合并到top chunk中去;
    }
    if(请求释放的chunk的一共的大小(加上合并的)超过fastbin合并阈值){ // 4398
        if(有fastchunks){ // 4399
            调用malloc_consolidate函数,此函数将fastbin中的内存相邻的chunk合并后插入到unsorted bin链表中;
        }
        if(topchunk大小超过一个阈值){ // 4404
            调用systrim函数,返还一些空间给系统; 
        }
    }
}
else{ // 4425
   用munmap_chunk函数释放chunk;
}
return;
```

### calloc流程

入口点：`__libc_calloc`

```cpp
算出用户需要的空间; // 3376
调用_int_malloc函数分配空间(跟malloc.c:3577行后的逻辑一样。不会优先用tcachebin); // 3428
//3464
if(chunk是通过mmap方式分配的){ // 3453
    return; // 不需要置0,直接返回
}
if(如果top chunk扩展了){ // 3464 
    设定需要置0的内存空间为原来top chunk的大小;
}
将需要置0的内存空间置0;
return _init_malloc返回chunk的数据部分;
```

### realloc 流程

当传入的指针是空时,入口点: `__libc_malloc`。 分配流程和 malloc(size) 一样。

否则入口点 `__libc_realloc`。编译出的程序直接 call 的地址不一样。

为了方便叙述，假设是这么调用的: `realloc(void *oldmem, size_t bytes)`。

oldmem指向旧chunk的数据部分，bytes是你想要新chunk中数据部分的大小。

```cpp
if(如果bytes为0){ //  3143
    调用 __libc_free 函数,直接释放 oldmem 所在的chunk;
}
if(oldmem为0){// 3150
    调用  __libc_malloc 函数,按照 malloc 的逻辑分配一个新chunk;
}
if(chunk 是mmap 分配的){ // 3183
    TODO: 流程分析
}
if(是单线程){ // 3224
    返回调用的_int_realloc函数的执行结果。
}
....
```

#### _int_realloc 函数流程分析

```cpp
if(旧的chunk大小大于等于所需的新的chunk大小){ // 4566
     新chunk=旧chunk; // 后面会切分多余,新chunk也是下面4631行处说的总共拿到的chunk。
}
else{ // 4573
    if(内存中下一个chunk是top chunk且top chunk分配之后大于最小chunk大小){ // 4576
        top chunk中切下所需扩展的大小给旧chunk，返回旧chunk;
    } 
    else if(如果内存中旧chunk下一个chunk不是top chunk且下一个chunk使用位为0且旧chunk大小+下一个chunk大小>=需要的大小){ // 4588
        newp=oldp; // oldp指向旧chunk,newp指向新chunk
        链表中释放内存中下一个chunk;
    }
    else{ // 4598
        调用_int_malloc函数分配所需大小的chunk;//newmem=_int_malloc(av,nb-MALLOC_ALIGN_MASK); ,nb 是所需chunk的大小,av是对应的arena
        if(新分配的chunk是旧chunk内存中下一个chunk){ // 4610 ,chunk在 fastbin，tacachebin中的情况
            把相邻的两个chunk合并在一起,不需要复制旧chunk中的内容;
        }
        else{ // 4615
            将旧chunk的内容复制到新chunk中; 
            释放旧chunk;
            return 所分配的新chunk;
        }
    }
}

if(总共拿到的chunk大小减去所需要的chunk大小小于最小chunk大小){ // 4631 ,有多余的情况。
    就按总共拿到的chunk大小算;
}
else{ // 4636 
    从总共拿到的chunk大小切出多余的部分;
    将切出的chunk释放掉; // 即调用_int_free(av,remainder,1); av指向arena,remainder指向切出的多余chunk,1表示have_lock为1。
}
return 新chunk所对应的数据区地址;
```

## 一些可能有用的资料：

除了上述所引用的链接外：

* [PART 1: UNDERSTANDING THE GLIBC HEAP IMPLEMENTATION](https://azeria-labs.com/heap-exploitation-part-2-glibc-heap-free-bins/)
* [PART 2: UNDERSTANDING THE GLIBC HEAP IMPLEMENTATION](https://azeria-labs.com/heap-exploitation-part-2-glibc-heap-free-bins/)
* [Overview of Malloc](https://sourceware.org/glibc/wiki/MallocInternals#Thread_Local_Cache_.28tcache.29)
* [Bins and Chunks](https://heap-exploitation.dhavalkapil.com/diving_into_glibc_heap/bins_chunks)
* [Glibc的堆实现](https://dere.press/2020/06/21/glibc-heap/#unlink_chunk)
