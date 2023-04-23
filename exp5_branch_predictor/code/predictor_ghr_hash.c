#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
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
// FNV-1a hash
#define TABLE_SIZE 8192
#define EMPTY UINT64_MAX
#define FNV_OFFSET_BASIS_32 2166136261U
#define FNV_PRIME_32 16777619U

#define TAKEN 'T'
#define NOT_TAKEN 'N'
typedef struct {
    UINT64 key;
    UINT64 value;
} Entry;

Entry hash_table[TABLE_SIZE];

// FNV-1a Hash Function
UINT32 fnv1a_hash(UINT64 key, UINT32 M) {
    const char *key_ptr = (const char *)&key;
    UINT32 hash = FNV_OFFSET_BASIS_32;
    for (int i = 0; i < sizeof(UINT64); i++) {
        hash ^= *key_ptr;
        hash *= FNV_PRIME_32;
        key_ptr++;
    }
    return hash % M;
}

void init_table() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        hash_table[i].key = EMPTY;
    }
}

bool insert(UINT64 key, UINT64 value) {
    UINT32 index = fnv1a_hash(key, TABLE_SIZE);
    UINT32 initial_index = index;
    do {
        if (hash_table[index].key == EMPTY || hash_table[index].key == key) {
            hash_table[index].key = key;
            hash_table[index].value = value;
            return true;
        }
        index = (index + 1) % TABLE_SIZE;
    } while (index != initial_index);
    return false;  // Hash table is full
}

UINT64 search(UINT64 key) {
    UINT32 index = fnv1a_hash(key, TABLE_SIZE);
    UINT32 initial_index = index;
    do {
        if (hash_table[index].key == key) {
            return hash_table[index].value;
        }
        if (hash_table[index].key == EMPTY) {
            return EMPTY;
        }
        index = (index + 1) % TABLE_SIZE;
    } while (index != initial_index);
    return EMPTY;  // Key not found
}
void PREDICTOR_init(void)
{
    init_table();
    
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
