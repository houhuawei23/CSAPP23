///////////////////////////////////////////////////////////////////////
////  Copyright 2022 by mars.                                        //
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>

#include "common.h"

#define DEBUG 0

#define GET_POWER_OF_2(X) (X == 0x00 ? 0 : X == 0x01	  ? 0  \
									   : X == 0x02		  ? 1  \
									   : X == 0x04		  ? 2  \
									   : X == 0x08		  ? 3  \
									   : X == 0x10		  ? 4  \
									   : X == 0x20		  ? 5  \
									   : X == 0x40		  ? 6  \
									   : X == 0x80		  ? 7  \
									   : X == 0x100		  ? 8  \
									   : X == 0x200		  ? 9  \
									   : X == 0x400		  ? 10 \
									   : X == 0x800		  ? 11 \
									   : X == 0x1000	  ? 12 \
									   : X == 0x2000	  ? 13 \
									   : X == 0x4000	  ? 14 \
									   : X == 0x8000	  ? 15 \
									   : X == 0x10000	  ? 16 \
									   : X == 0x20000	  ? 17 \
									   : X == 0x40000	  ? 18 \
									   : X == 0x80000	  ? 19 \
									   : X == 0x100000	  ? 20 \
									   : X == 0x200000	  ? 21 \
									   : X == 0x400000	  ? 22 \
									   : X == 0x800000	  ? 23 \
									   : X == 0x1000000	  ? 24 \
									   : X == 0x2000000	  ? 25 \
									   : X == 0x4000000	  ? 26 \
									   : X == 0x8000000	  ? 27 \
									   : X == 0x10000000  ? 28 \
									   : X == 0x20000000  ? 29 \
									   : X == 0x40000000  ? 30 \
									   : X == 0x80000000  ? 31 \
									   : X == 0x100000000 ? 32 \
														  : 0)

/*
	直接映射Data Cache，16KB大小
*/
#define DCACHE_SIZE 16384													// 16 KB
#define DCACHE_DATA_PER_LINE 16												// 每行的字节数
#define DCACHE_DATA_PER_LINE_ADDR_BITS GET_POWER_OF_2(DCACHE_DATA_PER_LINE) // 必须与上面设置一致，即64字节，需要6位地址 (b)
#define DCACHE_LINE (DCACHE_SIZE / DCACHE_DATA_PER_LINE)					// Cache的行数
#define DCACHE_LINE_ADDR_BITS GET_POWER_OF_2(DCACHE_LINE)					// 必须与上面设置一致，即256行，需要8位地址  (s)
/*
	直接映射Instruction Cache，16KB大小
*/
#define ICACHE_SIZE 16384		// 16 KB
#define ICACHE_DATA_PER_LINE 16 // 每行的字节数
#define ICACHE_DATA_PER_LINE_ADDR_BITS GET_POWER_OF_2(ICACHE_DATA_PER_LINE)
#define ICACHE_LINE (ICACHE_SIZE / ICACHE_DATA_PER_LINE) // Cache的行数
#define ICACHE_LINE_ADDR_BITS GET_POWER_OF_2(ICACHE_LINE)

/*
	统一L2 Cache，1MB
*/
#define L2CACHE_SIZE 1048576		// 1 MB
#define L2CACHE_DATA_PER_LINE 16 // 每行的字节数
#define L2CACHE_DATA_PER_LINE_ADDR_BITS GET_POWER_OF_2(L2CACHE_DATA_PER_LINE)
#define L2CACHE_LINE (L2CACHE_SIZE / L2CACHE_DATA_PER_LINE) // Cache的行数
#define L2CACHE_LINE_ADDR_BITS GET_POWER_OF_2(L2CACHE_LINE)

UINT8 AccessL2Cache(UINT64 Address, UINT8 Operation, UINT8 DataSize, UINT64 StoreValue, UINT64* LoadResult);

struct DCACHE_LineStruct
{
	UINT8 Valid;
	UINT64 Tag;
	UINT8 Data[DCACHE_DATA_PER_LINE];
} DCache[DCACHE_LINE], ICache[ICACHE_LINE], L2Cache[L2CACHE_LINE];

/*
	DCache初始化代码，一般需要把DCache的有效位Valid设置为0
	模拟器启动时，会调用此InitDataCache函数
*/
void InitDataCache()
{
	UINT32 i;
	printf("[%s] +-----------------------------------+\n", __func__);
	printf("[%s] |   hhw 的Data Cache初始化ing.... |\n", __func__);
	printf("[%s] +-----------------------------------+\n", __func__);
	for (i = 0; i < DCACHE_LINE; i++)
		DCache[i].Valid = 0;
}

