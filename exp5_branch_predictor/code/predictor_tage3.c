/*
make predictor_tage; ./predictor_tage traces/LONG_MOBILE-2.bt9.trace.gz
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

// TAGE分支预测器的状态信息
#define NUM_BASE_PREDICTORS 5 // 基本预测器的数量
#define MAX_HISTORY_LENGTH 20 // 最大全局历史长度
#define PHT_CTR_MAX 3         // 0 1 2 3
#define PHT_CTR_INIT 2
#define TAKEN 'T'
#define NOT_TAKEN 'N'

UINT32 ghr;                                                     // 全局历史寄存器
UINT32 *pht[NUM_BASE_PREDICTORS];                               // 模式历史表数组
UINT32 *tag[NUM_BASE_PREDICTORS];                               // 标签表数组
UINT32 numPhtEntries;                                           // 模式历史表中的项数
UINT32 historyLengths[NUM_BASE_PREDICTORS] = {2, 4, 8, 12, 20}; // 为每个基本预测器定义不同的历史长度

void PREDICTOR_init(void)
{
    ghr = 0;
    numPhtEntries = (1 << MAX_HISTORY_LENGTH);

    for (int i = 0; i < NUM_BASE_PREDICTORS; i++)
    {
        pht[i] = (UINT32 *)malloc(numPhtEntries * sizeof(UINT32));
        tag[i] = (UINT32 *)malloc(numPhtEntries * sizeof(UINT32));

        // 初始化模式历史表和标签表
        for (UINT32 ii = 0; ii < numPhtEntries; ii++)
        {
            pht[i][ii] = PHT_CTR_INIT;
            tag[i][ii] = 0;
        }
    }
}

char GetPrediction(UINT64 PC)
{
    int longestMatchIndex = -1;

    // 寻找与计算出的标签匹配的表项
    for (int i = 0; i < NUM_BASE_PREDICTORS; i++)
    {
        // ghr & ((1 << historyLengths[i]) - 1)
        // 取ghr的低historyLengths[i]位，与PC异或，得到索引
        UINT32 index = (PC ^ (ghr & ((1 << historyLengths[i]) - 1))) % numPhtEntries;
        UINT32 current_tag = (PC >> historyLengths[i]) % numPhtEntries;

        if (tag[i][index] == current_tag)
        {
            longestMatchIndex = i;
            break;
        }
    }

    UINT32 phtCounter;
    if (longestMatchIndex >= 0)
    {
        // 如果找到匹配的表项，就使用该表项的计数器
        UINT32 index = (PC ^ (ghr & ((1 << historyLengths[longestMatchIndex]) - 1))) % numPhtEntries;
        phtCounter = pht[longestMatchIndex][index];
    }
    else
    {
        phtCounter = PHT_CTR_INIT;
    }

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

    // 更新匹配的预测器
    for (int i = 0; i < NUM_BASE_PREDICTORS; i++)
    {
        UINT32 index = (PC ^ (ghr & ((1 << historyLengths[i]) - 1))) % numPhtEntries;
        UINT32 current_tag = (PC >> historyLengths[i]) % numPhtEntries;

        if (tag[i][index] == current_tag)
        {
            UINT32 phtCounter = pht[i][index];

            if (resolveDir == TAKEN)
            {
                pht[i][index] = SatIncrement(phtCounter, PHT_CTR_MAX);
            }
            else
            {
                pht[i][index] = SatDecrement(phtCounter);
            }
            break;
        }
    }

    // 更新最长历史预测器的标签
    int longestHistoryIndex = NUM_BASE_PREDICTORS - 1;
    UINT32 index = (PC ^ (ghr & ((1 << historyLengths[longestHistoryIndex]) - 1))) % numPhtEntries;
    UINT32 current_tag = (PC >> historyLengths[longestHistoryIndex]) % numPhtEntries;
    tag[longestHistoryIndex][index] = current_tag;

    // 更新全局历史寄存器
    ghr = (ghr << 1) & ((1 << MAX_HISTORY_LENGTH) - 1);

    if (resolveDir == TAKEN)
    {
        ghr = ghr | 0x1;
    }
}

void PREDICTOR_free(void)
{
    for (int i = 0; i < NUM_BASE_PREDICTORS; i++)
    {
        free(pht[i]);
        free(tag[i]);
    }
}
