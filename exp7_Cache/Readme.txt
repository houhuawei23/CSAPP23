注意！你只能修改Cache.c文件，其他文件请勿修改！


Linux:
  1. make
  2. ./Cache traces/long.trace.zst

提醒:Linux下需要安装libzstd-dev软件包，例如apt install libzstd-dev zstd

Windows VS 2019:
  1. 进入MyCache目录，打开MyCache.sln
  2. 编译即可生成可执行文件
  3. 启动命令行窗口， 运行.\MyCache.exe traces/long.trace.zst


提示：在traces目录下，有多个trace可以跑。


自行生成trace文件：
1、安装valgrind，Ubuntu下，可以使用apt install valgrind完成；
2、运行valgrind生成某个应用的trace文件。比如，为了生成ls命令运行时的trace，可以输入下列命令：
valgrind --tool=lackey -v --trace-mem=yes --log-file=ls.trace ls -l
3、将生成的trace文件打包为zst压缩格式，例如
   zstd ls.trace -o ls.trace.zst
4、使用Cache跑这个trace：
   ./Cache ./ls.trace.zst
