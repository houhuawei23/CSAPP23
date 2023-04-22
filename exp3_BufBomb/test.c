#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int cookie=0x11223344;
unsigned long rand1_h,rand1_l,rand_div;

/* 产生一个0~divv-1之间的随机数，同时更新随机数种子 */
void GenerateRandomNumber(unsigned long divv)
{
	long long x = rand1_h;
	x *= 0x6AC690C5;
	x += rand1_l;
	
	rand1_h = (unsigned long)x;
	rand1_l = (unsigned long)(x>>32);
	if (divv==0) return;
	
	rand_div = rand1_h % divv;
}

int main(int argc, char *argv[]){
    int i;
	char *MyRandomBuffer;

	/*依据学号，初始化一个随机数发生器*/
	rand1_h = (unsigned long)atoi(argv[1]);
	rand1_l=0x29A;
	GenerateRandomNumber(0);

	for (i=2;i<argc;i++)
	{
		rand1_l = (unsigned long)atoi(argv[i]);
		GenerateRandomNumber(0);
	}

    printf("rand1_h: %ld, \nrand2_l: %ld, \nrand1_div: %ld\n", rand1_h,rand1_l,rand_div);

	cookie = (int)rand1_h;
	printf("你的通行密码是0X%08X\n",cookie);
	printf("============================\n");
	printf("请输入攻击字符串（十六进制串）：");
	GenerateRandomNumber(512);
	MyRandomBuffer = (char *)_alloca(rand_div+1);	//在栈上分配随机空间
	MyRandomBuffer[0] = 'h';
    printf("\nrand1_h: %ld, \nrand2_l: %ld, \nrand1_div: %ld\n", rand1_h,rand1_l,rand_div);

    GenerateRandomNumber(23);
    printf("\nrand1_h: %ld, \nrand2_l: %ld, \nrand1_div: %ld\n", rand1_h,rand1_l,rand_div);
}