/* 
 *  此程序演示:
 *  malloc分配策略第1步。
 *  free 释放策略第3步。
 *  
 */
#include<string.h>
#include<stdlib.h>

int main(){
    /* 初始化堆,向系统申请空间,分配top chunk结构,把第一个chunk分配给tcache_perthread_struct结构分配。
     * 然后从top chunk中分出一块给a。chunk a的一共大小：64。
     * 计算chunk大小的宏: request2size(req)。
     */

    char *a=malloc(56);
    // 在a的后面申请一个最小chunk。chunk b一共大小：32。计算方法: request2size(req) 宏。
    char *b=malloc(0);
    
    /* 释放a。将a放入tcache bin[2]中。 
     * 一共有64个tcache bins ,间隔为对齐的大小(16)。可调参数。
     * 默认最大chunk为1032,最小chunk为32。
     * 从chunk大小到tcache bin下标的计算的宏：csize2tidx(x)。
     */
    free(a);
    /* 释放b。将b放入tcache bin[0]中。
     * tcache bin 下标计算宏：csize2tidx(x)。
     */
    free(b);

    /* 填充+对齐后大小完全符合tcache bin[2](tcache bin之间的大小差异恰好是对齐的倍数)的大小。
     * 这次申请时直接从tcache bin[2]链表头(也就是放chunk a的位置)取chunk。 
     * NOTE: c拿到的是和a同样的地址。
     */
    char * c=malloc(54);
}
