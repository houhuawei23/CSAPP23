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
	每行存放64个字节，共256行
*/
#define DCACHE_SIZE 16384													// 16 KB
#define DCACHE_DATA_PER_LINE 16												// 每行的字节数
#define DCACHE_DATA_PER_LINE_ADDR_BITS GET_POWER_OF_2(DCACHE_DATA_PER_LINE) // 必须与上面设置一致，即64字节，需要6位地址 (b)
#define DCACHE_LINE (DCACHE_SIZE / DCACHE_DATA_PER_LINE)					// Cache的行数
#define DCACHE_LINE_ADDR_BITS GET_POWER_OF_2(DCACHE_LINE)					// 必须与上面设置一致，即256行，需要8位地址  (s)

// Cache行的结构，包括Valid、Tag和Data。你所有的状态信息，只能记录在Cache行中！
struct DCACHE_LineStruct
{
	UINT8 Valid;
	UINT64 Tag;
	UINT8 Data[DCACHE_DATA_PER_LINE];
} DCache[DCACHE_LINE];

/*
	DCache初始化代码，一般需要把DCache的有效位Valid设置为0
	模拟器启动时，会调用此InitDataCache函数
*/
void InitDataCache()
{
	UINT32 i;
	printf("[%s] +-----------------------------------+\n", __func__);
	printf("[%s] |   hhw的Data Cache初始化ing.... |\n", __func__);
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
UINT64 ReadCache(UINT32 LineAddress, UINT8 DataSize, UINT8 BlockOffset)
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

void WriteCache(UINT32 LineAddress, UINT8 DataSize, UINT8 BlockOffset, UINT64 StoreValue)
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

	if (DCache[CacheLineAddress].Valid == 1 && DCache[CacheLineAddress].Tag == AddressTag)
	{
		MissFlag = 'H'; // 命中！

		if (Operation == 'L') // 读操作
		{
			ReadValue = ReadCache(CacheLineAddress, DataSize, BlockOffset);
			*LoadResult = ReadValue;
			if (DEBUG)
				printf("[%s] Address=%016llX Operation=%c DataSize=%u StoreValue=%016llX ReadValue=%016llX\n", __func__, Address, Operation, DataSize, StoreValue, ReadValue);
		}
		else if (Operation == 'S' || Operation == 'M') // 写操作（修改操作在此等价于写操作）
		{
			if (DEBUG)
				printf("[%s] Address=%016llX Operation=%c DataSize=%u StoreValue=%016llX\n", __func__, Address, Operation, DataSize, StoreValue);
			WriteCache(CacheLineAddress, DataSize, BlockOffset, StoreValue);
		}
	}
	else
	{
		if (DEBUG)
			printf("[%s] Address=%016llX Operation=%c DataSize=%u StoreValue=%016llX\n", __func__, Address, Operation, DataSize, StoreValue);
		MissFlag = 'M'; // 不命中
		if (DCache[CacheLineAddress].Valid == 1)
		{
			// 淘汰对应的Cache行，如果对应的Cache行有数据，需要写回到Memory中
			UINT64 OldAddress;
			// OldAddress = > (Tag,Set,0000)
			// 从Tag中恢复旧的地址
			OldAddress = ((DCache[CacheLineAddress].Tag << DCACHE_LINE_ADDR_BITS) << DCACHE_DATA_PER_LINE_ADDR_BITS) |
						 ((UINT64)CacheLineAddress << DCACHE_DATA_PER_LINE_ADDR_BITS);
			StoreDataCacheLineToMemory(OldAddress, CacheLineAddress);
		}
		// 需要从Memory中读入新的行（真实情况下，这个LoadCacheLineFromMemory需要很长时间的）
		LoadDataCacheLineFromMemory(Address, CacheLineAddress);
		DCache[CacheLineAddress].Valid = 1;
		DCache[CacheLineAddress].Tag = AddressTag;
		if (Operation == 'L') // 读操作
		{
			// 读操作不需要做事情，因为已经MISS了
		}
		else if (Operation == 'S' || Operation == 'M') // 写操作（修改操作在此等价于写操作）
		{
			// 写操作，需要将新的StoreValue更新到CacheLine中
			WriteCache(CacheLineAddress, DataSize, BlockOffset, StoreValue);
		}
	}
	return MissFlag;
}

/* 指令Cache实现部分，可选实现 */
void InitInstCache(void)
{
	return;
}

void LoadInstCacheLineFromMemory(UINT64 Address, UINT32 CacheLineAddress)
{
	return;
}

UINT8 AccessInstCache(UINT64 Address, UINT8 Operation, UINT8 InstSize, UINT64* InstResult)
{
	// 返回值'M' = Miss，'H'=Hit
	return 'M';
}