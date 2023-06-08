///////////////////////////////////////////////////////////////////////
////  Copyright 2020 by mars.                                        //
///////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef _WIN32
#define ZLIB_WINAPI
#endif

#include "common.h"
#include "zlib.h"

#ifdef _WIN64
#pragma comment(lib, "..\\zlibstat-win64.lib")
#elif _WIN32
#pragma comment(lib, "..\\zlibstat-win32.lib")
#endif

struct BT9_struct BT9;

PROCESS_STATE_ENUM PROCESS_STATE;

#define CHUNK 16384
// #define CHUNK 50
char *out_buf_ptr;
char in[CHUNK];
char out[CHUNK];
char linebuf[CHUNK];

int get_line_from_bt9(z_stream *strm, FILE *source)
{
	int ret;
	int need_read_file = 1; // read compressed file
	char *curr_line_ptr;

	curr_line_ptr = linebuf;
	// memset(linebuf, 0, CHUNK);

	// 如果out_buf_ptr为空，或者out中并没有包含换行符，则需要从流中解压
	if (out_buf_ptr == NULL)
		need_read_file = 1;
	else // out_buf_ptr不为空，且out中包含换行符，则直接从out中读取
		for (; out_buf_ptr < out + CHUNK - strm->avail_out; out_buf_ptr++, curr_line_ptr++)
		{
			// strm->avail_out 输出缓冲区剩余的空间大小
			// 若输出缓冲区还有未读取的元素(out_buf_ptr < out + CHUNK - strm->avail_out)，
			// 则将out缓冲区中的数据拷贝到linebuf中
			*curr_line_ptr = *out_buf_ptr; // 拷贝
			if (*out_buf_ptr == 0)		   // 如果拷贝的是空字符，则跳出循环
				break;
			if (*out_buf_ptr == '\n' || *out_buf_ptr == '\r') // 如果拷贝的是换行符，即读完一行了，则返回 Z_OK
			{
				out_buf_ptr++;
				*curr_line_ptr = 0;
				return Z_OK;
			}
		}
	// 到此已保证out缓冲区为空了，可以继续从in缓冲区中解压数据了
again:
	if (need_read_file)
	{
		if (strm->avail_in == 0) // 如果输入缓冲区为空，则从文件中读取数据
		{
			// 从压缩文件中读取数据到输入缓冲区，最大读取 CHUNK 个字节，返回实际读取的字节数
			strm->avail_in = (uInt)fread(in, 1, CHUNK, source);
			if (strm->avail_in == 0) // 若读取的字节数为0，则说明已经读取完毕，返回 0
				need_read_file = 0;
			if (ferror(source)) // 如果读取失败，则返回 Z_ERRNO
				return Z_ERRNO;
			strm->next_in = (unsigned char *)in; // 设置输入缓冲区的起始位置
		}

		/* run inflate() on input until output buffer not full */
		if (1)
		{
			// 从头开始填充out输出缓冲区
			strm->avail_out = CHUNK;
			strm->next_out = (unsigned char *)out;

			ret = inflate(strm, Z_NO_FLUSH); // 解压
			assert(ret != Z_STREAM_ERROR);	 /* state not clobbered */
			switch (ret)
			{
			case Z_NEED_DICT:		// 需要输入字典？？
				ret = Z_DATA_ERROR; /* and fall through */
				return ret;
			case Z_DATA_ERROR:
				return ret;
			case Z_MEM_ERROR:
				return ret;
			}
			// 确实解压出数据到out缓冲区了
			if (strm->avail_out < CHUNK)
			{
				ret = Z_STREAM_END;
				// 将out缓冲区中的数据拷贝到linebuf中
				for (out_buf_ptr = out; out_buf_ptr < out + CHUNK - strm->avail_out; out_buf_ptr++, curr_line_ptr++)
				{
					*curr_line_ptr = *out_buf_ptr;

					if (*out_buf_ptr == '\n' || *out_buf_ptr == '\r')
					{
						out_buf_ptr++;
						return Z_OK;
					}
				}
				// 前面没有遇到换行符，而是out缓冲区中的数据已经读完了，需要从文件中继续读取数据
				if (ret == Z_STREAM_END)
					goto again;
			}
		}
	}

	return ret;
}

