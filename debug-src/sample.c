#include<string.h>
#include<stdlib.h>

int main(){
    // 初始化堆,向系统申请空间,分配top chunk结构,分配第一个chunk给tcache结构。然后从top chunk中分出一块给a。chunk a的一共大小：64。计算方法: malloc.c:1196的request2size(req) 宏。
    char *a=malloc(56);
    // 在a的后面申请一个最小chunk。chunk b一共大小：32。计算方法: request2size(req) 宏。
    char *b=malloc(0);
    
    // 释放a。将a放入tcache bin[2]中，从chunk大小到tcache bin下标的计算的csize2tidx(x)宏在malloc.c:311。
    free(a);
    // 释放b。将b放入tcache bin[0]中, 计算方法：csize2tidx(x)宏
    free(b);
    
    // 填充+对齐后大小符合tcache bin[2](tcache bin之间的大小差异恰好是对齐的倍数)的大小。从tcache bin[2]链表头(也就是放chunk a的位置)取chunk。 NOTE: c拿到的是和a同样的地址。
    char * c=malloc(54);
    // 还原堆布局,将c放回tcache bin[2]中。
    free(c);     


    // 大小

/*
    // 本可以合并b,a后再分配。但top chunk还有空间。
    char *c=malloc(88);

    // 使用large bin
    char *large=malloc(1024);
    
    // 默认 global_max_fast:默认 128 字节
    //
    // tcache 最大大小：1032字节

*/
}
