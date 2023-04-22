# -*- coding: utf-8 -*-
"""
Created on Mon Apr  3 16:43:42 2023

@author: hhw
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
cnd = 0
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
        cond = ~ZF
    if ifun == 5:  # ge
        cond = ~(SF^OF)
    if ifun == 6:  # g
        cond = ~(SF^OF) & ~ZF


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
    print('iReg: %s' % iReg)
    # p rA, rB
    print("rA: %d, rB: %d" % (rA, rB))
    # p valC
    print('valC: 0x%x' % valC)
    # p valA, valB
    print("valA: %d, valB, %d" % (valA, valB))
    # p valE
    print("valE: 0x%x, %d" % (valE, valE))
    # p cc
    print("condition code:")
    print("ZF | SF | OF")
    print("%2d | %2d | %2d" % (ccReg[0], ccReg[1], ccReg[2]))
    # p valM
    print("valM: 0x%x, %d" % (valM, valM))