/*
	从Memory中读入一行数据到Data Cache中
*/
void LoadDataCacheLineFromMemory(UINT64 Address, UINT32 CacheLineAddress)
{
	// 一次性从Memory中将DCACHE_DATA_PER_LINE数据读入某个Data Cache行
	// 提供了一个函数，一次可以读入8个字节
	UINT32 i;
	UINT64 ReadData;
	UINT64 AlignAddress;
	UINT64* pp;

	AlignAddress = Address & ~(DCACHE_DATA_PER_LINE - 1); // 地址必须对齐到DCACHE_DATA_PER_LINE (64)字节边界
	pp = (UINT64*)DCache[CacheLineAddress].Data;
	for (i = 0; i < DCACHE_DATA_PER_LINE / 8; i++)
	{
		ReadData = ReadMemory(AlignAddress + 8LL * i);
		if (DEBUG)
			printf("[%s] Address=%016llX ReadData=%016llX\n", __func__, AlignAddress + 8LL * i, ReadData);
		pp[i] = ReadData;
	}
}

/*
	将Data Cache中的一行数据，写入存储器
*/
void StoreDataCacheLineToMemory(UINT64 Address, UINT32 CacheLineAddress)
{
	// 一次性将DCACHE_DATA_PER_LINE数据从某个Data Cache行写入Memory中
	// 提供了一个函数，一次可以写入8个字节
	UINT32 i;
	UINT64 WriteData;
	UINT64 AlignAddress;
	UINT64* pp;

	AlignAddress = Address & ~(DCACHE_DATA_PER_LINE - 1); // 地址必须对齐到DCACHE_DATA_PER_LINE (64)字节边界
	pp = (UINT64*)DCache[CacheLineAddress].Data;
	WriteData = 0;
	for (i = 0; i < DCACHE_DATA_PER_LINE / 8; i++)
	{
		WriteData = pp[i];
		WriteMemory(AlignAddress + 8LL * i, WriteData);
		if (DEBUG)
			printf("[%s] Address=%016llX ReadData=%016llX\n", __func__, AlignAddress + 8LL * i, WriteData);
	}
}

// /*
// 	从L2中读取数据到Data Cache中
// */
// void LoadDataCacheLineFromL2(UINT64 Address, UINT32 CacheLineAddress)
// {
// 	// 一次性从L2中将DCACHE_DATA_PER_LINE数据读入某个Data Cache行
// 	// 提供了一个函数，一次可以读入8个字节
// 	UINT32 i;
// 	UINT64 ReadData;
// 	UINT64 AlignAddress;
// 	UINT64 *pp;

// 	AlignAddress = Address & ~(DCACHE_DATA_PER_LINE - 1); // 地址必须对齐到DCACHE_DATA_PER_LINE (64)字节边界
// 	pp = (UINT64 *)DCache[CacheLineAddress].Data;
// 	for (i = 0; i < DCACHE_DATA_PER_LINE / 8; i++)
// 	{
// 		ReadData = ReadL2(AlignAddress + 8LL * i);
// 		if (DEBUG)
// 			printf("[%s] Address=%016llX ReadData=%016llX\n", __func__, AlignAddress + 8LL * i, ReadData);
// 		pp[i] = ReadData;
// 	}
// }

UINT64 ReadDataCache(UINT32 LineAddress, UINT8 DataSize, UINT8 BlockOffset)
{
	UINT64 ReadValue = 0;
	switch (DataSize)
	{
	case 1:
		ReadValue = DCache[LineAddress].Data[BlockOffset];
		break;
	case 2:
		BlockOffset = BlockOffset & 0xFE;
		ReadValue = DCache[LineAddress].Data[BlockOffset + 1];
		ReadValue = ReadValue << 8;
		ReadValue |= DCache[LineAddress].Data[BlockOffset + 0];
		break;
	case 4:
		BlockOffset = BlockOffset & 0xFC;
		ReadValue = DCache[LineAddress].Data[BlockOffset + 3];
		ReadValue = ReadValue << 8;
		ReadValue |= DCache[LineAddress].Data[BlockOffset + 2];
		ReadValue = ReadValue << 8;
		ReadValue |= DCache[LineAddress].Data[BlockOffset + 1];
		ReadValue = ReadValue << 8;
		ReadValue |= DCache[LineAddress].Data[BlockOffset + 0];
		break;
	case 8:
		BlockOffset = BlockOffset & 0xF8; // 需对齐到8字节边界
		ReadValue = DCache[LineAddress].Data[BlockOffset + 7];
		ReadValue = ReadValue << 8;
		ReadValue |= DCache[LineAddress].Data[BlockOffset + 6];
		ReadValue = ReadValue << 8;
		ReadValue |= DCache[LineAddress].Data[BlockOffset + 5];
		ReadValue = ReadValue << 8;
		ReadValue |= DCache[LineAddress].Data[BlockOffset + 4];
		ReadValue = ReadValue << 8;
		ReadValue |= DCache[LineAddress].Data[BlockOffset + 3];
		ReadValue = ReadValue << 8;
		ReadValue |= DCache[LineAddress].Data[BlockOffset + 2];
		ReadValue = ReadValue << 8;
		ReadValue |= DCache[LineAddress].Data[BlockOffset + 1];
		ReadValue = ReadValue << 8;
		ReadValue |= DCache[LineAddress].Data[BlockOffset + 0];
		break;
	}
	return ReadValue;
}

