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

/* ����һ��0~divv-1֮����������ͬʱ������������� */
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

/* ����16�����ַ�������ת��Ϊ��Ӧ���ַ�������\0���� */
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

/* ��ȡһ�������ַ��� */
int getbuf(void)
{
	char buf[12];
	getxs(buf);
	return 1;
}

/* �����Գ��� */
void test(void)
{
	int val;
	char *localbuf;
	volatile int bird = 0xdeadbeef;	//��˿ȸ��������
	GenerateRandomNumber(23);
	localbuf = (char *)_alloca(rand_div+1);	//��ջ�Ϸ�������ռ�
	localbuf[0] = 'l';

	val = getbuf();
	/* ����Ƿ�ջ���ƻ� */
	if (bird == 0xdeadbeef) {
		printf("�񻹻��ţ�\n");
	}
	else
		printf("�����ɱ����ջ�Ѿ������ƻ��ˣ�\n");
	
	if (val == cookie) {
		printf("����Ŷ������������ɹ�������getbuf���� 0X%08X\n", val);
	}
	else if (val == 1) {
		printf("������û�����.....����ʧ�ܣ���������\n");
	}
	else {
		printf("����Ŷ����Ȼ����������ɹ�������getbuf���� 0X%08X\n", val);
	}
}

/* ��1ֻľ��ֻ��Ҫ�޸ķ��ص�ַ�����ɽ��� */
void Trojan1(void)
{
	printf("��ϲ�㣡���Ѿ��ɹ�͵͵�����˵�1ֻľ��!\n");
	printf("ͨ����1ֻľ�����\n");
	exit(0);
}

/* ��2ֻľ��������Ҫ�޸ķ��ص�ַ������Ҫ�޸�ջ�з��صĽ�� */
void Trojan2(int val)
{
	if (val == cookie) {
		printf("����Ŷ����2ֻľ�������ˣ�����ͨ����������ȷ�ģ�(0X%08X)\n", val);
	} else
		printf("��Ҫ���ͣ���Ȼ��2ֻľ�������ˣ�����ͨ�������ǲ���ȷ�ģ�(0X%08X)\n", val);
	if (val == cookie)
		printf("ͨ����2ֻľ�����\n");
	exit(0);
}

/* ��3ֻľ�����������ǹ����ض��Ļ������������ջ�ڣ�Ȼ�󽫷��ص�ַ��Ϊ�ö��ض��������ڡ��˶δ��븺��global_value����Ϊ��Ҫ��cookieֵ */
/* ���ָ�����
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
		printf("��������3ֻľ�������ˣ��������޸���ȫ�ֱ�����ȷ��global_value = 0X%08X\n", global_value);
	} else
		printf("��һ�㣡��3ֻľ�������ˣ�����ȫ�ֱ����޸Ĵ���global_value = 0X%08X\n", global_value);
	if (global_value == cookie)
		printf("ͨ����3ֻľ�����\n");
	exit(0);
}

/* ��4ֻľ�����������ǹ����ض��Ļ������������ջ�ڣ�Ȼ�󽫷��ص�ַ��Ϊ�ö��ض��������ڡ��˶δ��븺��global_value����Ϊ��Ҫ��cookieֵ����Ҫ�������� */
/* ���ָ�����
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
		printf("��������4ֻľ�������ˣ��������޸���ȫ�ֱ�����ȷ��global_value = 0X%08X\n", global_value);
	} else
		printf("��һ�㣡��4ֻľ�������ˣ�����ȫ�ֱ������ԣ�global_value = 0X%08X\n", global_value);
	if (global_value == cookie)
		printf("ͨ����4ֻľ�����\n");
	return;	// �������أ���Ҫ�޸�ջ
}

/* ����������ѧ�ţ��������cookieֵ */
int main(int argc, char *argv[])
{
	int i;
	char *MyRandomBuffer;
	printf("\t2018����������ը������ӭ�㣡\n");
	printf("============================\n");
	
	if (argc == 1)
	{
		printf("ʹ�÷�����%s ѧ�ź�6λ [ѧ�ź�6λ] [ѧ�ź�6λ] ...\n",argv[0]);
		printf("����Ҫ���빥���ַ������Ա�����ľ��һ�������۹���....\n");
		printf("����ʮ��������ʽ���빥���ַ���������00 aa bb cc�ȵ�\n");
		return 0;
	}
	
	printf("��ӭ��ǰ����ս�� %s \n",argv[1]);
	/*����ѧ�ţ���ʼ��һ�������������*/
	rand1_h = (unsigned long)atoi(argv[1]);
	rand1_l=0x29A;
	GenerateRandomNumber(0);
	for (i=2;i<argc;i++)
	{
		rand1_l = (unsigned long)atoi(argv[i]);
		GenerateRandomNumber(0);
	}
	
	cookie = (int)rand1_h;
	printf("���ͨ��������0X%08X\n",cookie);
	printf("============================\n");
	printf("�����빥���ַ�����ʮ�����ƴ�����");
	GenerateRandomNumber(512);
	MyRandomBuffer = (char *)_alloca(rand_div+1);	//��ջ�Ϸ�������ռ�
	MyRandomBuffer[0] = 'h';
	test();

	return 0;
}
