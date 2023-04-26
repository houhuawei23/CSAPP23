///////////////////////////////////////////////////////////////////////
////  Copyright 2020 by mars.                                        //
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>

#include "common.h"

// 饱和计数器：加1
static inline UINT32 SatIncrement(UINT32 x, UINT32 max)
{
	if (x < max)
		return x + 1;
	return x;
}

// 饱和计数器：减1
static inline UINT32 SatDecrement(UINT32 x)
{
	if (x > 0)
		return x - 1;
	return x;
}

// The state is defined for Gshare, change for your design
// Gshare分支预测器的状态信息，你需要根据自己的设计进行调整
UINT32 ghr;			  // global history register  全局历史寄存器
UINT32 *pht;		  // pattern history table    模式历史表
UINT32 historyLength; // history length           历史长度
UINT32 numPhtEntries; // entries in pht           PHT中的项数

#define PHT_CTR_MAX 3 // 0 1 2 3
#define PHT_CTR_INIT 2

#define HIST_LEN 17 // 全局历史寄存器长度，取17位

#define TAKEN 'T'
#define NOT_TAKEN 'N'

void PREDICTOR_init(void)
{

	historyLength = HIST_LEN;
	ghr = 0;
	numPhtEntries = (1 << HIST_LEN); // 模式历史表，就有2^17项

	pht = (UINT32 *)malloc(numPhtEntries * sizeof(UINT32));

	// 将模式历史表，全部初始化为PHT_CTR_INIT
	for (UINT32 ii = 0; ii < numPhtEntries; ii++)
	{
		pht[ii] = PHT_CTR_INIT;
	}
}

// Gshare分支预测器
// 将PC的低17位，与全局历史寄存器进行异或（加密），去索引PHT，得到对应的饱和状态
// 如果该状态的值超过一半，则预测跳转
// 如果该状态的值低于一半，则预测不跳转
char GetPrediction(UINT64 PC)
{

	UINT32 phtIndex = (PC + ghr) % (numPhtEntries);
	UINT32 phtCounter = pht[phtIndex];

	if (phtCounter > (PHT_CTR_MAX / 2))
	{
		return TAKEN;
	}
	else
	{
		return NOT_TAKEN;
	}
}

// Gshare分支预测器
// 根据分支指令实际执行结果，来更新对应的饱和计数器
// 如果结果为跳转，则对应的饱和计数器+1
// 如果结果为不跳转，则对应的饱和计数器-1
// 更新全局历史寄存器：
// 结果为跳转，将1移位到GHR的最低位
// 结果为不跳转，将0移位到GHR的最低位
void UpdatePredictor(UINT64 PC, OpType opType, char resolveDir, char predDir, UINT64 branchTarget)
{

	opType = opType;
	predDir = predDir;
	branchTarget = branchTarget;

	UINT32 phtIndex = (PC ^ ghr) % (numPhtEntries);
	UINT32 phtCounter = pht[phtIndex];
	//	printf("PC=%016llx resolveDir=%c predDir=%c branchTarget=%016llx\n", PC, resolveDir, predDir, branchTarget);

	if (resolveDir == TAKEN)
	{
		pht[phtIndex] = SatIncrement(phtCounter, PHT_CTR_MAX); // 如果结果为跳转，则对应的饱和计数器+1
	}
	else
	{
		pht[phtIndex] = SatDecrement(phtCounter); // 如果结果为不跳转，则对应的饱和计数器-1
	}

	// update the GHR
	ghr = (ghr << 1);

	if (resolveDir == TAKEN)
	{
		ghr = ghr | 0x1;
	}
}

void PREDICTOR_free(void)
{
	free(pht);
}