void WriteDataCache(UINT32 LineAddress, UINT8 DataSize, UINT8 BlockOffset, UINT64 StoreValue)
{
	switch (DataSize)
	{
	case 1: // 1个字节
		DCache[LineAddress].Data[BlockOffset + 0] = StoreValue & 0xFF;
		break;
	case 2:								  // 2个字节
		BlockOffset = BlockOffset & 0xFE; // 需对齐到2字节边界
		DCache[LineAddress].Data[BlockOffset + 0] = StoreValue & 0xFF;
		StoreValue = StoreValue >> 8;
		DCache[LineAddress].Data[BlockOffset + 1] = StoreValue & 0xFF;
		break;
	case 4:								  // 4个字节
		BlockOffset = BlockOffset & 0xFC; // 需对齐到4字节边界
		DCache[LineAddress].Data[BlockOffset + 0] = StoreValue & 0xFF;
		StoreValue = StoreValue >> 8;
		DCache[LineAddress].Data[BlockOffset + 1] = StoreValue & 0xFF;
		StoreValue = StoreValue >> 8;
		DCache[LineAddress].Data[BlockOffset + 2] = StoreValue & 0xFF;
		StoreValue = StoreValue >> 8;
		DCache[LineAddress].Data[BlockOffset + 3] = StoreValue & 0xFF;
		break;
	case 8:								  // 8个字节
		BlockOffset = BlockOffset & 0xF8; // 需对齐到8字节边界
		DCache[LineAddress].Data[BlockOffset + 0] = StoreValue & 0xFF;
		StoreValue = StoreValue >> 8;
		DCache[LineAddress].Data[BlockOffset + 1] = StoreValue & 0xFF;
		StoreValue = StoreValue >> 8;
		DCache[LineAddress].Data[BlockOffset + 2] = StoreValue & 0xFF;
		StoreValue = StoreValue >> 8;
		DCache[LineAddress].Data[BlockOffset + 3] = StoreValue & 0xFF;
		StoreValue = StoreValue >> 8;
		DCache[LineAddress].Data[BlockOffset + 4] = StoreValue & 0xFF;
		StoreValue = StoreValue >> 8;
		DCache[LineAddress].Data[BlockOffset + 5] = StoreValue & 0xFF;
		StoreValue = StoreValue >> 8;
		DCache[LineAddress].Data[BlockOffset + 6] = StoreValue & 0xFF;
		StoreValue = StoreValue >> 8;
		DCache[LineAddress].Data[BlockOffset + 7] = StoreValue & 0xFF;
		break;
	}
}

// 如果 DCACHE_DATA_PER_LINE != L2CACHE_DATA_PER_LINE，该如何解决

void load_DCache_from_L2(UINT64 Address, UINT32 DCacheLineAddress) {
	UINT32 i;
	UINT64 ReadData;
	UINT64 L2CacheLineAddress;

	L2CacheLineAddress = (Address >> L2CACHE_DATA_PER_LINE_ADDR_BITS) % L2CACHE_LINE;
	UINT64* DCacheData_p = (UINT64*)DCache[DCacheLineAddress].Data;
	UINT64* L2CacheData_p = (UINT64*)L2Cache[L2CacheLineAddress].Data;
	for (i = 0; i < DCACHE_DATA_PER_LINE / 8; i++)
	{
		DCacheData_p[i] = L2CacheData_p[i];
	}
}