void Trim(char *src)
{
	char *begin = src;
	char *end = src;

	while (*end++)
		;

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

char **split_space(const char *source, int *field_count)
{
	char **pt;
	int j, n = 0;
	int count = 1;
	int len = (int)strlen(source);
	char *tmp = (char *)malloc(len + 1);
	tmp[0] = '\0';

	*field_count = 0;
	for (int i = 0; i < len; ++i)
	{
		if ((source[i] == ' ' || source[i] == '\t') && source[i + 1] == '\0')
			continue;
		else if ((source[i] == ' ' || source[i] == '\t') && (source[i + 1] != ' ' && source[i + 1] != '\t'))
			count++;
	}
	// 多分配一个char*，是为了设置结束标志
	pt = (char **)malloc((count + 1) * sizeof(char *));

	count = 0;
	for (int i = 0; i < len; ++i)
	{
		if (i == len - 1 && (source[i] != ' ' && source[i] != '\t'))
		{
			tmp[n++] = source[i];
			tmp[n] = '\0'; // 字符串末尾添加空字符
			j = (int)strlen(tmp) + 1;
			pt[count] = (char *)malloc(j * sizeof(char));
			strncpy(pt[count++], tmp, j);
		}
		else if ((source[i] == ' ' || source[i] == '\t'))
		{
			j = (int)strlen(tmp);
			if (j != 0)
			{
				tmp[n] = '\0'; // 字符串末尾添加空字符
				pt[count] = (char *)malloc((j + 1) * sizeof(char));
				strncpy(pt[count++], tmp, (j + 1));
				// 重置tmp
				n = 0;
				tmp[0] = '\0';
			}
		}
		else
			tmp[n++] = source[i];
	}
	// 设置结束标志
	pt[count] = NULL;
	free(tmp);
	*field_count = count;
	return pt;
}

int process_bt9_line(char *linebuf, UINT64 lineno)
{
	int ret = 0;

	if (PROCESS_STATE != PROCESS_TRACE)
	{
		// 删除行首、行尾空白回车等
		Trim(linebuf);
		int linelen = (int)strlen(linebuf);
		// 跳过空行
		if (linelen == 0)
			return 0;
		// 跳过注释行
		if (linebuf[0] == '#' || linebuf[0] == '/')
			return 0;
		char **pt;
		int field_count;
		pt = split_space(linebuf, &field_count);

		switch (PROCESS_STATE)
		{
		case PROCESS_START:
			if (_strcmpi(pt[0], "BT9_SPA_TRACE_FORMAT") == 0)
			{
				ret = 0;
				break;
			}
			if (_strcmpi(pt[0], "bt9_minor_version:") == 0)
			{
				if (field_count == 1)
				{
					printf("[%s] line %lld bt9_minor_version missing number\n", __func__, lineno);
					ret = -1;
					break;
				}
				BT9.bt9_minor_version = atoi(pt[1]);
				ret = 0;
				break;
			}
			if (_strcmpi(pt[0], "has_physical_address:") == 0)
			{
				if (field_count == 1)
				{
					printf("[%s] line %lld has_physical_address missing number\n", __func__, lineno);
					ret = -1;
					break;
				}
				BT9.has_physical_address = atoi(pt[1]);
				ret = 0;
				break;
			}
			if (_strcmpi(pt[0], "md5_checksum:") == 0)
			{
				ret = 0;
				break;
			}
			if (_strcmpi(pt[0], "conversion_date:") == 0)
			{
				ret = 0;
				break;
			}
			if (_strcmpi(pt[0], "original_stf_input_file:") == 0)
			{
				ret = 0;
				break;
			}
			if (_strcmpi(pt[0], "total_instruction_count:") == 0)
			{
				if (field_count == 1)
				{
					printf("[%s] line %lld total_instruction_count missing number\n", __func__, lineno);
					ret = -1;
					break;
				}
				BT9.total_instruction_count = strtoull(pt[1], NULL, 10);
				ret = 0;
				break;
			}
			if (_strcmpi(pt[0], "branch_instruction_count:") == 0)
			{
				if (field_count == 1)
				{
					printf("[%s] line %lld branch_instruction_count missing number\n", __func__, lineno);
					ret = -1;
					break;
				}
				BT9.branch_instruction_count = strtoull(pt[1], NULL, 10);
				ret = 0;
				break;
			}
			if (_strcmpi(pt[0], "invalid_physical_branch_target_count:") == 0)
			{
				if (field_count == 1)
				{
					printf("[%s] line %lld invalid_physical_branch_target_count missing number\n", __func__, lineno);
					ret = -1;
					break;
				}
				BT9.invalid_physical_branch_target_count = strtoull(pt[1], NULL, 10);
				ret = 0;
				break;
			}
			if (_strcmpi(pt[0], "A32_instruction_count:") == 0)
			{
				if (field_count == 1)
				{
					printf("[%s] line %lld A32_instruction_count missing number\n", __func__, lineno);
					ret = -1;
					break;
				}
				BT9.A32_instruction_count = strtoull(pt[1], NULL, 10);
				ret = 0;
				break;
			}
			if (_strcmpi(pt[0], "A64_instruction_count:") == 0)
			{
				if (field_count == 1)
				{
					printf("[%s] line %lld A64_instruction_count missing number\n", __func__, lineno);
					ret = -1;
					break;
				}
				BT9.A64_instruction_count = strtoull(pt[1], NULL, 10);
				ret = 0;
				break;
			}
			if (_strcmpi(pt[0], "T32_instruction_count:") == 0)
			{
				if (field_count == 1)
				{
					printf("[%s] line %lld T32_instruction_count missing number\n", __func__, lineno);
					ret = -1;
					break;
				}
				BT9.T32_instruction_count = strtoull(pt[1], NULL, 10);
				ret = 0;
				break;
			}
			if (_strcmpi(pt[0], "unidentified_instruction_count:") == 0)
			{
				if (field_count == 1)
				{
					printf("[%s] line %lld unidentified_instruction_count missing number\n", __func__, lineno);
					ret = -1;
					break;
				}
				BT9.unidentified_instruction_count = strtoull(pt[1], NULL, 10);
				ret = 0;
				break;
			}
			if (_strcmpi(pt[0], "BT9_NODES") == 0)
			{
				PROCESS_STATE = PROCESS_NODE;
				ret = 0;
				break;
			}
			printf("[%s] line %lld Unknown keyword: %s\n", __func__, lineno, pt[0]);
			ret = -2;
			break;
		case PROCESS_NODE:
			if (_strcmpi(pt[0], "BT9_EDGES") == 0)
			{
				PROCESS_STATE = PROCESS_EDGE;
				ret = 0;
				break;
			}

			if (field_count < 6)
			{
				printf("[%s] line %lld Wrong node definition.. need more fields\n", __func__, lineno);
				ret = -2;
				break;
			}

			if (_strcmpi(pt[0], "NODE") != 0)
			{
				printf("[%s] line %lld Wrong node definition.. unknown keyword %s\n", __func__, lineno, pt[0]);
				ret = -2;
				break;
			}

			if (BT9.BT9_NODE_count % 1024 == 0)
			{
				// 重新扩充原来的大小，扩充1024份记录
				BT9.NODE = (struct BT9_NODE *)realloc(BT9.NODE, ((BT9.BT9_NODE_count / 1024) + 1) * 1024 * sizeof(struct BT9_NODE));
			}
			UINT32 id;
			UINT64 virtual_address;
			UINT64 physical_address;
			UINT64 opcode;
			UINT32 size;
			UINT32 i = BT9.BT9_NODE_count;
			OpType optype;
			optype = OPTYPE_ERROR;

			id = atoi(pt[1]);
			virtual_address = strtoull(pt[2], NULL, 16);
			physical_address = strtoull(pt[3], NULL, 16);
			opcode = strtoull(pt[4], NULL, 16);
			size = atoi(pt[5]);
			if (id != i)
			{
				printf("[%s] line %lld Wrong node definition.. id not in sequence, it should be %d\n", __func__, lineno, i);
				ret = -2;
				break;
			}

			if (field_count > 10)
			{
				if (_strcmpi(pt[7], "RET+IND+UCD") == 0)
					optype = OPTYPE_RET_UNCOND;
				else if (_strcmpi(pt[7], "RET+IND+CND") == 0)
					optype = OPTYPE_RET_COND;
				else if (_strcmpi(pt[7], "CALL+DIR+UCD") == 0)
					optype = OPTYPE_CALL_DIRECT_UNCOND;
				else if (_strcmpi(pt[7], "CALL+DIR+CND") == 0)
					optype = OPTYPE_CALL_DIRECT_COND;
				else if (_strcmpi(pt[7], "CALL+IND+CND") == 0)
					optype = OPTYPE_CALL_INDIRECT_COND;
				else if (_strcmpi(pt[7], "CALL+IND+UCD") == 0)
					optype = OPTYPE_CALL_INDIRECT_UNCOND;
				else if (_strcmpi(pt[7], "JMP+DIR+CND") == 0)
					optype = OPTYPE_JMP_DIRECT_COND;
				else if (_strcmpi(pt[7], "JMP+DIR+UCD") == 0)
					optype = OPTYPE_JMP_DIRECT_UNCOND;
				else if (_strcmpi(pt[7], "JMP+IND+UCD") == 0)
					optype = OPTYPE_JMP_INDIRECT_UNCOND;
				else if (_strcmpi(pt[7], "JMP+IND+CND") == 0)
					optype = OPTYPE_JMP_INDIRECT_COND;
				else
				{
					printf("[%s] lineno %lld Wrong Instruction Class %s\n", __func__, lineno, pt[7]);
					ret = -2;
					break;
				}
			}

			BT9.NODE[i].virtual_address = virtual_address;
			BT9.NODE[i].physical_address = physical_address;
			BT9.NODE[i].opcode = opcode;
			BT9.NODE[i].optype = optype;
			BT9.NODE[i].size = size;
			BT9.BT9_NODE_count++;
			break;
		case PROCESS_EDGE:
			if (_strcmpi(pt[0], "BT9_EDGE_SEQUENCE") == 0)
			{
				PROCESS_STATE = PROCESS_TRACE;
				ret = 0;
				break;
			}

			if (field_count < 10)
			{
				printf("[%s] line %lld Wrong edge definition.. need more fields\n", __func__, lineno);
				ret = -2;
				break;
			}

			if (_strcmpi(pt[0], "EDGE") != 0)
			{
				printf("[%s] line %lld Wrong edge definition.. unknown keyword %s\n", __func__, lineno, pt[0]);
				ret = -2;
				break;
			}

			if (BT9.BT9_EDGE_count % 1024 == 0)
			{
				// 重新扩充原来的大小，扩充1024份记录
				BT9.EDGE = (struct BT9_EDGE *)realloc(BT9.EDGE, ((BT9.BT9_EDGE_count / 1024) + 1) * 1024 * sizeof(struct BT9_EDGE));
			}
			//			UINT32 id;
			UINT32 src_id;
			UINT32 dest_id;
			char taken;
			UINT64 br_virt_target;
			UINT64 br_phy_target;
			UINT64 inst_cnt;
			UINT64 traverse_cnt;

			i = BT9.BT9_EDGE_count;
			id = atoi(pt[1]);
			src_id = atoi(pt[2]);
			dest_id = atoi(pt[3]);
			taken = pt[4][0];
			br_virt_target = strtoull(pt[5], NULL, 16);
			br_phy_target = strtoull(pt[6], NULL, 16);
			inst_cnt = strtoull(pt[7], NULL, 10);
			traverse_cnt = strtoull(pt[8], NULL, 10);

			if (id != i)
			{
				printf("[%s] line %lld Wrong edge definition.. id not in sequence, it should be %d\n", __func__, lineno, i);
				ret = -2;
				break;
			}
			if (src_id > 32767)
			{
				printf("[%s] line %lld src_id should in 0~32767, but got %d\n", __func__, lineno, src_id);
				ret = -2;
				break;
			}
			if (dest_id > 32767)
			{
				printf("[%s] line %lld dest_id should in 0~32767, but got %d\n", __func__, lineno, dest_id);
				ret = -2;
				break;
			}

			BT9.EDGE[i].src_id = (short)src_id;
			BT9.EDGE[i].dest_id = (short)dest_id;
			BT9.EDGE[i].taken = taken;
			BT9.EDGE[i].br_virt_target = br_virt_target;
			BT9.EDGE[i].br_phy_target = br_phy_target;
			BT9.EDGE[i].inst_cnt = inst_cnt;
			BT9.EDGE[i].traverse_cnt = traverse_cnt;
			BT9.BT9_EDGE_count++;
			break;
		case PROCESS_TRACE:
			printf("[%s] Error! Should not be here!!\n", __func__);
			break;
		}

		free(pt);
	}
	else
	{
		if (BT9.BT9_TRACE_count % (1024 * 1024) == 0)
		{
			// 重新扩充原来的大小，扩充1024*1024份记录
			BT9.TRACE = (UINT16 *)realloc(BT9.TRACE, ((BT9.BT9_TRACE_count / (1024 * 1024)) + 1) * 1024 * 1024 * sizeof(UINT16));
		}

		UINT16 TRACE_ID = (UINT16)atoi(linebuf);
		if (TRACE_ID == 0)
		{
			if (linebuf[0] == 'E' && linebuf[1] == 'O' && linebuf[2] == 'F')
				ret = 1;
		}
		BT9.TRACE[BT9.BT9_TRACE_count++] = TRACE_ID;
	}
	return ret;
}

int parse_bt9_file(char *filename)
{
	FILE *source;
	UINT64 lineno;
	int ret_get_line = 0;
	int ret_parse_line = 0;
	z_stream strm;

	printf("[%s] Begin parse BT9 file, please wait...\n", __func__);
#ifdef _WIN32
	errno_t err;
	err = fopen_s(&source, filename, "rb");
	if (err != 0)
	{
#else
	source = fopen(filename, "rb");
	if (source == NULL)
	{
#endif
		printf("[%s] Can't open file %s\n", __func__, filename);
		return -1;
	}
	lineno = 0;
	PROCESS_STATE = PROCESS_START;
	memset(&BT9, 0, sizeof(BT9));
	out_buf_ptr = NULL;
	memset(linebuf, 0, sizeof(linebuf));
	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret_get_line = inflateInit2(&strm, 15 + 32); // 15 window bits, and the +32 tells zlib to to detect if using gzip or zlib
	if (ret_get_line != Z_OK)
	{
		fclose(source);
		printf("[%s] zlib init fail...inflateInit2", __func__);
		return ret_get_line;
	}
	do
	{
		lineno++;
		ret_get_line = get_line_from_bt9(&strm, source); // 从文件中读取1行
		if (ret_get_line != Z_OK && ret_get_line != Z_STREAM_END)
			break;

		ret_parse_line = process_bt9_line(linebuf, lineno); // 分析处理1行
		if (ret_parse_line == 1)
			break;
	} while (1);

	fclose(source);
	inflateEnd(&strm);
	printf("[%s] End of parse BT9 file, total %lld lines parsed...\n", __func__, lineno);
	return 0;
}

void FreeBT9(void)
{
	if (BT9.BT9_NODE_count)
		free(BT9.NODE);
	if (BT9.BT9_EDGE_count)
		free(BT9.EDGE);
	if (BT9.BT9_TRACE_count)
		free(BT9.TRACE);
}

int SimBT9(struct BT9_struct *BT)
{
	UINT64 i;
	UINT64 PC;
	OpType optype;
	char predDir;
	char resolveDir;
	UINT64 branchTarget;
	UINT32 EDGE_id;
	struct BT9_EDGE *EDGE;
	struct BT9_NODE *src_NODE;
	UINT64 MISS_Count = 0LL;
	UINT64 cond_br_Count = 0LL;
	UINT64 uncond_br_Count = 0LL;

	PREDICTOR_init();

	// 跳过第0条，无效分支
	for (i = 1; i < BT->BT9_TRACE_count; i++)
	{
		EDGE_id = BT->TRACE[i];
		EDGE = &(BT->EDGE[EDGE_id]);
		src_NODE = &(BT->NODE[EDGE->src_id]);
		PC = src_NODE->virtual_address;
		resolveDir = EDGE->taken;
		branchTarget = EDGE->br_virt_target;
		optype = src_NODE->optype;

		// 仅仅处理条件跳转指令。注意：貌似测试集里面，只有OPTYPE_JMP_DIRECT_COND
		if (optype == OPTYPE_RET_COND || optype == OPTYPE_JMP_DIRECT_COND || optype == OPTYPE_JMP_INDIRECT_COND || optype == OPTYPE_CALL_DIRECT_COND || optype == OPTYPE_CALL_INDIRECT_COND)
		{
			cond_br_Count++;
			predDir = GetPrediction(PC);
			UpdatePredictor(PC, optype, resolveDir, predDir, branchTarget);
			if (resolveDir != predDir)
				MISS_Count++;
		}
		else
			uncond_br_Count++;
	}
	PREDICTOR_free();

	printf("  NUM_INSTRUCTIONS            \t : %10llu\n", BT->total_instruction_count);
	printf("  NUM_BR                      \t : %10llu\n", BT->branch_instruction_count - 1);
	printf("  NUM_UNCOND_BR               \t : %10llu\n", uncond_br_Count);
	printf("  NUM_CONDITIONAL_BR          \t : %10llu\n", cond_br_Count);
	printf("  NUM_MISPREDICTIONS          \t : %10llu\n", MISS_Count);
	printf("  MISPRED_PER_1K_INST         \t : %10.4f\n", 1000.0 * (double)(MISS_Count) / (double)(BT->total_instruction_count));
	return 0;
}