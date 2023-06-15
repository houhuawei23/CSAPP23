/*
 * mm-naive.c - 参考实现，是一个最快的、最低效率的malloc.
 *
 * 在这个参考实现中，分配一个块，仅仅是增加brk指针，
 * 块内部全部是载荷数据，块内没有header或者footer等
 * 管理用的数据信息。分配出去的块，永远不释放或者回收。
 * 重分配函数（realloc）的实现，是直接通过mm_malloc和mm_free实现的
 *
 * 亲们请注意：你需要把此段注释，替换成你的算法设计思想。用描述性
 * 的话来说清楚。
 * 请将此文件，重新命名为mm_201309060024.c（就是mm_你的学号.c）
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * 亲们请注意：开始之前，请把下面的信息修改为你的个人信息
 ********************************************************/
team_t team = {
	/* 团队名字 */
	"hhw team",
	/* 团队老大的名字 */
	"hhw",
	/* 团队老大的email地址 */
	"2589622350@qq.com",
	/* 团队其他成员的名字 (如果没有，就空着) */
	"",
	/* 团队其他成员的email地址 (如果没有，就空着) */
	""};

/* 单字 (4) 还是双字 (8) 边界对齐 */
#define ALIGNMENT 8

/* 舍入到最近的ALIGNMENT边界上 */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

static void print_block(int request_id, int payload);

/*
 * mm_init - 初始化malloc系统，此函数，在整个运行期间，只被调用1次，用于建立初始化环境
 */
int mm_init(void)
{
	return 0;
}

/*
 * mm_malloc - 通过增加brk指针，来分配一块内存。
 *     总是分配一块内存，它的大小是ALIGNMENT的整数倍（对齐）。
 */
void *mm_malloc(size_t size)
{
	// 将大小调整到ALIGNMENT的整数倍（对齐）
	int newsize = ALIGN(size + SIZE_T_SIZE);

	// 修改brk指针
	void *p = mem_sbrk(newsize);
	if (p == (void *)-1)
	{
		printf("[%s]mm_alloc失败：size=%zu newsize=%d\n", __func__, size, newsize);
		return NULL;
	}
	else
	{
		*(size_t *)p = size;
		return (void *)((char *)p + SIZE_T_SIZE);
	}
}

/*
 * mm_free - 释放一块内存。其实没干啥事....
 */
void mm_free(void *ptr)
{
}

/*
 * mm_realloc - 重新扩展一块已分配的内存。仅仅是使用mm_malloc和mm_free来实现，很蠢
 */
void *mm_realloc(void *ptr, size_t size)
{
	void *oldptr = ptr;
	void *newptr;
	size_t copySize;

	// 首先分配一块大一点的内存
	newptr = mm_malloc(size);
	if (newptr == NULL)
		return NULL;
	copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
	if (size < copySize)
		copySize = size;

	// 把老内存里面的内容，复制到新内存里面
	memcpy(newptr, oldptr, copySize);

	// 释放掉老内存
	mm_free(oldptr);

	// 返回新内存的指针
	return newptr;
}
/*
 * mm_heapcheck - 目前暂不支持堆检查，可以不用修改
 */
void mm_heapcheck(void)
{
}

/*
 * 输出一块数据 - 用于heapcheck，然而在此并没有什么用，可以不用修改
 */

static void print_block(int request_id, int payload)
{
	printf("\n[%s]$BLOCK %d %d\n", __func__, request_id, payload);
}