void store_DCache_to_L2(UINT64 Address, UINT32 DCacheLineAddress) {
	UINT32 i;
	UINT64 WriteData;
	UINT64 L2CacheLineAddress;

	L2CacheLineAddress = (Address >> L2CACHE_DATA_PER_LINE_ADDR_BITS) % L2CACHE_LINE;
	UINT64* DCacheData_p = (UINT64*)DCache[DCacheLineAddress].Data;
	UINT64* L2CacheData_p = (UINT64*)L2Cache[L2CacheLineAddress].Data;
	for (i = 0; i < DCACHE_DATA_PER_LINE / 8; i++)
	{
		L2CacheData_p[i] = DCacheData_p[i];
	}
}
/*
	Data Cache访问接口，系统模拟器会调用此接口，来实现对你的Data Cache访问
	Address:	访存字节地址
	Operation:	操作：读操作（'L'）、写操作（'S'）、读-修改-写操作（'M'）
	DataSize:	数据大小：1字节、2字节、4字节、8字节
	StoreValue:	当执行写操作的时候，需要写入的数据
	LoadResult:	当执行读操作的时候，从Cache读出的数据
*/
UINT8 AccessDataCache(UINT64 Address, UINT8 Operation, UINT8 DataSize, UINT64 StoreValue, UINT64* LoadResult)
{
	UINT32 CacheLineAddress;
	UINT8 BlockOffset;
	UINT64 AddressTag;
	UINT8 MissFlag = 'M';
	UINT64 ReadValue;

	*LoadResult = 0;

	/*
	 *	直接映射中，Address被切分为  AddressTag，CacheLineAddress，BlockOffset
	 */

	 // CacheLineAddress Cache的行号，在直接映射中，就是组号（每组1行）
	CacheLineAddress = (Address >> DCACHE_DATA_PER_LINE_ADDR_BITS) % DCACHE_LINE;
	BlockOffset = Address % DCACHE_DATA_PER_LINE;
	// 地址去掉DCACHE_SET、DCACHE_DATA_PER_LINE，剩下的作为Tag。警告！不能将整个地址作为Tag！！
	AddressTag = (Address >> DCACHE_DATA_PER_LINE_ADDR_BITS) >> DCACHE_LINE_ADDR_BITS;
	// printf("Address: 0x%016llX, CacheLineAddress: 0x%08X, BlockOffset: 0x%02X, AddressTag: 0x%016llX\n", Address, CacheLineAddress, BlockOffset, AddressTag);
	if (DCache[CacheLineAddress].Valid == 1 && DCache[CacheLineAddress].Tag == AddressTag)
	{
		MissFlag = 'H'; // 命中！

		if (Operation == 'L') // 读操作
		{
			ReadValue = ReadDataCache(CacheLineAddress, DataSize, BlockOffset);
			*LoadResult = ReadValue;
			if (DEBUG)
				printf("[%s] Hit Load: Address=%016llX Operation=%c DataSize=%u StoreValue=%016llX ReadValue=%016llX\n", __func__, Address, Operation, DataSize, StoreValue, ReadValue);
		}
		else if (Operation == 'S' || Operation == 'M') // 写操作（修改操作在此等价于写操作）
		{
			if (DEBUG)
				printf("[%s] Hit S/M: Address=%016llX Operation=%c DataSize=%u StoreValue=%016llX\n", __func__, Address, Operation, DataSize, StoreValue);
			WriteDataCache(CacheLineAddress, DataSize, BlockOffset, StoreValue);
		}
	}
	else // L1 DataCache不命中
	{
		if (DEBUG)
			printf("[%s] Miss: Address=%016llX Operation=%c DataSize=%u Offset=%u StoreValue=%016llX\n", __func__, Address, Operation, DataSize, BlockOffset, StoreValue);
		MissFlag = AccessL2Cache(Address, Operation, DataSize, StoreValue, LoadResult);
		// if (MissFlag == 'H')
		// { // L2 Cache 命中
		// 无论L2是否命中，L2对应Cache行已经准备好了
		if (DCache[CacheLineAddress].Valid == 1)
		{ // L1 Cache 行 被占用
			// 将L1 Cache行的数据写回到L2 Cache
			UINT64 OldAddress;
			OldAddress = ((DCache[CacheLineAddress].Tag << DCACHE_LINE_ADDR_BITS) << DCACHE_DATA_PER_LINE_ADDR_BITS) |
				((UINT64)CacheLineAddress << DCACHE_DATA_PER_LINE_ADDR_BITS);
			store_DCache_to_L2(OldAddress, CacheLineAddress);
		}
		// 将L2 Cache行的数据写入到L1 Cache
		DCache[CacheLineAddress].Valid = 1;
		DCache[CacheLineAddress].Tag = AddressTag;
		load_DCache_from_L2(Address, CacheLineAddress);

		if (Operation == 'L') {
			//
		}
		else if (Operation == 'S' || Operation == 'M') {
			WriteDataCache(CacheLineAddress, DataSize, BlockOffset, StoreValue);
		}

	}
	// printf("LoadResult: 0x%016llX\n", *LoadResult);
	return MissFlag;
}

