动态内存分配器实验

你需要修改mm.c（你只能修改此文件，其他文件不能修改），使之能够处理内存分配（mm_malloc）、内存释放（mm_free）、内存扩张（mm_realloc）等功能。
你可以修改traces目录下的TRACE_LIST.txt，以运行不同的trace。
你需要跑尽可能多的trace，并在评分中拿到尽可能的高分。

【注意】并不是所有trace都可以跑的。有些trace内部包含了错误的操作，是跑不通的。比如：
1、试图realloc一个不存在的指针
2、试图free一个不存在的指针

Linux：
1、make
2、./malloc -t traces

Windows:
1、用VS2019打开工程myMalloc/myMalloc.sln，编译
2、生成可执行代码，myMalloc -t traces


【提交】你需要将mm.c修改为mm_201900221122.c，其中后面是你的学号。提交到educoder上。你只需要提交mm.c文件。