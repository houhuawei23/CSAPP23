#include <sys/time.h>
#include <sys/resource.h>
#include <iostream>

using namespace std;
const int SIZE = 1000000000;

int main()
{
    // 矩阵乘
    int n = 100000;
    int **a = new int *[n];
    int **b = new int *[n];
    int **c = new int *[n];
    for (int i = 0; i < n; ++i)
    {
        a[i] = new int[n];
        b[i] = new int[n];
        c[i] = new int[n];
    }
    cout << "Matrix multiplication" << endl;
    int x = 5;
    // 写文件 a.txt
    FILE *fp = fopen("a.txt", "w");
    for (int i = 0; i < n; ++i)
    {
        fwrite(&x, sizeof(int), 1, fp);
    }
    fclose(fp);

    // 读文件 a.txt
    fp = fopen("a.txt", "r");
    for (int i = 0; i < n; ++i)
    {
        fread(&x, sizeof(int), 1, fp);
    }
    fclose(fp);

    cout << "File read/write" << endl;
    // test
    int *p = new int[SIZE];
    for (int i = 0; i < SIZE; i++)
    {
        p[i] = i;
    }
    delete[] p;
    struct rusage r_usage;
    int res = getrusage(RUSAGE_SELF, &r_usage);
    if (res == -1)
    {
        cout << "Error: getrusage" << endl;
        return -1;
    }
    cout << "Memory usage: " << r_usage.ru_maxrss << endl;
    cout << "User time: " << r_usage.ru_utime.tv_sec << endl;
    cout << "System time: " << r_usage.ru_stime.tv_sec << endl;
    cout << "page reclaims (soft page faults): " << r_usage.ru_minflt << endl;
    cout << "page faults (hard page faults): " << r_usage.ru_majflt << endl;
    return 0;
}