/*
	Instruction Cache访问接口，系统模拟器会调用此接口，来实现对你的Instruction Cache访问
	Address:	访存字节地址
	Operation:	操作：读操作（'L'）
	InstSize:	指令大小：1字节、2字节、4字节、8字节
	InstResult:	当执行读操作的时候，从Cache读出的指令
*/
void InitInstCache(void)
{
	UINT32 i;
	printf("[%s] +-----------------------------------+\n", __func__);
	printf("[%s] | hhw 的 Instrction Cache 初始化ing..|\n", __func__);
	printf("[%s] +-----------------------------------+\n", __func__);
	for (i = 0; i < DCACHE_LINE; i++)
		ICache[i].Valid = 0;
}

void LoadInstCacheLineFromMemory(UINT64 Address, UINT32 CacheLineAddress)
{
	UINT32 i;
	UINT64 ReadData;
	UINT64 AlignAddress;
	UINT64* pp;

	AlignAddress = Address & ~(ICACHE_DATA_PER_LINE - 1); // 地址必须对齐到DCACHE_DATA_PER_LINE (64)字节边界
	pp = (UINT64*)ICache[CacheLineAddress].Data;
	for (i = 0; i < ICACHE_DATA_PER_LINE / 8; i++)
	{
		ReadData = ReadMemory(AlignAddress + 8LL * i);
		if (DEBUG)
			printf("[%s] Address=%016llX ReadData=%016llX\n", __func__, AlignAddress + 8LL * i, ReadData);
		pp[i] = ReadData;
	}

	return;
}

UINT64 ReadInstCache(UINT32 LineAddress, UINT8 DataSize, UINT8 BlockOffset)
{
	UINT64 ReadValue = 0;
	switch (DataSize)
	{
	case 1:
		ReadValue = ICache[LineAddress].Data[BlockOffset];
		break;
	case 2:
		BlockOffset = BlockOffset & 0xFE;
		ReadValue = ICache[LineAddress].Data[BlockOffset + 1];
		ReadValue = ReadValue << 8;
		ReadValue |= ICache[LineAddress].Data[BlockOffset + 0];
		break;
	case 4:
		BlockOffset = BlockOffset & 0xFC;
		ReadValue = ICache[LineAddress].Data[BlockOffset + 3];
		ReadValue = ReadValue << 8;
		ReadValue |= ICache[LineAddress].Data[BlockOffset + 2];
		ReadValue = ReadValue << 8;
		ReadValue |= ICache[LineAddress].Data[BlockOffset + 1];
		ReadValue = ReadValue << 8;
		ReadValue |= ICache[LineAddress].Data[BlockOffset + 0];
		break;
	case 8:
		BlockOffset = BlockOffset & 0xF8; // 需对齐到8字节边界
		ReadValue = ICache[LineAddress].Data[BlockOffset + 7];
		ReadValue = ReadValue << 8;
		ReadValue |= ICache[LineAddress].Data[BlockOffset + 6];
		ReadValue = ReadValue << 8;
		ReadValue |= ICache[LineAddress].Data[BlockOffset + 5];
		ReadValue = ReadValue << 8;
		ReadValue |= ICache[LineAddress].Data[BlockOffset + 4];
		ReadValue = ReadValue << 8;
		ReadValue |= ICache[LineAddress].Data[BlockOffset + 3];
		ReadValue = ReadValue << 8;
		ReadValue |= ICache[LineAddress].Data[BlockOffset + 2];
		ReadValue = ReadValue << 8;
		ReadValue |= ICache[LineAddress].Data[BlockOffset + 1];
		ReadValue = ReadValue << 8;
		ReadValue |= ICache[LineAddress].Data[BlockOffset + 0];
		break;
	}
	return ReadValue;
}

UINT8 AccessInstCache(UINT64 Address, UINT8 Operation, UINT8 InstSize, UINT64* InstResult)
{
	// 返回值'M' = Miss，'H'=Hit
	UINT32 CacheLineAddress;
	UINT8 BlockOffset;
	UINT64 AddressTag;
	UINT8 MissFlag = 'M';
	UINT64 ReadValue;

	*InstResult = 0;

	/*
	 *	直接映射中，Address被切分为  AddressTag，CacheLineAddress，BlockOffset
	 */
	CacheLineAddress = (Address >> DCACHE_DATA_PER_LINE_ADDR_BITS) % DCACHE_LINE;
	BlockOffset = Address % DCACHE_DATA_PER_LINE;
	AddressTag = (Address >> DCACHE_DATA_PER_LINE_ADDR_BITS) >> DCACHE_LINE_ADDR_BITS;

	if (ICache[CacheLineAddress].Valid == 1 && ICache[CacheLineAddress].Tag == AddressTag)
	{
		MissFlag = 'H'; // 命中！

		if (Operation == 'I')
		{ // 读指令
			ReadValue = ReadInstCache(CacheLineAddress, InstSize, BlockOffset);
			*InstResult = ReadValue;
		}
	}
	else
	{
		MissFlag = 'M'; // 不命中
		// 访问L2 Cache

		LoadInstCacheLineFromMemory(Address, CacheLineAddress);
		ICache[CacheLineAddress].Valid = 1;
		ICache[CacheLineAddress].Tag = AddressTag;
	}
	return MissFlag;
}

