/**************************************************************************
	行/列求和函数。按下面的要求编辑此文件：
	1. 将你的学号、姓名，以注释的方式写到下面；
	2. 实现不同版本的行列求和函数；
	3. 编辑rc_fun_rec rc_fun_tab数组，将你的最好的答案
		（最好的行和列求和、最好的列求和）作为数组的前两项
***************************************************************************/

/*
	学号：202102001015
	姓名：侯华玮
*/

#include <stdio.h>
#include <stdlib.h>
#include "rowcol.h"
#include <math.h>
#include <x86intrin.h>

/* 参考的列求和函数实现 */
/* 计算矩阵中的每一列的和。请注意对于行和列求和来说，调用参数是
	一样的，只是第2个参数不会用到而已
*/

__m256i x;
void c_sum(matrix_t M, vector_t rowsum, vector_t colsum)
{
	int i, j;
	memset(colsum, 0, 512 * sizeof(int));
	for (j = 0; j < N; j++)
	{
		for (i = 0; i < N; i++)
			colsum[j] += M[i][j];
	}
}

void c_sum_f(matrix_t M, vector_t rowsum, vector_t colsum)
{
	register int i, j;
	memset(colsum, 0, 512 * sizeof(int));
	for (i = 0; i < N; i++)
	{
		for (j = 0; j < N; j++)
			colsum[j] += M[i][j];
	}
}
void c_sum_f_ext(matrix_t M, vector_t rowsum, vector_t colsum)
{
	register int i, j;
	register int sum;
	register int limit = N - 7;
	// memset(colsum, 0, 512*sizeof(int));
	for (i = 0; i < limit; i += 8)
	{
		colsum[i] = 0;
		colsum[i + 1] = 0;
		colsum[i + 2] = 0;
		colsum[i + 3] = 0;
		colsum[i + 4] = 0;
		colsum[i + 5] = 0;
		colsum[i + 6] = 0;
		colsum[i + 7] = 0;
	}
	// register long int *p = (long int *)colsum;
	// for(i = 0; i < N; i+=8){
	// 	*p = 0;
	// 	p++;
	// }
	for (i = 0; i < N; i++)
	{
		for (j = 0; j < limit; j += 8)
		{
			colsum[j] += M[i][j];
			colsum[j + 1] += M[i][j + 1];
			colsum[j + 2] += M[i][j + 2];
			colsum[j + 3] += M[i][j + 3];
			colsum[j + 4] += M[i][j + 4];
			colsum[j + 5] += M[i][j + 5];
			colsum[j + 6] += M[i][j + 6];
			colsum[j + 7] += M[i][j + 7];
		}
	}
}
// void c_sum_f(matrix_t M, vector_t rowsum, vector_t colsum){
// 	int i, j;
// 	memset(colsum, 0, 512*sizeof(int));
// 	register int sum;
// 	register int limit = N - 3;
// 	for(i = 0; i < limit; i += 4){
// 		for(j = 0; j < N; j++){
// 			colsum[j] += M[i][j];
// 		}
// 		for(j = 0; j < N; j++){
// 			colsum[j] += M[i+1][j];
// 		}
// 		for(j = 0; j < N; j++){
// 			colsum[j] += M[i+2][j];
// 		}
// 		for(j = 0; j < N; j++){
// 			colsum[j] += M[i+3][j];
// 		}
// 	}
// }

void c_sum_jubu2(matrix_t M, vector_t rowsum, vector_t colsum)
{
	register int i, j;
	register int sum1, sum2;
	for (j = 0; j < N - 1; j += 2)
	{
		sum1 = sum2 = 0;
		for (i = 0; i < N; i++)
		{
			sum1 += M[i][j];
			sum2 += M[i][j + 1];
		}
		colsum[j] = sum1;
		colsum[j + 1] = sum2;
	}
}
// 512//4
void c_sum_jubu4(matrix_t M, vector_t rowsum, vector_t colsum)
{
	register int i, j;
	register int sum1, sum2, sum3, sum4;
	register int limit = N - 3;
	for (j = 0; j < limit; j += 4)
	{
		sum1 = sum2 = sum3 = sum4 = 0;
		for (i = 0; i < N; i++)
		{
			sum1 += M[i][j];
			sum2 += M[i][j + 1];
			sum3 += M[i][j + 2];
			sum4 += M[i][j + 3];
		}
		colsum[j] = sum1;
		colsum[j + 1] = sum2;
		colsum[j + 2] = sum3;
		colsum[j + 3] = sum4;
	}
}

