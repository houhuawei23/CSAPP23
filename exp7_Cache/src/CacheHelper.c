///////////////////////////////////////////////////////////////////////
////  Copyright 2022 by mars.                                        //
///////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "getopt.h"
#include "cbsl.h"
// #define L2_CACHE 1
#define CBSL_ERROR_CHECK(X)                       \
	{                                             \
		if ((X) == cbsl_error)                    \
		{                                         \
			fprintf(stderr, "error: %s\n", (#X)); \
		}                                         \
	}

/*
fcn  ideal%     #items   #buckets  dup%  fl   add_usec  find_usec  del-all usec
---  ------ ---------- ---------- -----  -- ---------- ----------  ------------
OAT   89.1%     597731     524288    0%  ok     317292     179676         59726
FNV   88.6%     597731     262144    0%  ok     226650     220626         61619
JEN   87.8%     597731     524288    0%  ok     321989     175945         59956
BER   86.4%     597731     262144    0%  ok     198477     179332         60901
SAX   70.4%     597731     524288    0%  ok     270281     196427         64064
SFH   69.2%     597731     524288    0%  ok     289843     165105         61860
FNV1A_Pippip_Yurii 更快？？
*/

// 使用OAT hash算法，以获得最高性能
#include "uthash.h"
#undef HASH_FUNCTION
// #define HASH_FUNCTION HASH_OAT
#define HASH_FUNCTION(keyptr, keylen, hashv) (hashv) = FNV1A_Pippip_Yurii((UINT64 *)keyptr)

#define VERBOSE_MSG 0

#ifdef _WIN32
#define ZLIB_WINAPI
#endif

#include "common.h"

#ifdef _WIN64
#pragma comment(lib, "..\\zstd\\libzstd_static-win64.lib")
#elif _WIN32
#pragma comment(lib, "..\\zstd\\libzstd_static-win32.lib")
#endif

/*
	构建一个用Hash表访问的Memory，以便能够紧凑存放稀疏、随机的Memory数据
*/

struct MemoryDataStruct
{
	UINT64 Address;		   // 地址（默认是64位字地址）
	UINT64 Data;		   // 数据（默认保存64位字数据）
	UT_hash_handle memory; // makes this structure hashable
} *MemoryHash = NULL;

static void WriteMemoryHash(UINT64 Address, UINT64 WriteValue, UINT8 WriteSize);
static UINT64 ReadMemoryHash(UINT64 Address, UINT8 ReadSize);

#define MEMORY_TRACE_CHUNK (1 << 20) // MemoryTrace结构，每次增长1M条记录
struct MemoryTraceStruct
{
	UINT8 Operation;
	UINT64 Address;
	UINT8 Size;
} *MemoryTrace;
UINT64 MemoryTraceCounter, MemoryTraceCapacity;

/*
 *	统计计数器
 */
UINT64 GlobalMemoryModifyCounter;
UINT64 GlobalMemoryInstCounter;
UINT64 GlobalMemoryWriteCounter;
UINT64 GlobalCacheWriteHitCounter;
UINT64 GlobalMemoryReadCounter;
UINT64 GlobalCacheReadHitCounter;
UINT64 GlobalCacheModifyHitCounter;
UINT64 GlobalSimReadMemoryCounter;
UINT64 GlobalSimWriteMemoryCounter;

UINT64 GlobalCacheInstHitCounter;

#define CHUNK 16384

static void Trim(char *src)
{
	char *begin = src;
	char *end = src + strlen(src);

	if (begin == end)
		return;

	while (*begin == ' ' || *begin == '\t')
		++begin;
	while ((*end) == '\0' || *end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')
		--end;

	if (begin > end)
	{
		*src = '\0';
		return;
	}
	while (begin != end)
	{
		*src++ = *begin++;
	}

	*src++ = *end;
	*src = '\0';

	return;
}

typedef struct
{
	const char *start;
	size_t len;
} token;

// https://stackoverflow.com/a/39286524
static char **split_space(const char *str, int *field_count)
{
	char **array;
	unsigned int start = 0, stop, toks = 0, t;
	token *tokens = malloc((strlen(str) + 1) * sizeof(token));
	for (stop = 0; str[stop]; stop++)
	{
		if (str[stop] == ' ' || str[stop] == '\t' || str[stop] == ',')
		{
			tokens[toks].start = str + start;
			tokens[toks].len = stop - start;
			toks++;
			start = stop + 1;
		}
	}
	/* Mop up the last token */
	tokens[toks].start = str + start;
	tokens[toks].len = stop - start;
	toks++;
	array = malloc(toks * sizeof(char *));
	for (t = 0; t < toks; t++)
	{
		/* Calloc makes it nul-terminated */
		char *token = calloc(tokens[t].len + 1, 1);
		memcpy(token, tokens[t].start, tokens[t].len);
		array[t] = token;
	}
	free(tokens);
	*field_count = toks;
	return array;
}

#define SR_A ((UINT64)1103515245)
#define SR_C ((UINT64)12345)
#define SR_M ((UINT64)1 << 32)
UINT64 Xn;

static void SyncRand(UINT64 Seed)
{
	Xn = Seed;
}

static UINT8 GetRand8()
{
	Xn = ((SR_A * Xn + SR_C) % SR_M);
	return (UINT8)Xn;
}

static UINT16 GetRand16()
{
	Xn = ((SR_A * Xn + SR_C) % SR_M);
	return (UINT16)Xn;
}

static UINT32 GetRand32()
{
	Xn = ((SR_A * Xn + SR_C) % SR_M);
	return (UINT32)Xn;
}

static UINT64 GetRand64()
{
	UINT32 Hi, Lo;
	Hi = GetRand32();
	Lo = GetRand32();

	return ((UINT64)Hi << 32 | Lo);
}

// https://www.codeproject.com/articles/716530/fastest-hash-function-for-table-lookups-in-c
static inline UINT32 FNV1A_Pippip_Yurii(UINT64 *Address)
{
	const UINT32 PRIME = 591798841;
	UINT32 hash32;
	UINT64 hash64 = 14695981039346656037ULL;
	hash64 = (hash64 ^ *Address) * PRIME;
	hash32 = (uint32_t)(hash64 ^ (hash64 >> 32));
	return hash32 ^ (hash32 >> 16);
}

static void InitMemoryHash()
{
}

static void FreeMemoryHash()
{
	struct MemoryDataStruct *p, *tmp;
	HASH_ITER(memory, MemoryHash, p, tmp)
	{
		HASH_DELETE(memory, MemoryHash, p);
		free(p);
	}
}

static void UpdateMemoryData(UINT64 *Location, UINT8 Offset, UINT64 WriteValue, UINT8 WriteSize)
{
	UINT64 OriginValue;
	OriginValue = *Location;
	switch (WriteSize)
	{
	case 1:					  // 1个字节
		Offset = Offset << 3; // 从字节换算成位
		OriginValue = (OriginValue & ~((UINT64)0xFF << Offset)) | ((WriteValue & (UINT64)0xFF) << Offset);
		break;
	case 2:						// 2个字节
		Offset = Offset & 0xFE; // 对齐到2字节边界
		Offset = Offset << 3;	// 从字节换算成位
		OriginValue = (OriginValue & ~((UINT64)0xFFFF << Offset)) | ((WriteValue & (UINT64)0xFFFF) << Offset);
		break;
	case 4:						// 4个字节
		Offset = Offset & 0xFC; // 对齐到4字节边界
		Offset = Offset << 3;	// 从字节换算成位
		OriginValue = (OriginValue & ~((UINT64)0xFFFFFFFF << Offset)) | ((WriteValue & (UINT64)0xFFFFFFFF) << Offset);
		break;
	case 8: // 8个字节
		OriginValue = WriteValue;
		break;
	}
	*Location = OriginValue;
}

static void WriteMemoryHash(UINT64 Address, UINT64 WriteValue, UINT8 WriteSize)
{
	UINT64 AlignAddress;
	UINT8 Offset;
	struct MemoryDataStruct *s;

	AlignAddress = Address & 0xFFFFFFFFFFFFFFF8;
	Offset = Address & 0x7;

	HASH_FIND(memory, MemoryHash, &AlignAddress, sizeof(AlignAddress), s);
	if (s == NULL)
	{
		// 没有在MemoryData中命中
		s = (struct MemoryDataStruct *)malloc(sizeof(struct MemoryDataStruct));
		s->Address = AlignAddress;
		s->Data = 0xDEADBEEFDEADC0DE;
		HASH_ADD(memory, MemoryHash, Address, sizeof(AlignAddress), s);
	}
	UpdateMemoryData(&(s->Data), Offset, WriteValue, WriteSize);
}

static UINT64 ReadMemoryHash(UINT64 Address, UINT8 ReadSize)
{
	UINT64 AlignAddress;
	UINT8 Offset;
	struct MemoryDataStruct *s;
	UINT64 OriginValue, ReadValue;

	AlignAddress = Address & 0xFFFFFFFFFFFFFFF8;
	Offset = Address & 0x7;

	HASH_FIND(memory, MemoryHash, &AlignAddress, sizeof(AlignAddress), s);
	if (s == NULL)
	{
		// 没有在MemoryData中命中，出错了！
		if (VERBOSE_MSG)
			printf("[%s] 试图从一个未初始化的内存读取数据Address=%016llX！\n", __func__, Address);
		OriginValue = 0xDEADBEEFDEADC0DE;
	}
	else
	{
		OriginValue = s->Data;
	}

	/*
	 *  根据Offset和Size，对读出的数据进行修正，对齐到小端
	 */
	ReadValue = OriginValue;
	switch (ReadSize)
	{
	case 1:					  // 1个字节
		Offset = Offset << 3; // 从字节换算成位
		ReadValue = (OriginValue >> Offset) & 0xFF;
		break;
	case 2:						// 2个字节
		Offset = Offset & 0xFE; // 对齐到2字节边界
		Offset = Offset << 3;	// 从字节换算成位
		ReadValue = (OriginValue >> Offset) & 0xFFFF;
		break;
	case 4:						// 4个字节
		Offset = Offset & 0xFC; // 对齐到4字节边界
		Offset = Offset << 3;	// 从字节换算成位
		ReadValue = (OriginValue >> Offset) & 0xFFFFFFFF;
		break;
	case 8: // 8个字节
		ReadValue = OriginValue;
		break;
	}
	return ReadValue;
}

UINT64 ReadMemory(UINT64 Address)
{
	GlobalSimReadMemoryCounter++;
	return ReadMemoryHash(Address, 8);
}

void WriteMemory(UINT64 Address, UINT64 WriteData)
{
	GlobalSimWriteMemoryCounter++;
	WriteMemoryHash(Address, WriteData, 8);
}

static void MemoryTraceStat()
{
	UINT64 i;

	GlobalMemoryInstCounter = 0;
	GlobalMemoryReadCounter = 0;
	GlobalMemoryWriteCounter = 0;
	GlobalMemoryModifyCounter = 0;

	for (i = 0; i < MemoryTraceCounter; i++)
	{
		if (MemoryTrace[i].Operation == 'I')
			GlobalMemoryInstCounter++;
		else if (MemoryTrace[i].Operation == 'L')
			GlobalMemoryReadCounter++;
		else if (MemoryTrace[i].Operation == 'S')
			GlobalMemoryWriteCounter++;
		else if (MemoryTrace[i].Operation == 'M')
			GlobalMemoryModifyCounter++;
	}
}

static int process_TRACE_line(char *linebuf, UINT64 lineno)
{
	int i;
	int ret = 0;
	UINT8 Operation;
	UINT64 Address;
	UINT8 Size;

	UINT64 RandValue64;

	// 删除行首、行尾空白回车等
	Trim(linebuf);
	int linelen = (int)strlen(linebuf);
	// 跳过空行
	if (linelen == 0)
		return 0;
	// 跳过超长的行
	if (linelen >= 100)
		return 0;
	// 跳过注释行
	if (linebuf[0] == '#' || linebuf[0] == '/' || linebuf[0] == '=' || linebuf[0] == '-')
		return 0;
	// printf("lineno=%llu\n", lineno);
	char **pt;
	int field_count;
	pt = split_space(linebuf, &field_count);

	if (field_count == 3 || field_count == 4)
	{
		// 格式： Operation Address,Size
		Operation = pt[0][0];
		if (field_count == 3)
		{
			Address = strtoull(pt[1], NULL, 16);
			Size = atoi(pt[2]);
		}
		else
		{
			Address = strtoull(pt[2], NULL, 16);
			Size = atoi(pt[3]);
		}

		if (Operation != 'I' && Operation != 'L' && Operation != 'S' && Operation != 'M')
		{
			if (VERBOSE_MSG)
				printf("[%s] line %lld 操作类型错误，不是I、L、S、M！ %s\n", __func__, lineno, linebuf);
			ret = -1;
		}
		else if (Size != 1 && Size != 2 && Size != 4 && Size != 8)
		{
			if (VERBOSE_MSG)
				printf("[%s] line %lld 数据大小错误，不是1、2、4、8！ %s\n", __func__, lineno, linebuf);
			ret = -1;
		}
		else
		{
			RandValue64 = GetRand64();

			// 初始化阶段，需要将所有地址的数据，设置为随机值
			WriteMemoryHash(Address, RandValue64, Size);

			if (MemoryTraceCounter >= MemoryTraceCapacity)
			{
				if (VERBOSE_MSG)
					printf("[%s] line %lld 扩展内存到%llu！\n", __func__, lineno, MemoryTraceCapacity + MEMORY_TRACE_CHUNK);
				MemoryTrace = (struct MemoryTraceStruct *)realloc(MemoryTrace, (MemoryTraceCapacity + MEMORY_TRACE_CHUNK) * sizeof(struct MemoryTraceStruct));
				if (MemoryTrace == NULL)
				{
					printf("[%s] line %lld 分配内存失败！\n", __func__, lineno);
					ret = -1;
				}
				else
					MemoryTraceCapacity += MEMORY_TRACE_CHUNK;
			}

			if (MemoryTrace)
			{
				MemoryTrace[MemoryTraceCounter].Operation = Operation;
				MemoryTrace[MemoryTraceCounter].Address = Address;
				MemoryTrace[MemoryTraceCounter].Size = Size;
				MemoryTraceCounter++;

				ret = 0;
			}
		}
	}
	else
	{
		printf("[%s] line %lld 格式错误！[fields=%d] %s\n", __func__, lineno, field_count, linebuf);
		ret = -1;
	}

	for (i = 0; i < field_count; i++)
		free(pt[i]);
	free(pt);
	return ret;
}

static int parse_TRACE_file(char *filename)
{
	UINT64 lineno;
	int ret_parse_line = 0;
	clock_t tick1, tick2;
	cbsl_errors cbsl_ret = cbsl_error;
	char linebuf[CHUNK];

	tick1 = clock();
	cbsl_ctx *ctx = cbsl_open(cbsl_load_mode, filename);
	if (ctx == NULL)
	{
		printf("[%s] 不能以读方式打开Trace文件 %s\n", __func__, filename);
		return -1;
	}

	lineno = 0;
	do
	{
		lineno++;
		cbsl_ret = cbsl_readline(ctx, linebuf, sizeof(linebuf));
		CBSL_ERROR_CHECK(cbsl_ret); // 从文件中读取1行

		ret_parse_line = process_TRACE_line(linebuf, lineno); // 分析处理1行
		if (ret_parse_line == 1 || cbsl_ret == cbsl_end)
			break;

		// if (lineno % 10000 == 0)
		// {
		// 	printf("\33[?25l[%s] ====已处理%llu行====\r", __func__, lineno); // 隐藏光标，显示进度
		// }
	} while (1);

	// printf("\n\33[?25h"); // 显示光标
	CBSL_ERROR_CHECK(cbsl_close(ctx));

	tick2 = clock();

	MemoryTraceStat();
	printf("[%s] +-----------------------------------------------------+\n", __func__);
	printf("[%s] |  Memory Trace数量                 \t : %10llu    |\n", __func__, MemoryTraceCounter);
	printf("[%s] |    Instruction操作数量            \t : %10llu    |\n", __func__, GlobalMemoryInstCounter);
	printf("[%s] |    Data Load操作数量              \t : %10llu    |\n", __func__, GlobalMemoryReadCounter);
	printf("[%s] |    Data Store操作数量             \t : %10llu    |\n", __func__, GlobalMemoryWriteCounter);
	printf("[%s] |    Data Modify操作数量            \t : %10llu    |\n", __func__, GlobalMemoryModifyCounter);
	printf("[%s] | 时间耗费（ms）                    \t : %10.0f    |\n", __func__, ((float)(tick2 - tick1) / CLOCKS_PER_SEC) * 1000.0);
	printf("[%s] +-----------------------------------------------------+\n", __func__);

	if (MemoryTraceCounter == 0)
		return -1;
	return 0;
}

int SimTrace()
{
	int ret = 0;
	UINT32 i;
	UINT8 Operation;
	UINT64 Address;
	UINT8 Size;

	UINT64 RandValue64;
	UINT64 DataFromCache;
	UINT64 DataFromMemory;
	UINT8 MissFlag;

	clock_t tick1, tick2;
	tick1 = clock();

	GlobalMemoryInstCounter = 0;
	GlobalMemoryReadCounter = 0;
	GlobalMemoryWriteCounter = 0;
	GlobalMemoryModifyCounter = 0;

	GlobalCacheInstHitCounter = 0;
	GlobalCacheReadHitCounter = 0;
	GlobalCacheWriteHitCounter = 0;
	GlobalCacheModifyHitCounter = 0;

	GlobalSimReadMemoryCounter = 0;
	GlobalSimWriteMemoryCounter = 0;

	for (i = 0; i < MemoryTraceCounter; i++)
	{
		Operation = MemoryTrace[i].Operation;
		Address = MemoryTrace[i].Address;
		Size = MemoryTrace[i].Size;

		if (Operation == 'L' || Operation == 'S' || Operation == 'M')
		{
			RandValue64 = GetRand64();
			MissFlag = AccessDataCache(Address, Operation, Size, RandValue64, &DataFromCache);
			if (Operation == 'S' || Operation == 'M')
			{
				WriteMemoryHash(Address, RandValue64, Size);
				if (Operation == 'S')
				{
					GlobalMemoryWriteCounter++;
					if (MissFlag == 'H')
					{
						GlobalCacheWriteHitCounter++;
					}
				}
				else if (Operation == 'M')
				{
					GlobalMemoryModifyCounter++;
					if (MissFlag == 'H')
					{
						GlobalCacheModifyHitCounter++;
					}
				}
			}
			else if (Operation == 'L')
			{
				GlobalMemoryReadCounter++;
				if (MissFlag == 'H')
				{
					DataFromMemory = ReadMemoryHash(Address, Size);
					if (DataFromMemory == DataFromCache)
						GlobalCacheReadHitCounter++;
					else
					{
						printf("[%s] 关键错误！数据Cache读错误，内存地址=%016llX 内存数据=%016llX Cache读数据=%016llX 大小%d字节\n", __func__, Address, DataFromMemory, DataFromCache, Size);
						ret = -1;
						break;
					}
				}
			}
		}
		else if (Operation == 'I')
		{
			MissFlag = AccessInstCache(Address, Operation, Size, &DataFromCache);
			GlobalMemoryInstCounter++;
			if (MissFlag == 'H')
			{
				DataFromMemory = ReadMemoryHash(Address, Size);
				if (DataFromMemory == DataFromCache)
					GlobalCacheInstHitCounter++;
				else
				{
					printf("[%s] 关键错误！指令Cache读错误，内存地址=%016llX 内存数据=%016llX Cache读数据=%016llX 大小%d字节\n", __func__, Address, DataFromMemory, DataFromCache, Size);
					ret = -1;
					break;
				}
			}
		}
	}
	tick2 = clock();
	printf("[%s] +-----------------------------------------------------+\n", __func__);
	printf("[%s] |  Memory Trace数量                \t : %10llu    |\n", __func__, MemoryTraceCounter);
	printf("[%s] |    Instruction操作数量           \t : %10llu    |\n", __func__, GlobalMemoryInstCounter);
	printf("[%s] |    Data Load操作数量             \t : %10llu    |\n", __func__, GlobalMemoryReadCounter);
	printf("[%s] |    Data Store操作数量            \t : %10llu    |\n", __func__, GlobalMemoryWriteCounter);
	printf("[%s] |    Data Modify操作数量           \t : %10llu    |\n", __func__, GlobalMemoryModifyCounter);
	printf("[%s] |    Instruction操作Cache命中数量  \t : %10llu    |\n", __func__, GlobalCacheInstHitCounter);
	printf("[%s] |    Data Load操作Cache命中数量    \t : %10llu    |\n", __func__, GlobalCacheReadHitCounter);
	printf("[%s] |    Data Store操作Cache命中数量   \t : %10llu    |\n", __func__, GlobalCacheWriteHitCounter);
	printf("[%s] |    Data Modify操作Cache命中数量  \t : %10llu    |\n", __func__, GlobalCacheModifyHitCounter);
	printf("[%s] |  Cache访存数量                   \t : %10llu    |\n", __func__, GlobalSimReadMemoryCounter + GlobalSimWriteMemoryCounter);
	printf("[%s] |    Cache读存储器数量             \t : %10llu    |\n", __func__, GlobalSimReadMemoryCounter);
	printf("[%s] |    Cache写存储器数量             \t : %10llu    |\n", __func__, GlobalSimWriteMemoryCounter);
	printf("[%s] |  Data Cache命中率                \t : %9.2f%%    |\n", __func__, (double)((GlobalCacheReadHitCounter + GlobalCacheWriteHitCounter + GlobalCacheModifyHitCounter) * 100) / (double)(GlobalMemoryReadCounter + GlobalMemoryWriteCounter + GlobalMemoryModifyCounter));
	printf("[%s] |  Inst Cache命中率                \t : %9.2f%%    |\n", __func__, (GlobalMemoryInstCounter == 0) ? 0 : (double)(GlobalCacheInstHitCounter * 100) / (double)GlobalMemoryInstCounter);
	printf("[%s] |  时间耗费（ms）                  \t : %10.0f    |\n", __func__, ((float)(tick2 - tick1) / CLOCKS_PER_SEC) * 1000.0);
	printf("[%s] +-----------------------------------------------------+\n", __func__);
	return ret;
}
void DisplayHelp(char *argv[])
{
	printf("[%s] 请在Cache.c中，实现你自己的Cache，然后编译项目，执行。\n", __func__);
	printf("[%s] 从文本格式压缩文件中读取Trace:\t%s <trace>.zst\n", __func__, argv[0]);
	printf("[%s]     例如: %s ./traces/dave.trace.zst\n", __func__, argv[0]);
	printf("[%s] 将文本格式的Trace转换到bin格式:\t%s -w <trace>.zst\n", __func__, argv[0]);
	printf("[%s] 从bin格式文件中读取Trace:\t\t%s -r <trace>.bin.zst\n", __func__, argv[0]);
	printf("[%s] 提示：从bin中读取Trace速度要远远快于从文本格式中读取。\n", __func__);
}

int main(int argc, char *argv[])
{
	int ret_val = -1;
	UINT8 ReadBinFileFlag, WriteBinFileFlag, ReadTxtFileFlag;
	char *pfilename;
	char pfilename_bin[CHUNK];
	clock_t tick1, tick2;
	UINT64 i;

	printf("[%s] Cache模拟器框架 v3.0 by mars, 2022\n", __func__);

	pfilename = NULL;
	ReadBinFileFlag = 0;
	WriteBinFileFlag = 0;
	ReadTxtFileFlag = 1;
	/* check arguments */
	while (1)
	{
		int c = getopt(argc, argv, "-hrw");
		if (c == -1)
			break;

		switch (c)
		{
		case 'h':
			DisplayHelp(argv);
			return 1;
		case 'r':
			ReadBinFileFlag = 1;
			ReadTxtFileFlag = 0;
			break;
		case 'w':
			WriteBinFileFlag = 1;
			ReadTxtFileFlag = 0;
			break;
		case 1:
			pfilename = optarg;
			break;
		}
	}

	if ((ReadBinFileFlag == 1 || WriteBinFileFlag == 1 || ReadTxtFileFlag == 1) && pfilename == NULL)
	{
		DisplayHelp(argv);
		return 1;
	}

	InitMemoryHash();
	printf("[%s] 初始化存储器，读入Trace文件[%s]，请稍后...\n", __func__, pfilename);

	if (ReadTxtFileFlag || WriteBinFileFlag)
	{
		MemoryTrace = (struct MemoryTraceStruct *)malloc(MEMORY_TRACE_CHUNK * sizeof(struct MemoryTraceStruct));
		MemoryTraceCounter = 0;
		MemoryTraceCapacity = MEMORY_TRACE_CHUNK;
		ret_val = parse_TRACE_file(pfilename);
		if (ret_val != 0)
		{
			FreeMemoryHash();
			if (MemoryTrace)
				free(MemoryTrace);
			printf("[%s] 解压缩文件失败 %s\n", __func__, argv[1]);
			return -1;
		}

		if (WriteBinFileFlag)
		{
			// 将内存中的MemoryTrace保存到bin文件中
			int filenamelen = (int)strlen(pfilename);
			memcpy(pfilename_bin, pfilename, filenamelen);
			pfilename_bin[filenamelen] = '\0';
			if (filenamelen > 5)
			{
				if ((pfilename_bin[filenamelen - 4] == '.' && pfilename_bin[filenamelen - 3] == 'z' && pfilename_bin[filenamelen - 2] == 's' && pfilename_bin[filenamelen - 1] == 't') ||
					(pfilename_bin[filenamelen - 4] == '.' && pfilename_bin[filenamelen - 3] == 'Z' && pfilename_bin[filenamelen - 2] == 'S' && pfilename_bin[filenamelen - 1] == 'T'))
				{
					pfilename_bin[filenamelen - 3] = 'b';
					pfilename_bin[filenamelen - 2] = 'i';
					pfilename_bin[filenamelen - 1] = 'n';
					pfilename_bin[filenamelen - 0] = '.';
					pfilename_bin[filenamelen + 1] = 'z';
					pfilename_bin[filenamelen + 2] = 's';
					pfilename_bin[filenamelen + 3] = 't';
					pfilename_bin[filenamelen + 4] = '\0';

					cbsl_ctx *ctx = cbsl_open(cbsl_store_mode, pfilename_bin);
					if (ctx == NULL)
					{
						printf("[%s] 不能以写方式打开文件 %s\n", __func__, pfilename_bin);
						ret_val = -1;
					}
					else
					{
						CBSL_ERROR_CHECK(cbsl_write(ctx, &MemoryTraceCounter, sizeof(MemoryTraceCounter)));
						CBSL_ERROR_CHECK(cbsl_write(ctx, MemoryTrace, MemoryTraceCounter * sizeof(struct MemoryTraceStruct)));
						CBSL_ERROR_CHECK(cbsl_close(ctx));
						printf("[%s] 已经将Trace保存到文件中 %s\n", __func__, pfilename_bin);
						ret_val = 0;
					}
				}
				else
				{
					printf("[%s] 文件扩展名不是.zst或者.ZST！不能转换！\n", __func__);
					ret_val = -1;
				}
			}
			else
			{
				printf("[%s] 文件名长度不足5字符！不能转换！\n", __func__);
				ret_val = -1;
			}

			FreeMemoryHash();
			if (MemoryTrace)
				free(MemoryTrace);
			return ret_val;
		}
	}
	else if (ReadBinFileFlag)
	{
		cbsl_ctx *ctx = cbsl_open(cbsl_load_mode, pfilename);
		if (ctx == NULL)
		{
			printf("[%s] 不能以读方式打开文件 %s\n", __func__, pfilename);
			ret_val = -1;
		}
		else
		{
			tick1 = clock();
			CBSL_ERROR_CHECK(cbsl_read(ctx, &MemoryTraceCounter, sizeof(MemoryTraceCounter)));
			{
				MemoryTrace = (struct MemoryTraceStruct *)malloc(MemoryTraceCounter * sizeof(struct MemoryTraceStruct));
				MemoryTraceCapacity = MemoryTraceCounter;

				CBSL_ERROR_CHECK(cbsl_read(ctx, MemoryTrace, MemoryTraceCounter * sizeof(struct MemoryTraceStruct)));
				CBSL_ERROR_CHECK(cbsl_close(ctx));
				{
					// 初始化阶段，需要将所有地址的数据，设置为随机值
					UINT64 RandValue64;
					for (i = 0; i < MemoryTraceCounter; i++)
					{
						RandValue64 = GetRand64();
						WriteMemoryHash(MemoryTrace[i].Address, RandValue64, MemoryTrace[i].Size);
					}

					MemoryTraceStat();
					tick2 = clock();
					printf("[%s] +-----------------------------------------------------+\n", __func__);
					printf("[%s] |  Memory Trace数量                 \t : %10llu    |\n", __func__, MemoryTraceCounter);
					printf("[%s] |    Instruction操作数量            \t : %10llu    |\n", __func__, GlobalMemoryInstCounter);
					printf("[%s] |    Data Load操作数量              \t : %10llu    |\n", __func__, GlobalMemoryReadCounter);
					printf("[%s] |    Data Store操作数量             \t : %10llu    |\n", __func__, GlobalMemoryWriteCounter);
					printf("[%s] |    Data Modify操作数量            \t : %10llu    |\n", __func__, GlobalMemoryModifyCounter);
					printf("[%s] | 时间耗费（ms）                    \t : %10.0f    |\n", __func__, ((float)(tick2 - tick1) / CLOCKS_PER_SEC) * 1000.0);
					printf("[%s] +-----------------------------------------------------+\n", __func__);
					printf("[%s] 已经将Trace从文件中读取 %s\n", __func__, pfilename);
					ret_val = 0;
				}
			}
		}

		if (ret_val == -1)
		{
			if (MemoryTrace)
				free(MemoryTrace);
			FreeMemoryHash();
			return ret_val;
		}
	}
	printf("[%s] 处理Trace文件完毕\n", __func__);
	InitDataCache();
	InitInstCache();
	// for L2
#ifdef L2_CACHE
	InitL2Cache();
#endif
	printf("[%s] 开始Cache模拟，请稍后...\n", __func__);
	ret_val = SimTrace();
	if (ret_val != 0)
	{
		FreeMemoryHash();
		if (MemoryTrace)
			free(MemoryTrace);
		printf("[%s] Cache模拟失败\n", __func__);
		return -1;
	}
	else
		printf("[%s] Cache模拟成功完成\n", __func__);
	if (MemoryTrace)
		free(MemoryTrace);
	FreeMemoryHash();
	return 0;
}