// L2 Cache
#define L2_CACHE 1
void InitL2Cache(void)
{
	UINT32 i;
	printf("[%s] +-----------------------------------+\n", __func__);
	printf("[%s] |      hhw 的 L2 Cache 初始化ing..  |\n", __func__);
	printf("[%s] +-----------------------------------+\n", __func__);
	for (i = 0; i < L2CACHE_LINE; i++)
		L2Cache[i].Valid = 0;
}

/*

*/

void LoadL2CacheLineFromMemory(UINT64 Address, UINT32 CacheLineAddress)
{
	UINT32 i;
	UINT64 ReadData;
	UINT64 AlignAddress;
	UINT64* pp;

	AlignAddress = Address & ~(L2CACHE_DATA_PER_LINE - 1); // 地址必须对齐到L2CACHE_DATA_PER_LINE (64)字节边界
	pp = (UINT64*)L2Cache[CacheLineAddress].Data;
	for (i = 0; i < L2CACHE_DATA_PER_LINE / 8; i++)
	{
		ReadData = ReadMemory(AlignAddress + 8LL * i);
		if (DEBUG)
			printf("[%s] Address=%016llX ReadData=%016llX\n", __func__, AlignAddress + 8LL * i, ReadData);
		pp[i] = ReadData;
	}

	return;
}

void StoreL2CacheLineToMemory(UINT64 Address, UINT32 CacheLineAddress)
{
	UINT32 i;
	UINT64 WriteData;
	UINT64 AlignAddress;
	UINT64* pp;

	AlignAddress = Address & ~(L2CACHE_DATA_PER_LINE - 1); // 地址必须对齐到L2CACHE_DATA_PER_LINE (64)字节边界
	pp = (UINT64*)L2Cache[CacheLineAddress].Data;
	for (i = 0; i < L2CACHE_DATA_PER_LINE / 8; i++)
	{
		WriteData = pp[i];
		if (DEBUG)
			printf("[%s] Address=%016llX WriteData=%016llX\n", __func__, AlignAddress + 8LL * i, WriteData);
		WriteMemory(AlignAddress + 8LL * i, WriteData);
	}

	return;
}

