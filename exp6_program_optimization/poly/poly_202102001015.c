/**************************************************************************
	多项式计算函数。按下面的要求编辑此文件：
	1. 将你的学号、姓名，以注释的方式写到下面；
	2. 实现不同版本的多项式计算函数；
	3. 编辑peval_fun_rec peval_fun_tab数组，将你的最好的答案
		（最小CPE、最小C10）作为数组的前两项
***************************************************************************/

/*
	学号：202102001015
	姓名：侯华玮
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <x86intrin.h>
// #include <stdint.h>
typedef int (*peval_fun)(int *, int, int);

typedef struct
{
	peval_fun f;
	char *descr;
} peval_fun_rec, *peval_fun_ptr;

/**************************************************************************
 Edit this comment to indicate your name and Andrew ID
#ifdef ASSIGN
   Submission by Harry Q. Bovik, bovik@andrew.cmu.edu
#else
   Instructor's version.
   Created by Randal E. Bryant, Randy.Bryant@cs.cmu.edu, 10/07/02
#endif
***************************************************************************/

/*
	实现一个指定的常系数多项式计算
	第一次，请直接运行程序，以便获知你需要实现的常系数是啥
	result=68+34*x+91*x^2+65*x^3
*/
int const_poly_eval(int *not_use, int not_use2, int x)
{
	// register 效果比 no register 效果好
	register int x2 = x * x;
	register int p2 = (x2 << 7) - (x2 << 5) - (x2 << 2) - x2;
	register int x3 = x2 * x;
	register int p3 = (x3 << 6) + x3;
	return 68 + (x << 5) + (x << 1) + p2 + p3;
}

/* 多项式计算函数。注意：这个只是一个参考实现，你需要实现自己的版本 */

/*
	友情提示：lcc支持ATT格式的嵌入式汇编，例如

	_asm("movl %eax,%ebx");
	_asm("pushl %edx");

	可以在lcc中project->configuration->Compiler->Code Generation->Generate .asm，
	将其选中后，可以在lcc目录下面生成对应程序的汇编代码实现。通过查看汇编文件，
	你可以了解编译器是如何实现你的代码的。有些实现可能非常低效。
	你可以在适当的地方加入嵌入式汇编，来大幅度提高计算性能。
*/

int poly_eval(int *a, int degree, int x)
{
	int result = 0;
	int i;
	int xpwr = 1;
	for (i = 0; i <= degree; i++)
	{
		result += a[i] * xpwr;
		xpwr *= x;
	}
	return result;
}

int poly_eval_ext2(int *a, int degree, int x)
{
	register int result1 = 0;
	register int result2 = 0;
	register int i;
	register int xpwr1 = 1;
	register int xpwr2 = x;
	register int step = xpwr2 * x;
	register int limit = degree - 1;
	for (i = 0; i <= limit; i += 2)
	{
		result1 += a[i] * xpwr1;
		result2 += a[i + 1] * xpwr2;
		xpwr1 *= step;
		xpwr2 *= step;
	}
	result1 += result2;
	for (; i <= degree; i++)
	{
		result1 += a[i] * xpwr1;
		xpwr1 *= x;
	}
	return result1;
}

int poly_eval_ext3(int *a, int degree, int x)
{
	register int result1 = 0;
	register int result2 = 0;
	register int result3 = 0;
	register int i;
	register int xpwr1 = 1;
	register int xpwr2 = x;
	register int xpwr3 = x * x;
	register int step = xpwr3 * x;
	register int limit = degree - 2;
	for (i = 0; i <= limit; i += 3)
	{
		result1 += a[i] * xpwr1;
		result2 += a[i + 1] * xpwr2;
		result3 += a[i + 2] * xpwr3;
		xpwr1 *= step;
		xpwr2 *= step;
		xpwr3 *= step;
	}
	result1 += result2;
	result1 += result3;
	for (; i <= degree; i++)
	{
		result1 += a[i] * xpwr1;
		xpwr1 *= x;
	}
	return result1;
}

