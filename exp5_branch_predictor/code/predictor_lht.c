/*
局部历史信息
make predictor_lht;./predictor_lht traces/LONG_MOBILE-2.bt9.trace.gz
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

UINT32 *lht;          // local history table 局部历史表
UINT32 *pht;          // pattern history table 模式历史表
UINT32 numLhtEntries; // entries in lht LHT中的项数
UINT32 numPhtEntries; // entries in pht PHT中的项数

#define PHT_CTR_MAX 3 // 0, 1, 2, 3
#define PHT_CTR_INIT 2
#define LHT_LEN 10 // 局部历史寄存器长度，取10位
#define PHT_LEN 15 // 模式历史表长度，取15位

// #define LHT_SIZE 1024   // 局部历史表大小
// #define PHT_SIZE 1024   // 模式历史表大小
#define LOCAL_HIST_LEN 3 // 局部历史寄存器长度，取3位

#define TAKEN 'T'
#define NOT_TAKEN 'N'

void PREDICTOR_init(void)
{
    numLhtEntries = (1 << LHT_LEN);

    numPhtEntries = (1 << PHT_LEN);

    lht = (UINT32 *)malloc(numLhtEntries * sizeof(UINT32));
    pht = (UINT32 *)malloc(numPhtEntries * sizeof(UINT32));

    // Initialize local history table and pattern history table
    for (UINT32 ii = 0; ii < numLhtEntries; ii++)
    {
        lht[ii] = 0;
    }

    for (UINT32 ii = 0; ii < numPhtEntries; ii++)
    {
        pht[ii] = PHT_CTR_INIT;
    }
}

char GetPrediction(UINT64 PC)
{
    UINT32 lhtIndex = (PC<<2) % numLhtEntries; // PC[12:2] 中间10位用于索引lht
    UINT32 localHistory = lht[lhtIndex];

    UINT32 phtIndex = lhtIndex + localHistory;
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

    UINT32 lhtIndex = (PC<<2) % numLhtEntries; // PC[12:2] 中间10位用于索引lht
    UINT32 localHistory = lht[lhtIndex];

    UINT32 phtIndex = lhtIndex + localHistory;
    UINT32 phtCounter = pht[phtIndex];

    // Update pattern history table based on the actual outcome
    if (resolveDir == TAKEN)
    {
        pht[phtIndex] = SatIncrement(phtCounter, PHT_CTR_MAX);
    }
    else
    {
        pht[phtIndex] = SatDecrement(phtCounter);
    }

    // Update local history table
    localHistory = (localHistory << 1) & ((1 << LOCAL_HIST_LEN) - 1);

    if (resolveDir == TAKEN)
    {
        localHistory |= 1;
    }

    lht[lhtIndex] = localHistory;
}
void PREDICTOR_free(void)
{
    free(lht);
	free(pht);
}