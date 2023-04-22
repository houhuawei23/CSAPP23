# -*- coding: utf-8 -*-
"""
Created on Sat Apr  1 10:12:28 2023

@author: hhw

The simulator of y86-64.

"""

rf_name = ['rax', 'rcx', 'rdx', 'rbx', 'rsp', 'rbp', 'rsi', 'rdi',
           'r8', 'r9', 'r10', 'r11', 'r12', 'r13', 'r14', 'no']
ccReg_name = ['ZF', 'SF', 'OF']
# programm status
mem = ['00']*1000  # ['30', 'f2', ... ]， 字节寻址
rf = [0]*16
ccReg = [0]*3
pcReg = 0  # 字节寻址
iReg = ''
ilenth = 0  # count by bytes
icode = 0
ifun = 0
rA = 0
rB = 0

valA = 0
valB = 0
valC = 0
valE = 0
valM = 0
valP = 0
cond = 0
memaddr = 0


def load_bytes_program(filename):
    global mem, memaddr
    file = open(filename, 'r')
    while True:
        line = file.readline().strip()
        if line == '':
            break
        for byte in split_string_by_length(line, 2):
            mem[memaddr] = byte
            memaddr = memaddr + 1

    file.close


def lendhex2dec(string):
    # input
    res = ''
    for i in range(1, len(string) - 1, 2):
        res = string[i-1:i+1] + res
    return int(res, 16)


def decimal_to_hex_width(decimal_num, width=16):
    hex_string = hex(decimal_num).replace("0x", "")
    hex_string = hex_string.zfill(width)
    return hex_string


def split_string_by_length(input_str, length):
    return [input_str[i:i+length] for i in range(0, len(input_str), length)]


def readmem(baseaddr):
    datastr = ''
    for offset in range(8):
        maddr = baseaddr + offset
        datastr = datastr + mem[maddr]
    return lendhex2dec(datastr)


def writemem(baseaddr, val):
    global mem
    datalist = split_string_by_length(decimal_to_hex_width(val), 2)
    datalist.reverse()  # ['', '']
    for offset in range(8):
        maddr = baseaddr + offset
        mem[maddr] = datalist[offset]


def setcond(ifun):
    global cond, ccReg
    ZF = ccReg[0]
    SF = ccReg[1]
    OF = ccReg[2]
    if ifun == 0:  # no condition
        cond = 1
    if ifun == 1:  # le cond = (SF^OF)|ZF
        cond = (SF ^ OF) | ZF
    if ifun == 2:  # l cond = SF ^ OF
        cond = SF ^ OF
    if ifun == 3:  # e ZF
        cond = ZF
    if ifun == 4:  # ne
        cond = 1 - ZF
    if ifun == 5:  # ge
        cond = 1 - (SF ^ OF)
    if ifun == 6:  # g
        cond = (1 - (SF ^ OF) )& (1 - ZF)


def print_status():
    for i in range(60):
        print('-', end='')
    print()
    # p pc
    print("pc: 0x%x" % pcReg)
    # p valP
    print("valP: 0x%x" % valP)
    # p rf
    for i in range(15):
        print("%-3x " % i, end='')
    print()
    for i in range(15):
        print("%-3s " % rf_name[i], end='')
    print()
    for i in range(15):
        print("%-3d " % rf[i], end='')
    print()
    # print()
    # p cc
    # for i in range(3):
    #     print('%-3s ' % ccReg_name[i], end='')
    # print()
    # for i in range(3):
    #     print("%-3d " % ccReg[i], end='')
    print()
    # p iReg
    print('iReg: 0x%s' % iReg)
    # p rA, rB
    print("rA: %d, rB: %d" % (rA, rB))
    # p valC
    print('valC: 0x%x' % valC)
    # p valA, valB
    print("valA: %d, valB, %d" % (valA, valB))
    # p valE
    print("valE: 0x%x, %d" % (valE, valE))
    # p cc
    print("condition code: %d"%cond)
    print("ZF | SF | OF")
    print("%2d | %2d | %2d" % (ccReg[0], ccReg[1], ccReg[2]))
    # p valM
    print("valM: 0x%x, %d" % (valM, valM))


# seq
"""
use: pc
update: icode, ifun, rA, rB, valC

"""


def fetch():
    global mem, rf, ccReg, pcReg, iReg, ilenth, \
        icode, ifun, rA, rB, valA, valB, valC, valE, valP
    iReg = ''
    for i in range(pcReg, pcReg + 10):
        iReg = iReg + mem[i]

    # iReg = mem[pcReg*2: pcReg*2 + 20]   # 预取10个字节，20*4bits
    icode = int(iReg[0], 16)
    ifun = int(iReg[1], 16)

    if icode == 0 or icode == 1 or icode == 9:
        ilenth = 1
        iReg = iReg[:2]
    elif icode == 2 or icode == 6 or icode == 10 or icode == 11:
        iReg = iReg[:4]
        ilenth = 2
        rA = int(iReg[2], 16)
        rB = int(iReg[3], 16)
    elif icode == 7 or icode == 8:  # jxx, call
        iReg = iReg[:18]
        ilenth = 9
        valC = lendhex2dec(iReg[2:18])
    elif icode == 3 or icode == 4 or icode == 5:
        iReg = iReg
        ilenth = 10
        rA = int(iReg[2], 16)
        rB = int(iReg[3], 16)
        valC = lendhex2dec(iReg[4:20])  # 小端机器
    valP = pcReg + ilenth
    return