void c_sum_jubu8(matrix_t M, vector_t rowsum, vector_t colsum)
{
	register int i, j;
	register int sum1, sum2, sum3, sum4, sum5, sum6, sum7, sum8;
	register int limit = N - 7;
	for (j = 0; j < limit; j += 8)
	{
		sum1 = sum2 = sum3 = sum4 = sum5 = sum6 = sum7 = sum8 = 0;
		for (i = 0; i < N; i++)
		{
			sum1 += M[i][j];
			sum2 += M[i][j + 1];
			sum3 += M[i][j + 2];
			sum4 += M[i][j + 3];
			sum5 += M[i][j + 4];
			sum6 += M[i][j + 5];
			sum7 += M[i][j + 6];
			sum8 += M[i][j + 7];
		}
		colsum[j] = sum1;
		colsum[j + 1] = sum2;
		colsum[j + 2] = sum3;
		colsum[j + 3] = sum4;
		colsum[j + 4] = sum5;
		colsum[j + 5] = sum6;
		colsum[j + 6] = sum7;
		colsum[j + 7] = sum8;
	}
}

// void c_sum_jubu8

void c_sum_jubu16(matrix_t M, vector_t rowsum, vector_t colsum)
{
	register int i, j;
	register int sum1, sum2, sum3, sum4, sum5, sum6, sum7, sum8;
	register int sum9, sum10, sum11, sum12, sum13, sum14, sum15, sum16;
	register int limit = N - 15;
	for (j = 0; j < limit; j += 16)
	{
		sum1 = sum2 = sum3 = sum4 = sum5 = sum6 = sum7 = sum8 = 0;
		sum9 = sum10 = sum11 = sum12 = sum13 = sum14 = sum15 = sum16 = 0;
		for (i = 0; i < N; i++)
		{
			sum1 += M[i][j];
			sum2 += M[i][j + 1];
			sum3 += M[i][j + 2];
			sum4 += M[i][j + 3];
			sum5 += M[i][j + 4];
			sum6 += M[i][j + 5];
			sum7 += M[i][j + 6];
			sum8 += M[i][j + 7];
			sum9 += M[i][j + 8];
			sum10 += M[i][j + 9];
			sum11 += M[i][j + 10];
			sum12 += M[i][j + 11];
			sum13 += M[i][j + 12];
			sum14 += M[i][j + 13];
			sum15 += M[i][j + 14];
			sum16 += M[i][j + 15];
		}
		colsum[j] = sum1;
		colsum[j + 1] = sum2;
		colsum[j + 2] = sum3;
		colsum[j + 3] = sum4;
		colsum[j + 4] = sum5;
		colsum[j + 5] = sum6;
		colsum[j + 6] = sum7;
		colsum[j + 7] = sum8;
		colsum[j + 8] = sum9;
		colsum[j + 9] = sum10;
		colsum[j + 10] = sum11;
		colsum[j + 11] = sum12;
		colsum[j + 12] = sum13;
		colsum[j + 13] = sum14;
		colsum[j + 14] = sum15;
		colsum[j + 15] = sum16;
	}
}