UINT64 ReadL2Cache(UINT32 LineAddress, UINT8 DataSize, UINT8 BlockOffset)
{
	UINT64 ReadValue = 0;
	switch (DataSize)
	{
	case 1:
		ReadValue = L2Cache[LineAddress].Data[BlockOffset];
		break;
	case 2:
		BlockOffset = BlockOffset & 0xFE;
		ReadValue = L2Cache[LineAddress].Data[BlockOffset + 1];
		ReadValue = ReadValue << 8;
		ReadValue |= L2Cache[LineAddress].Data[BlockOffset + 0];
		break;
	case 4:
		BlockOffset = BlockOffset & 0xFC;
		ReadValue = L2Cache[LineAddress].Data[BlockOffset + 3];
		ReadValue = ReadValue << 8;
		ReadValue |= L2Cache[LineAddress].Data[BlockOffset + 2];
		ReadValue = ReadValue << 8;
		ReadValue |= L2Cache[LineAddress].Data[BlockOffset + 1];
		ReadValue = ReadValue << 8;
		ReadValue |= L2Cache[LineAddress].Data[BlockOffset + 0];
		break;
	case 8:
		BlockOffset = BlockOffset & 0xF8; // 需对齐到8字节边界
		ReadValue = L2Cache[LineAddress].Data[BlockOffset + 7];
		ReadValue = ReadValue << 8;
		ReadValue |= L2Cache[LineAddress].Data[BlockOffset + 6];
		ReadValue = ReadValue << 8;
		ReadValue |= L2Cache[LineAddress].Data[BlockOffset + 5];
		ReadValue = ReadValue << 8;
		ReadValue |= L2Cache[LineAddress].Data[BlockOffset + 4];
		ReadValue = ReadValue << 8;
		ReadValue |= L2Cache[LineAddress].Data[BlockOffset + 3];
		ReadValue = ReadValue << 8;
		ReadValue |= L2Cache[LineAddress].Data[BlockOffset + 2];
		ReadValue = ReadValue << 8;
		ReadValue |= L2Cache[LineAddress].Data[BlockOffset + 1];
		ReadValue = ReadValue << 8;
		ReadValue |= L2Cache[LineAddress].Data[BlockOffset + 0];
		break;
	default:
		printf("[%s] Error DataSize=%d\n", __func__, DataSize);
		break;
	}
	return ReadValue;
}
// 指定地址写入指定大小的数据
void WriteL2Cache(UINT32 LineAddress, UINT8 DataSize, UINT8 BlockOffset, UINT64 WriteValue)
{
	switch (DataSize)
	{
	case 1:
		L2Cache[LineAddress].Data[BlockOffset] = WriteValue;
		break;
	case 2:
		BlockOffset = BlockOffset & 0xFE;
		L2Cache[LineAddress].Data[BlockOffset + 1] = WriteValue >> 8;
		L2Cache[LineAddress].Data[BlockOffset + 0] = WriteValue;
		break;
	case 4:
		BlockOffset = BlockOffset & 0xFC;
		L2Cache[LineAddress].Data[BlockOffset + 3] = WriteValue >> 24;
		L2Cache[LineAddress].Data[BlockOffset + 2] = WriteValue >> 16;
		L2Cache[LineAddress].Data[BlockOffset + 1] = WriteValue >> 8;
		L2Cache[LineAddress].Data[BlockOffset + 0] = WriteValue;
		break;
	case 8:
		BlockOffset = BlockOffset & 0xF8; // 需对齐到8字节边界
		L2Cache[LineAddress].Data[BlockOffset + 7] = WriteValue >> 56;
		L2Cache[LineAddress].Data[BlockOffset + 6] = WriteValue >> 48;
		L2Cache[LineAddress].Data[BlockOffset + 5] = WriteValue >> 40;
		L2Cache[LineAddress].Data[BlockOffset + 4] = WriteValue >> 32;
		L2Cache[LineAddress].Data[BlockOffset + 3] = WriteValue >> 24;
		L2Cache[LineAddress].Data[BlockOffset + 2] = WriteValue >> 16;
		L2Cache[LineAddress].Data[BlockOffset + 1] = WriteValue >> 8;
		L2Cache[LineAddress].Data[BlockOffset + 0] = WriteValue;
		break;
	default:
		printf("[%s] Error DataSize=%d\n", __func__, DataSize);
		break;
	}
	return;
}

/*
 * L2Cache 访问接口
 * Address: 64位地址
 * DataSize: 读取数据的大小
 * 返回值: 读取的数据
 */

UINT8 AccessL2Cache(UINT64 Address, UINT8 Operation, UINT8 DataSize, UINT64 StoreValue, UINT64* LoadResult)
{
	UINT32 CacheLineAddress;
	UINT8 BlockOffset;
	UINT64 AddressTag;
	UINT8 MissFlag = 'M';
	UINT64 ReadValue;

	*LoadResult = 0;

	CacheLineAddress = (Address >> L2CACHE_DATA_PER_LINE_ADDR_BITS) % L2CACHE_LINE;
	BlockOffset = Address % L2CACHE_DATA_PER_LINE;
	AddressTag = (Address >> L2CACHE_DATA_PER_LINE_ADDR_BITS) >> L2CACHE_LINE_ADDR_BITS;
	if (DEBUG) {
		printf("L2: [%s] Address=0x%llx, CacheLineAddress=0x%x, BlockOffset=0x%x, AddressTag=0x%llx\n", __func__, Address, CacheLineAddress, BlockOffset, AddressTag);
		printf("L2: [%s] Operation=%c, DataSize=%d, StoreValue=0x%llx\n", __func__, Operation, DataSize, StoreValue);

	}
	if (L2Cache[CacheLineAddress].Valid == 1 && L2Cache[CacheLineAddress].Tag == AddressTag)
	{
		MissFlag = 'H';
	}
	else
	{
		MissFlag = 'M';
	}

	if (MissFlag == 'H') // L2Cache Hit
	{
		if (Operation == 'L') // Load
		{
			ReadValue = ReadL2Cache(CacheLineAddress, DataSize, BlockOffset);
			*LoadResult = ReadValue;
		}
		else if (Operation == 'S' || Operation == 'M') // Store or Modify
		{
			WriteL2Cache(CacheLineAddress, DataSize, BlockOffset, StoreValue);
		}
		else
		{
			printf("[%s] Error Operation=%c\n", __func__, Operation);
		}
	}
	else // L2Cache Miss
	{
		if (L2Cache[CacheLineAddress].Valid == 1)
		{
			// UINT64 OldAddress = (L2Cache[CacheLineAddress].Tag << L2CACHE_LINE_ADDR_BITS) + CacheLineAddress;
			UINT64 OldAddress = ((L2Cache[CacheLineAddress].Tag << L2CACHE_LINE_ADDR_BITS) << L2CACHE_DATA_PER_LINE_ADDR_BITS) |
				((UINT64)CacheLineAddress << L2CACHE_DATA_PER_LINE_ADDR_BITS);
			StoreL2CacheLineToMemory(OldAddress, CacheLineAddress);
		}
		LoadL2CacheLineFromMemory(Address, CacheLineAddress);
		L2Cache[CacheLineAddress].Valid = 1;
		L2Cache[CacheLineAddress].Tag = AddressTag;
		*LoadResult = ReadL2Cache(CacheLineAddress, DataSize, BlockOffset);
		if (Operation == 'L')
		{
			// ReadValue = ReadL2Cache(CacheLineAddress, DataSize, BlockOffset);
			// *LoadResult = ReadValue;
		}
		else if (Operation == 'S' || Operation == 'M')
		{
			WriteL2Cache(CacheLineAddress, DataSize, BlockOffset, StoreValue);
		}
		else
		{
			printf("[%s] Error Operation=%c\n", __func__, Operation);
		}
	}
	if (DEBUG)
		printf("L2: LoadResult=0x%llx\n", *LoadResult);
	return MissFlag;
}

