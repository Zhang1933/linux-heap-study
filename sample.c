/*
 * 演示程序为64位下的情况。
 */
#include<string.h>
#include<stdlib.h>
#include <unistd.h>

# define TCACHE_FILL_COUNT 7 // tcache bin 最大chunk数，为可调参数 

size_t tcache_max_bytes=1032; // tcache_max_bytes是tcache bins chunk中数据字段最大大小。参数定义malloc.c:1763。

#ifndef INTERNAL_SIZE_T
# define INTERNAL_SIZE_T size_t
#endif

/*  fastbins中chunk的最大大小 。初始化在malloc.c:1805 */
static INTERNAL_SIZE_T global_max_fast=128; 

#define FASTBIN_CONSOLIDATION_THRESHOLD  (65536UL)

// 当chunk大小超过DEFAULT_MMAP_THRESHOLD_MIN大小时，用mmap方法分配。
#ifndef DEFAULT_MMAP_THRESHOLD_MIN
#define DEFAULT_MMAP_THRESHOLD_MIN (128 * 1024)
#endif

// 最小chunk大小，定义在malloc.c:1183
#define MINSIZE 32

// malloc分配块的对齐,定义在malloc.c:27 
#define MALLOC_ALIGNMENT 16

int main(){
    size_t pagesize =getpagesize();
    char *pad="012345678";
    char *s[TCACHE_FILL_COUNT];
    // 申请TCACHE_FILL_COUNT 个chunk。
    for(int i=0;i<TCACHE_FILL_COUNT;i++){ // gdb 中用 until 跳出这该死的循环。
        s[i]=malloc(0);
        strcpy(s[i],pad);
    }
    
    //再申请2个chunk
    char *to_fasta=malloc(0); 
    char *to_fastb=malloc(0); 
    strcpy(to_fasta,pad);
    // 释放TCACHE_FILL_COUNT 个chunk，把tcache 对应的bin 填满。tcache为单链表结构。
    for(int i=0;i<TCACHE_FILL_COUNT;i++){
        free(s[i]);
    }
    // 此时释放，tcache bin中对应bin装满了chunk,to_fasta的chunk大小小于等于fastbin的最大大小,会放到fastbin对应的链表头部中。
    free(to_fasta);
    // 在fastbin链表头加入to_fastb。
    free(to_fastb);
    
    // 让我们分配一个数据字段占tcache_max_bytes的chunk。此时会调用malloc_consolidate函数, 将fastbin中的chunk(即to_fasta,to_fastb这两个chunk)合并到top chunk中。
    char *largebin=malloc(tcache_max_bytes+1);

    // 将largebin合并到top chunk。此时合并后的chunk size>=FASTBIN_CONSOLIDATION_THRESHOLD。会执行systrim函数减少空闲top chunk。因为malloc初始化时会向系统要33页的空间，第一次到这里时，大小肯定是会超过32页的,但没有超过33页。
    free(largebin);

    // 用mmap分配、释放。
    char *mmap=malloc(DEFAULT_MMAP_THRESHOLD_MIN+pagesize);
    free(mmap);
    
/* ---------------------unosrted bin playground------------------------------*/
 
    // 此时fastbin中没有，tcache bin[0]是满的。创建两个放入largebin中的chunk。large chunk a大小为1056
    char *largechunka=malloc(tcache_max_bytes+1);
    // 将largechunka,largechunkb隔开,使其不能合并。
    char *edgeab=malloc(MINSIZE+1);
    // b的chunk大小大于a,large chunk b 大小为1072.
    char *largechunkb=malloc(1056);
    //分配一个48chunk大小的边界。使chunk上面的与top chunk 隔开。
    char *edgetop=malloc(MINSIZE+1);

    // free完后，largechunka,largechunkb在unsorted bin中。
    free(largechunka);
    free(largechunkb);
    
    // 现在我们来申请一个largechunkc，此时会遍历unsorted bin,将largechunka,largechunkb从小到大放到large bin中。
    char *largechunkc=malloc(tcache_max_bytes*2);
    // 调试calloc情况，,置0操作   break sample.c:83
    char *largechunkd=calloc(1,8);
}