void c_sum_jubu32(matrix_t M, vector_t rowsum, vector_t colsum)
{
	register int i, j;
	register int sum1, sum2, sum3, sum4, sum5, sum6, sum7, sum8;
	register int sum9, sum10, sum11, sum12, sum13, sum14, sum15, sum16;
	register int sum17, sum18, sum19, sum20, sum21, sum22, sum23, sum24;
	register int sum25, sum26, sum27, sum28, sum29, sum30, sum31, sum32;
	register int limit = N - 31;
	for (j = 0; j < limit; j += 32)
	{
		sum1 = sum2 = sum3 = sum4 = sum5 = sum6 = sum7 = sum8 = 0;
		sum9 = sum10 = sum11 = sum12 = sum13 = sum14 = sum15 = sum16 = 0;
		sum17 = sum18 = sum19 = sum20 = sum21 = sum22 = sum23 = sum24 = 0;
		sum25 = sum26 = sum27 = sum28 = sum29 = sum30 = sum31 = sum32 = 0;
		for (i = 0; i < N; i++)
		{
			sum1 += M[i][j];
			sum2 += M[i][j + 1];
			sum3 += M[i][j + 2];
			sum4 += M[i][j + 3];
			sum5 += M[i][j + 4];
			sum6 += M[i][j + 5];
			sum7 += M[i][j + 6];
			sum8 += M[i][j + 7];
			sum9 += M[i][j + 8];
			sum10 += M[i][j + 9];
			sum11 += M[i][j + 10];
			sum12 += M[i][j + 11];
			sum13 += M[i][j + 12];
			sum14 += M[i][j + 13];
			sum15 += M[i][j + 14];
			sum16 += M[i][j + 15];
			sum17 += M[i][j + 16];
			sum18 += M[i][j + 17];
			sum19 += M[i][j + 18];
			sum20 += M[i][j + 19];
			sum21 += M[i][j + 20];
			sum22 += M[i][j + 21];
			sum23 += M[i][j + 22];
			sum24 += M[i][j + 23];
			sum25 += M[i][j + 24];
			sum26 += M[i][j + 25];
			sum27 += M[i][j + 26];
			sum28 += M[i][j + 27];
			sum29 += M[i][j + 28];
			sum30 += M[i][j + 29];
			sum31 += M[i][j + 30];
			sum32 += M[i][j + 31];
		}
		colsum[j] = sum1;
		colsum[j + 1] = sum2;
		colsum[j + 2] = sum3;
		colsum[j + 3] = sum4;
		colsum[j + 4] = sum5;
		colsum[j + 5] = sum6;
		colsum[j + 6] = sum7;
		colsum[j + 7] = sum8;
		colsum[j + 8] = sum9;
		colsum[j + 9] = sum10;
		colsum[j + 10] = sum11;
		colsum[j + 11] = sum12;
		colsum[j + 12] = sum13;
		colsum[j + 13] = sum14;
		colsum[j + 14] = sum15;
		colsum[j + 15] = sum16;
		colsum[j + 16] = sum17;
		colsum[j + 17] = sum18;
		colsum[j + 18] = sum19;
		colsum[j + 19] = sum20;
		colsum[j + 20] = sum21;
		colsum[j + 21] = sum22;
		colsum[j + 22] = sum23;
		colsum[j + 23] = sum24;
		colsum[j + 24] = sum25;
		colsum[j + 25] = sum26;
		colsum[j + 26] = sum27;
		colsum[j + 27] = sum28;
		colsum[j + 28] = sum29;
		colsum[j + 29] = sum30;
		colsum[j + 30] = sum31;
		colsum[j + 31] = sum32;
	}
}
void c_sum_ext2(matrix_t M, vector_t rowsum, vector_t colsum)
{
	register int i, j;
	register int sum1, sum2;
	register int limit = N - 1;
	for (j = 0; j < N; j++)
	{
		sum1 = sum2 = 0;
		for (i = 0; i < limit; i += 2)
		{
			sum1 += M[i][j];
			sum2 += M[i + 1][j];
		}
		for (; i < N; i++)
			sum1 += M[i][j];
		colsum[j] = sum1 + sum2;
	}
}

void c_sum_ext3(matrix_t M, vector_t rowsum, vector_t colsum)
{
	register int i, j;
	register int sum1, sum2, sum3;
	register int limit = N - 2;
	for (j = 0; j < N; j++)
	{
		sum1 = sum2 = sum3 = 0;
		for (i = 0; i < limit; i += 3)
		{
			sum1 += M[i][j];
			sum2 += M[i + 1][j];
			sum3 += M[i + 2][j];
		}
		for (; i < N; i++)
			sum1 += M[i][j];
		colsum[j] = sum1 + sum2 + sum3;
	}
}

void c_sum_ext4(matrix_t M, vector_t rowsum, vector_t colsum)
{
	register int i, j;
	register int sum1, sum2, sum3, sum4;
	register int limit = N - 3;
	for (j = 0; j < N; j++)
	{
		sum1 = sum2 = sum3 = sum4 = 0;
		for (i = 0; i < limit; i += 4)
		{
			sum1 += M[i][j];
			sum2 += M[i + 1][j];
			sum3 += M[i + 2][j];
			sum4 += M[i + 3][j];
		}
		for (; i < N; i++)
			sum1 += M[i][j];
		colsum[j] = sum1 + sum2 + sum3 + sum4;
	}
}
/* 参考的列和行求和函数实现 */
/* 计算矩阵中的每一行、每一列的和。 */

void rc_sum(matrix_t M, vector_t rowsum, vector_t colsum)
{
	int i, j;
	for (i = 0; i < N; i++)
	{
		rowsum[i] = colsum[i] = 0;
		for (j = 0; j < N; j++)
		{
			rowsum[i] += M[i][j];
			colsum[i] += M[j][i];
		}
	}
}

