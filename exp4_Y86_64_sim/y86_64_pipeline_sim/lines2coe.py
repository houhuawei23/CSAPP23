import time

radix = 16
width = 16
bytewidth = 4
datastr = ''
with open('sum.txt', 'r') as f:
    lines = f.readlines()

for line in lines:
    datastr = datastr + line.strip()
with open('sum.coe', 'w') as outf:
    outf.write(';XILINX COREGenerator(tm)Distributed Arithmetic FIR filter coefficient (.COE) File\n')
    outf.write(';Created by Python\n')
    outf.write(';Date: %s\n' % time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time.time())))
    outf.write('Radix = %d;\n' % radix)
    outf.write('Coefficient_Width = %d;\n' % width)
    outf.write('CoefData = \n')
    for i in range(0, len(datastr), bytewidth):
        if i > len(datastr) - bytewidth:
            outf.write('%s' % datastr[i:i + bytewidth])
        else:
            outf.write('%s,\n' % datastr[i:i + bytewidth])
    outf.write(';\n')