"""
input: rA, rB
output: valA, valB
"""


def decode():
    global mem, rf, ccReg, pcReg, iReg, ilenth, \
        icode, ifun, rA, rB, valA, valB, valC, valE

    valA = rf[rA] if rA != 15 else 0  # f
    valB = rf[rB] if rB != 15 else 0

    if icode == 8 or icode == 9:  # call ret
        valA = rf[4]   # rsp
        valB = rf[4]
    elif icode == 10:  # push
        valB = rf[4]
    elif icode == 11:  # pop
        valA = rf[4]
        valB = rf[4]

    return


"""
input: icode, ifun, valA, valB, valC
output: cc, valE, valA
"""


def execute():
    global mem, rf, ccReg, pcReg, iReg, ilenth, \
        icode, ifun, rA, rB, valA, valB, valC, valE

    if icode == 2:  # rrmov # cmovXX
        setcond(ifun)
        valE = valA
    elif icode == 3:  # irmov
        valE = valC
    elif icode == 4 or icode == 5:  # rmmov and mrmov
        valE = valB + valC
    elif icode == 6:  # OPq
        if ifun == 0:
            valE = valA + valB
        elif ifun == 1:
            valE = valB - valA
        elif ifun == 2:
            valE = valA & valB
        elif ifun == 3:
            valE = valA ^ valB

        # set ccReg
        ccReg[0] = 1 if valE == 0 else 0
        ccReg[1] = 1 if valE < 0 else 0
        ccReg[2] = 1 if valE > 100000 else 0
        # if valE == 0:
        #     ccReg[0] = 1
        # if valE < 0:
        #     ccReg[1] = 1
        # if valE > 9999:
        #     ccReg[2] = 1
    elif icode == 7:  # jxx
        setcond(ifun)
    elif icode == 8:  # call
        valE = valB - 8
    elif icode == 9:  # ret
        valE = valB + 8
    elif icode == 10:  # push
        valE = valB - 8
    elif icode == 11:  # pop
        valE = valB + 8

    #

    return


"""
input: valE, valA
output: valM
"""


def memory():
    global mem, rf, ccReg, pcReg, iReg, ilenth, \
        icode, ifun, rA, rB, valA, valB, valC, valE, valM, valP
    if icode == 4:  # rm
        writemem(valE, valA)

    elif icode == 5:  # mr
        valM = readmem(valE)

    elif icode == 8:  # call
        writemem(valE, valP)

    elif icode == 9:  # ret
        valM = readmem(valA)

    elif icode == 10:  # push
        writemem(valE, valA)

    elif icode == 11:  # pop
        valM = readmem(valA)


#     return
"""
input: valE, valM
output:
"""


def write_back():
    global mem, rf, ccReg, pcReg, iReg, ilenth, \
        icode, ifun, rA, rB, valA, valB, valC, valE, valM
    # rr, ir, OP
    if icode == 2 or icode == 3 or icode == 6:
        rf[rB] = valE

    elif icode == 5:  # mr
        rf[rA] = valM

    elif icode == 8:  # call
        rf[4] = valE

    elif icode == 9:  # ret
        rf[4] = valE

    elif icode == 10:  # push
        rf[4] = valE

    elif icode == 11:  # pop
        rf[4] = valE
        rf[rA] = valM

    return


"""
input: icode, cc, valM, valC, valP
output: newpc
"""


def pc_update():
    global mem, rf, ccReg, pcReg, iReg, ilenth, \
        icode, ifun, rA, rB, valA, valB, valC, valE, valM
    if icode == 2 or icode == 3 or icode == 4 or icode == 5 or icode == 6 or\
            icode == 10 or icode == 11:
        pcReg = valP
    elif icode == 7:  # jxx
        pcReg = valC if cond else valP
    elif icode == 8:  # call
        pcReg = valC
    elif icode == 9:  # ret
        pcReg = valM
    return


"""
per cycle
"""


def cycle():
    global mem, rf, ccReg, pcReg
    fetch()
    decode()


def run(file):
    global mem, rf, ccReg, pcReg, iReg, ilenth, \
        icode, ifun, rA, rB, valA, valB, valC, valE

    load_bytes_program(file)
    while True:
        fetch()
        decode()
        execute()
        memory()
        write_back()
        print_status()
        if iReg == '00':
            print('Halt!')
            break
        pc_update()


"""
test
"""
run('sum.txt')