void rc_sum_jubu16(matrix_t M, vector_t rowsum, vector_t colsum)
{
	register int i, j;
	register int sum1, sum2, sum3, sum4, sum5, sum6, sum7, sum8;
	register int sum9, sum10, sum11, sum12, sum13, sum14, sum15, sum16;
	register int limit = N - 15;
	for (j = 0; j < limit; j += 16)
	{
		sum1 = sum2 = sum3 = sum4 = sum5 = sum6 = sum7 = sum8 = 0;
		sum9 = sum10 = sum11 = sum12 = sum13 = sum14 = sum15 = sum16 = 0;
		for (i = 0; i < N; i++)
		{
			sum1 += M[i][j];
			sum2 += M[i][j + 1];
			sum3 += M[i][j + 2];
			sum4 += M[i][j + 3];
			sum5 += M[i][j + 4];
			sum6 += M[i][j + 5];
			sum7 += M[i][j + 6];
			sum8 += M[i][j + 7];
			sum9 += M[i][j + 8];
			sum10 += M[i][j + 9];
			sum11 += M[i][j + 10];
			sum12 += M[i][j + 11];
			sum13 += M[i][j + 12];
			sum14 += M[i][j + 13];
			sum15 += M[i][j + 14];
			sum16 += M[i][j + 15];
		}
		colsum[j] = sum1;
		colsum[j + 1] = sum2;
		colsum[j + 2] = sum3;
		colsum[j + 3] = sum4;
		colsum[j + 4] = sum5;
		colsum[j + 5] = sum6;
		colsum[j + 6] = sum7;
		colsum[j + 7] = sum8;
		colsum[j + 8] = sum9;
		colsum[j + 9] = sum10;
		colsum[j + 10] = sum11;
		colsum[j + 11] = sum12;
		colsum[j + 12] = sum13;
		colsum[j + 13] = sum14;
		colsum[j + 14] = sum15;
		colsum[j + 15] = sum16;
	}
	for (i = 0; i < N; i++)
	{
		sum1 = sum2 = sum3 = sum4 = sum5 = sum6 = sum7 = sum8 = 0;
		sum9 = sum10 = sum11 = sum12 = sum13 = sum14 = sum15 = sum16 = 0;
		for (j = 0; j < limit; j += 16)
		{
			sum1 += M[i][j];
			sum2 += M[i][j + 1];
			sum3 += M[i][j + 2];
			sum4 += M[i][j + 3];
			sum5 += M[i][j + 4];
			sum6 += M[i][j + 5];
			sum7 += M[i][j + 6];
			sum8 += M[i][j + 7];
			sum9 += M[i][j + 8];
			sum10 += M[i][j + 9];
			sum11 += M[i][j + 10];
			sum12 += M[i][j + 11];
			sum13 += M[i][j + 12];
			sum14 += M[i][j + 13];
			sum15 += M[i][j + 14];
			sum16 += M[i][j + 15];
		}
		rowsum[i] = sum1 + sum2 + sum3 + sum4 + sum5 + sum6 + sum7 + sum8 + sum9 + sum10 + sum11 + sum12 + sum13 + sum14 + sum15 + sum16;
	}
}
void rc_sum_jubu8(matrix_t M, vector_t rowsum, vector_t colsum)
{
	register int i, j;
	register int sum1, sum2, sum3, sum4, sum5, sum6, sum7, sum8;
	register int limit = N - 7;
	for (j = 0; j < limit; j += 8)
	{
		sum1 = sum2 = sum3 = sum4 = sum5 = sum6 = sum7 = sum8 = 0;
		for (i = 0; i < N; i++)
		{
			sum1 += M[i][j];
			sum2 += M[i][j + 1];
			sum3 += M[i][j + 2];
			sum4 += M[i][j + 3];
			sum5 += M[i][j + 4];
			sum6 += M[i][j + 5];
			sum7 += M[i][j + 6];
			sum8 += M[i][j + 7];
		}
		colsum[j] = sum1;
		colsum[j + 1] = sum2;
		colsum[j + 2] = sum3;
		colsum[j + 3] = sum4;
		colsum[j + 4] = sum5;
		colsum[j + 5] = sum6;
		colsum[j + 6] = sum7;
		colsum[j + 7] = sum8;
	}

	for (i = 0; i < N; i++)
	{
		sum1 = sum2 = sum3 = sum4 = sum5 = sum6 = sum7 = sum8 = 0;
		for (j = 0; j < limit; j += 8)
		{
			sum1 += M[i][j];
			sum2 += M[i][j + 1];
			sum3 += M[i][j + 2];
			sum4 += M[i][j + 3];
			sum5 += M[i][j + 4];
			sum6 += M[i][j + 5];
			sum7 += M[i][j + 6];
			sum8 += M[i][j + 7];
		}
		rowsum[i] = sum1 + sum2 + sum3 + sum4 + sum5 + sum6 + sum7 + sum8;
	}
}
void rc_sum_jubu4(matrix_t M, vector_t rowsum, vector_t colsum)
{
	register int i, j;
	register int sum1, sum2, sum3, sum4;
	register int limit = N - 3;
	for (j = 0; j < limit; j += 4)
	{
		sum1 = sum2 = sum3 = sum4 = 0;
		for (i = 0; i < N; i++)
		{
			sum1 += M[i][j];
			sum2 += M[i][j + 1];
			sum3 += M[i][j + 2];
			sum4 += M[i][j + 3];
		}
		colsum[j] = sum1;
		colsum[j + 1] = sum2;
		colsum[j + 2] = sum3;
		colsum[j + 3] = sum4;
	}
}
void rc_sum_ext2(matrix_t M, vector_t rowsum, vector_t colsum)
{
	register int i, j;
	register int rsum1, rsum2, csum1, csum2;
	// register int limit = N - 1;
	for (i = 0; i < N; i++)
	{
		rsum1 = rsum2 = csum1 = csum2 = 0;
		for (j = 0; j < N - 1; j += 2)
		{
			rsum1 += M[i][j];
			rsum2 += M[i][j + 1];
			csum1 += M[j][i];
			csum2 += M[j + 1][i];
		}
		for (; j < N; j++)
		{
			rsum1 += M[i][j];
			csum1 += M[j][i];
		}
		rowsum[i] = rsum1 + rsum2;
		colsum[i] = csum1 + csum2;
	}
}

