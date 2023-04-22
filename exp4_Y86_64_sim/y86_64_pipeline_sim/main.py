'''
主测试函数
'''
from utils import *
from classes import *

# filename = 'sum.txt'
filename = 'y86_64_test.txt'
coefilename = 'sum.coe'

# 初始化
mypc = Processor()
# mypc.mem.load(filename)
mypc.mem.read_xilinx_coe_file(coefilename)

# 运行
while True:
    mypc.run()
    mypc.print()
    if mypc.Stat == STAT.HLT.value:
        print('Halt!')
        break