int poly_eval_ext4(int *a, int degree, int x)
{
	register int result1 = 0;
	register int result2 = 0;
	register int result3 = 0;
	register int result4 = 0;
	register int i;
	register int xpwr1 = 1;
	register int xpwr2 = x;
	register int xpwr3 = x * x;
	register int xpwr4 = xpwr3 * x;
	register int step = xpwr3 * xpwr3;
	register int limit = degree - 3;
	for (i = 0; i <= limit; i += 4)
	{
		result1 += a[i] * xpwr1;
		result2 += a[i + 1] * xpwr2;
		result3 += a[i + 2] * xpwr3;
		result4 += a[i + 3] * xpwr4;
		xpwr1 *= step;
		xpwr2 *= step;
		xpwr3 *= step;
		xpwr4 *= step;
	}
	// result1 += result2;
	// result1 += result3;
	// result1 += result4;
	result1 += result2;
	result3 += result4;
	result1 += result3;
	for (; i <= degree; i++)
	{
		result1 += a[i] * xpwr1;
		xpwr1 *= x;
	}
	return result1;
}
int poly_eval_ext5(int *a, int degree, int x)
{
	register int result1 = 0;
	register int result2 = 0;
	register int result3 = 0;
	register int result4 = 0;
	register int result5 = 0;
	register int i;
	register int xpwr1 = 1;
	register int xpwr2 = x;
	register int xpwr3 = x * x;
	register int xpwr4 = xpwr3 * x;
	register int xpwr5 = xpwr4 * x;
	register int step = xpwr3 * xpwr4;
	register int limit = degree - 4;
	for (i = 0; i <= limit; i += 5)
	{
		result1 += a[i] * xpwr1;
		result2 += a[i + 1] * xpwr2;
		result3 += a[i + 2] * xpwr3;
		result4 += a[i + 3] * xpwr4;
		result5 += a[i + 4] * xpwr5;
		xpwr1 *= step;
		xpwr2 *= step;
		xpwr3 *= step;
		xpwr4 *= step;
		xpwr5 *= step;
	}
	result1 += result2;
	result1 += result3;
	result1 += result4;
	result1 += result5;

	for (; i <= degree; i++)
	{
		result1 += a[i] * xpwr1;
		xpwr1 *= x;
	}
	return result1;
}
int poly_eval_ext5_no(int *a, int degree, int x)
{
	int result1 = 0;
	int result2 = 0;
	int result3 = 0;
	int result4 = 0;
	int result5 = 0;
	int i;
	int xpwr1 = 1;
	int xpwr2 = x;
	int xpwr3 = x * x;
	int xpwr4 = xpwr3 * x;
	int xpwr5 = xpwr4 * x;
	int step = xpwr3 * xpwr4;
	int limit = degree - 4;
	for (i = 0; i <= limit; i += 5)
	{
		result1 += a[i] * xpwr1;
		result2 += a[i + 1] * xpwr2;
		result3 += a[i + 2] * xpwr3;
		result4 += a[i + 3] * xpwr4;
		result5 += a[i + 4] * xpwr5;
		xpwr1 *= step;
		xpwr2 *= step;
		xpwr3 *= step;
		xpwr4 *= step;
		xpwr5 *= step;
	}
	// result1 += result2;
	// result1 += result3;
	// result1 += result4;
	// result1 += result5;
	result1 += result2;
	result3 += result4;
	result1 += result3;
	result1 += result5;
	for (; i <= degree; i++)
	{
		result1 += a[i] * xpwr1;
		xpwr1 *= x;
	}
	return result1;
}
int poly_eval_ext10(int *a, int degree, int x)
{
	register int result1 = 0;
	register int result2 = 0;
	register int result3 = 0;
	register int result4 = 0;
	register int result5 = 0;
	register int result6 = 0;
	register int result7 = 0;
	register int result8 = 0;
	register int result9 = 0;
	register int result10 = 0;
	register int i;
	register int xpwr1 = 1;
	register int xpwr2 = x;
	register int xpwr3 = x * x;
	register int xpwr4 = xpwr3 * x;
	register int xpwr5 = xpwr4 * x;
	register int xpwr6 = xpwr5 * x;
	register int xpwr7 = xpwr6 * x;
	register int xpwr8 = xpwr7 * x;
	register int xpwr9 = xpwr8 * x;
	register int xpwr10 = xpwr9 * x;
	register int step = xpwr10 * x;
	register int limit = degree - 9;
	for (i = 0; i <= limit; i += 10)
	{
		result1 += a[i] * xpwr1;
		result2 += a[i + 1] * xpwr2;
		result3 += a[i + 2] * xpwr3;
		result4 += a[i + 3] * xpwr4;
		result5 += a[i + 4] * xpwr5;
		result6 += a[i + 5] * xpwr6;
		result7 += a[i + 6] * xpwr7;
		result8 += a[i + 7] * xpwr8;
		result9 += a[i + 8] * xpwr9;
		result10 += a[i + 9] * xpwr10;
		xpwr1 *= step;
		xpwr2 *= step;
		xpwr3 *= step;
		xpwr4 *= step;
		xpwr5 *= step;
		xpwr6 *= step;
		xpwr7 *= step;
		xpwr8 *= step;
		xpwr9 *= step;
		xpwr10 *= step;
	}
	result1 += result2;
	result1 += result3;
	result1 += result4;
	result1 += result5;
	result1 += result6;
	result1 += result7;
	result1 += result8;
	result1 += result9;
	result1 += result10;

	for (; i <= degree; i++)
	{
		result1 += a[i] * xpwr1;
		xpwr1 *= x;
	}
	return result1;
}

/*
	这个表格包含多个数组元素，每一组元素（函数名字, "描述字符串"）
	将你认为最好的两个实现，放在最前面。
	比如：
	{my_poly_eval1, "超级垃圾实现"},
	{my_poly_eval2, "好一点的实现"},
*/

peval_fun_rec peval_fun_tab[] = {

	/* 第一项，应当是你写的最好CPE的函数实现 */
	{poly_eval_ext3, "poly_eval_ext3 的循环展开"},
	/* 第二项，应当是你写的在10阶时具有最好性能的实现 */
	{poly_eval_ext2, "poly_eval_ext2 的循环展开"},
	{poly_eval_ext4, "poly_eval_ext4 的循环展开"},
	{poly_eval_ext5, "poly_eval_ext5 的循环展开"},
	{poly_eval_ext5_no, "poly_eval_ext5_no 的循环展开"},
	{poly_eval_ext10, "poly_eval_ext10 的循环展开"},
	/* 下面的代码不能修改或者删除！！表明数组列表结束 */
	{NULL, ""}

};