void rc_sum_ext4(matrix_t M, vector_t rowsum, vector_t colsum)
{
	register int i, j;
	register int rsum1, rsum2, rsum3, rsum4, csum1, csum2, csum3, csum4;
	// register int limit = N - 3;
	for (i = 0; i < N; i++)
	{
		rsum1 = rsum2 = rsum3 = rsum4 = csum1 = csum2 = csum3 = csum4 = 0;
		for (j = 0; j < N - 3; j += 4)
		{
			rsum1 += M[i][j];
			rsum2 += M[i][j + 1];
			rsum3 += M[i][j + 2];
			rsum4 += M[i][j + 3];
			csum1 += M[j][i];
			csum2 += M[j + 1][i];
			csum3 += M[j + 2][i];
			csum4 += M[j + 3][i];
		}
		for (; j < N; j++)
		{
			rsum1 += M[i][j];
			csum1 += M[j][i];
		}
		rowsum[i] = rsum1 + rsum2 + rsum3 + rsum4;
		colsum[i] = csum1 + csum2 + csum3 + csum4;
	}
}

/*
	这个表格包含多个数组元素，每一组元素（函数名字, COL/ROWCOL, "描述字符串"）
	COL表示该函数仅仅计算每一列的和
	ROWCOL表示该函数计算每一行、每一列的和
	将你认为最好的两个实现，放在最前面。
	比如：
	{my_c_sum1, "超级垃圾列求和实现"},
	{my_rc_sum2, "好一点的行列求和实现"},
*/

rc_fun_rec rc_fun_tab[] =
	{

		/* 第一项，应当是你写的最好列求和的函数实现 */
		// {c_sum_ext3, COL, "c_sum_ext3"},
		{c_sum_jubu32, COL, "c_sum_jubu32"},
		{rc_sum_jubu16, ROWCOL, "rc_sum_jubu16"},
		{c_sum_jubu16, COL, "c_sum_jubu16"},
		{rc_sum_jubu8, ROWCOL, "rc_sum_jubu8"},
		// {c_sum_jubu2, COL, "c_sum_jubu2"},
		// {c_sum_ext4, COL, "c_sum_ext4"},
		// {rc_sum_test, ROWCOL, "rc_sum_test"},
		// {rc_sum, ROWCOL, "Best row and column sum"},

		// {c_sum_ext2, COL, "c_sum_ext2"},
		// {c_sum, COL, "Best column sum"},
		// /* 第二项，应当是你写的最好行列求和的函数实现 */
		// {rc_sum, ROWCOL, "Best row and column sum"},

		// {c_sum, COL, "Column sum, reference implementation"},

		// {rc_sum, ROWCOL, "Row and column sum, reference implementation"},

		/* 下面的代码不能修改或者删除！！表明数组列表结束 */
		{NULL, ROWCOL, NULL}};
