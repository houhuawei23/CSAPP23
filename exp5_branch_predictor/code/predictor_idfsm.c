/*
为每个分支指令，分配完全独立的状态机，并测试 predictor_idfsm
思路：
	完全独立？意味着不存在冲突，应该设计hash函数以实现？
*/
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

// The state is defined for 1-bit predictor
UINT32 *pht;          // pattern history table 模式历史表
UINT32 numPhtEntries; // entries in pht PHT中的项数

#define PHT_CTR_MAX 1 // 0 or 1
#define PHT_CTR_INIT 0

#define HIST_LEN 15
#define PHT_LEN 8192 // 模式历史表的长度，质数，降低冲突
#define TAKEN 'T'
#define NOT_TAKEN 'N'

void PREDICTOR_init(void)
{
    numPhtEntries = PHT_LEN; // 实际项数可以根据需求调整

    pht = (UINT32 *)malloc(numPhtEntries * sizeof(UINT32));

    // Initialize pattern history table
    for (UINT32 ii = 0; ii < numPhtEntries; ii++)
    {
        pht[ii] = PHT_CTR_INIT;
    }
}

char GetPrediction(UINT64 PC)
{
    UINT32 phtIndex = PC % numPhtEntries;
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

void UpdatePredictor(UINT64 PC, OpType opType, char resolveDir, char predDir, UINT64 branchTarget)
{
    opType = opType;
    predDir = predDir;
    branchTarget = branchTarget;

    UINT32 phtIndex = PC % numPhtEntries;
    UINT32 phtCounter = pht[phtIndex];

    if (resolveDir == TAKEN)
    {
        pht[phtIndex] = SatIncrement(phtCounter, PHT_CTR_MAX); // 如果结果为跳转，则对应的饱和计数器+1
    }
    else
    {
        pht[phtIndex] = SatDecrement(phtCounter); // 如果结果为不跳转，则对应的饱和计数器-1
    }
}

void PREDICTOR_free(void)
{
    free(pht);
}
