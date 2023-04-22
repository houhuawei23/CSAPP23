#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef __linux__
#define _alloca alloca
#endif

int getbuf(void);
void test(void);

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

/* 输入16进制字符串，并转换为对应的字符串，以\0结束 */
char*getxs(char*dest)
{
  int c;
  int even =1; /* Have read even number of digits */
  int otherd =0; /* Other hex digit of pair */
  char*sp = dest;
  while ((c = getchar()) != EOF && c !='\n' && c != '\r') {
    if (isxdigit(c)) {
      int val;
      if ('0'<= c && c <='9')
        val = c -'0';
      else if ('A'<= c && c <='F')
        val = c -'A'+10;
      else
        val = c -'a'+10;
      if (even) {
        otherd = val;
        even =0;
      }
      else {
        *sp++= otherd *16+ val;
        even =1;
      }
    }
  }
  *sp++='\0';
  return dest;
}

/* 获取一行输入字符串 */
int getbuf(void)
{
	char buf[12];
	getxs(buf);
	return 1;
}

/* 主测试程序 */
void test(void)
{
	int val;
	char *localbuf;
	volatile int bird = 0xdeadbeef;	//金丝雀保护机制
	GenerateRandomNumber(23);
	localbuf = (char *)_alloca(rand_div+1);	//在栈上分配随机空间
	localbuf[0] = 'l';

	val = getbuf();
	/* 检测是否栈被破坏 */
	if (bird == 0xdeadbeef) {
		printf("鸟还活着！\n");
	}
	else
		printf("不妙！鸟被杀死，栈已经被你破坏了！\n");
	
	if (val == cookie) {
		printf("不错哦，缓冲区溢出成功，而且getbuf返回 0X%08X\n", val);
	}
	else if (val == 1) {
		printf("缓冲区没有溢出.....攻击失败，请重来吧\n");
	}
	else {
		printf("不对哦，虽然缓冲区溢出成功，但是getbuf返回 0X%08X\n", val);
	}
}

/* 第1只木马，只需要修改返回地址，即可进入 */
void Trojan1(void)
{
	printf("恭喜你！你已经成功偷偷运行了第1只木马!\n");
	printf("通过第1只木马测试\n");
	exit(0);
}

/* 第2只木马，不仅需要修改返回地址，而且要修改栈中返回的结果 */
void Trojan2(int val)
{
	if (val == cookie) {
		printf("不错哦！第2只木马运行了，而且通行密码是正确的！(0X%08X)\n", val);
	} else
		printf("需要加油！虽然第2只木马运行了，但是通行密码是不正确的！(0X%08X)\n", val);
	if (val == cookie)
		printf("通过第2只木马测试\n");
	exit(0);
}

/* 第3只木马，本关任务是构造特定的机器代码放置在栈内，然后将返回地址置为该段特定代码的入口。此段代码负责将global_value设置为想要的cookie值 */
/* 汇编指令程序：
   MOV EAX,cookie
   MOV global_val,EAX
   PUSH Trojan3
   RET
   
   0:   a1 e4 c1 04 08          MOV    EAX,0x804c1e4
   5:   a3 ec c1 04 08          MOV    0x804c1ec,EAX
   a:   68 eb 8c 04 08          PUSH   $0x8048ceb
   f:   c3                      RET
*/
int global_value = 0;
void Trojan3(int val)
{
	if (global_value == cookie) {
		printf("厉害！第3只木马运行了，而且你修改了全局变量正确！global_value = 0X%08X\n", global_value);
	} else
		printf("差一点！第3只木马运行了，但是全局变量修改错误！global_value = 0X%08X\n", global_value);
	if (global_value == cookie)
		printf("通过第3只木马测试\n");
	exit(0);
}

/* 第4只木马，本关任务是构造特定的机器代码放置在栈内，然后将返回地址置为该段特定代码的入口。此段代码负责将global_value设置为想要的cookie值，需要正常返回 */
/* 汇编指令程序：
   MOV EAX,cookie
   MOV global_val,EAX
   PUSH Trojan3
   RET
   
   0:   a1 e4 c1 04 08          MOV    EAX,0x804c1e4
   5:   a3 ec c1 04 08          MOV    0x804c1ec,EAX
   a:   68 eb 8c 04 08          PUSH   $0x8048ceb
   f:   c3                      RET
*/
void Trojan4(int val)
{
	if (global_value == cookie) {
		printf("厉害！第4只木马运行了，而且你修改了全局变量正确！global_value = 0X%08X\n", global_value);
	} else
		printf("差一点！第4只木马运行了，但是全局变量不对！global_value = 0X%08X\n", global_value);
	if (global_value == cookie)
		printf("通过第4只木马测试\n");
	return;	// 正常返回，需要修复栈
}

/* 主程序，依据学号，随机生成cookie值 */
int main(int argc, char *argv[])
{
	int i;
	char *MyRandomBuffer;
	printf("\t2018超级缓冲区炸弹，欢迎你！\n");
	printf("============================\n");
	
	if (argc == 1)
	{
		printf("使用方法：%s 学号后6位 [学号后6位] [学号后6位] ...\n",argv[0]);
		printf("你需要输入攻击字符串，以便种入木马，一旦出错，哇哈哈....\n");
		printf("请以十六进制形式输入攻击字符串，例如00 aa bb cc等等\n");
		return 0;
	}
	
	printf("欢迎你前来挑战！ %s \n",argv[1]);
	/*依据学号，初始化一个随机数发生器*/
	rand1_h = (unsigned long)atoi(argv[1]);
	rand1_l=0x29A;
	GenerateRandomNumber(0);
	for (i=2;i<argc;i++)
	{
		rand1_l = (unsigned long)atoi(argv[i]);
		GenerateRandomNumber(0);
	}
	
	cookie = (int)rand1_h;
	printf("你的通行密码是0X%08X\n",cookie);
	printf("============================\n");
	printf("请输入攻击字符串（十六进制串）：");
	GenerateRandomNumber(512);
	MyRandomBuffer = (char *)_alloca(rand_div+1);	//在栈上分配随机空间
	MyRandomBuffer[0] = 'h';
	test();

	return 0;
}