// DCache_to_L2
void DCache_to_L2(UINT64 Address, UINT8 DataSize, UINT64 Value)
{
	UINT32 CacheLineAddress;
	UINT8 BlockOffset;
	UINT64 AddressTag;
	UINT8 MissFlag = 'M';
	UINT64 ReadValue;

	CacheLineAddress = (Address >> L2CACHE_DATA_PER_LINE_ADDR_BITS) % L2CACHE_LINE;
	BlockOffset = Address % L2CACHE_DATA_PER_LINE;
	AddressTag = (Address >> L2CACHE_DATA_PER_LINE_ADDR_BITS) >> L2CACHE_LINE_ADDR_BITS;

	if (L2Cache[CacheLineAddress].Valid == 1)
	{ // L2Cache中有数据
		UINT64 OldAddress = (L2Cache[CacheLineAddress].Tag << L2CACHE_LINE_ADDR_BITS) + CacheLineAddress;
		StoreL2CacheLineToMemory(OldAddress, CacheLineAddress);
	}
	// 讲Value写到L2Cache中
	L2Cache[CacheLineAddress].Valid = 1;
	L2Cache[CacheLineAddress].Tag = AddressTag;
	WriteL2Cache(CacheLineAddress, DataSize, BlockOffset, Value);
}

// L2 to L1
void L2_to_L1(UINT64 Address, UINT8 DataSize, UINT64* Value)
{
	UINT32 CacheLineAddress;
	UINT8 BlockOffset;
	UINT64 AddressTag;
	UINT8 MissFlag = 'M';
	UINT64 ReadValue;

	CacheLineAddress = (Address >> L2CACHE_DATA_PER_LINE_ADDR_BITS) % L2CACHE_LINE;
	BlockOffset = Address % L2CACHE_DATA_PER_LINE;
	AddressTag = (Address >> L2CACHE_DATA_PER_LINE_ADDR_BITS) >> L2CACHE_LINE_ADDR_BITS;

	if (L2Cache[CacheLineAddress].Valid == 1 && L2Cache[CacheLineAddress].Tag == AddressTag)
	{
		MissFlag = 'H';
	}
	else
	{
		MissFlag = 'M';
	}

	if (MissFlag == 'H')
	{
		ReadValue = ReadL2Cache(CacheLineAddress, DataSize, BlockOffset);
		*Value = ReadValue;
	}
	else
	{
		if (L2Cache[CacheLineAddress].Valid == 1)
		{
			UINT64 OldAddress = (L2Cache[CacheLineAddress].Tag << L2CACHE_LINE_ADDR_BITS) + CacheLineAddress;
			StoreL2CacheLineToMemory(OldAddress, CacheLineAddress);
		}
		LoadL2CacheLineFromMemory(Address, CacheLineAddress);
		L2Cache[CacheLineAddress].Valid = 1;
		L2Cache[CacheLineAddress].Tag = AddressTag;
		ReadValue = ReadL2Cache(CacheLineAddress, DataSize, BlockOffset);
		*Value = ReadValue;
	}
}
