///////////////////////////////////////////////////////////////////////
////  Copyright 2022 by mars.                                        //
///////////////////////////////////////////////////////////////////////

#ifndef __TRACE_COMMON_H__
#define __TRACE_COMMON_H__

typedef unsigned char      UINT8;
typedef unsigned short     UINT16;
typedef unsigned int       UINT32;
typedef int                INT32;
typedef unsigned long long UINT64;

void InitDataCache(void);
UINT8 AccessDataCache(UINT64 Address, UINT8 Operation, UINT8 DataSize, UINT64 StoreValue, UINT64* LoadResult);

void InitInstCache(void);
UINT8 AccessInstCache(UINT64 Address, UINT8 Operation, UINT8 InstSize, UINT64* InstResult);

UINT64 ReadMemory(UINT64 Address);
void WriteMemory(UINT64 Address, UINT64 WriteData);
#endif