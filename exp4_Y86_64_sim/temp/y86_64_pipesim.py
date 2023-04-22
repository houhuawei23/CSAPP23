# -*- coding: utf-8 -*-
"""
Created on Sat Apr  1 10:12:28 2023

@author: hhw

The pipeline simulator of y86-64.

"""
from enum import Enum
import threading


class ICODE(Enum):
    HALT = 0
    NOP = 1
    RRMOV = 2
    IRMOV = 3
    RMMOV = 4
    MRMOV = 5
    OP = 6
    JXX = 7
    COMVXX = 2
    CALL = 8
    RET = 9
    PUSH = 10
    POP = 11


class STAT(Enum):
    SAOK = 1
    SADR = 2
    SINS = 3
    SHLT = 4
    SBUB = 5


class RF(Enum):
    RAX = 0
    RCX = 1
    RBX = 2
    RDX = 3
    RSP = 4
    RBP = 5
    RSI = 6
    RDI = 7
    R8 = 8
    R9 = 9
    R10 = 10
    R11 = 11
    R12 = 12
    R13 = 13
    R14 = 14
    RNONE = 15


class ALUFUN(Enum):
    ADD = 0
    SUB = 1
    AND = 2
    XOR = 3


# utils
def split_string_by_length(input_str, length):
    return [input_str[i:i + length] for i in range(0, len(input_str), length)]


def lEndhex_to_dec(string):
    # input
    res = ''
    for i in range(1, len(string) - 1, 2):
        res = string[i - 1:i + 1] + res
    return int(res, 16)


def decimal_to_hex_width(decimal_num, width=16):
    hex_string = hex(decimal_num).replace("0x", "")
    hex_string = hex_string.zfill(width)
    return hex_string


class pipeline_regs:
    def __init__(self, predPC=0, stat="", icode=0, ifun=0, rA=0, rB=0,
                 valC=0, valP=0, valA=0, valB=0, dstE=0, dstM=0,
                 srcA=0, srcB=0, cnd=0, valE=0):
        self.predPC = predPC
        self.stat = stat
        self.icode = icode
        self.ifun = ifun
        self.rA = rA
        self.rB = rB
        self.valC = valC
        self.valP = valP
        self.valA = valA
        self.valB = valB
        self.dstE = dstE
        self.dstM = dstM
        self.srcA = srcA
        self.srcB = srcB
        self.cnd = cnd
        self.valE = valE

    def update_members(self, **kwargs):
        for member_name, new_value in kwargs.items():
            if hasattr(self, member_name):
                setattr(self, member_name, new_value)
            else:
                print(f"Error: {member_name} is not a member of this class.")

    def bubble(self):
        self.icode = ICODE.NOP
        self.dstE = RF.RNONE
        self.dstM = RF.RNONE
        self.srcA = RF.RNONE
        self.srcB = RF.RNONE


# processor


class processor(threading.Thread):
    mem = ['00'] * 500

    pc = 0
    rf = [0] * 16
    # cc
    ZF = 0
    SF = 0
    OF = 0
    # Stat
    Stat = 0
    # pass
    predPC = 0

    # control signal
    wb_if_stall = 0
    if_id_stall = 0
    if_id_bubble = 0
    id_ex_stall = 0
    id_ex_bubble = 0
    ex_mm_stall = 0
    ex_mm_bubble = 0
    mm_wb_stall = 0
    mm_wb_bubble = 0
    # init the pipline regs between stages
    wb_if_regs = pipeline_regs()

    if_id_regs = pipeline_regs()

    id_ex_regs = pipeline_regs()

    ex_mm_regs = pipeline_regs()

    mm_wb_regs = pipeline_regs()

    def update_members(self, **kwargs):
        for member_name, new_value in kwargs.items():
            if hasattr(self, member_name):
                setattr(self, member_name, new_value)
            else:
                print(f"Error: {member_name} is not a member of this class.")

    def __init__(self, filename):
        super().__init__(self)
        self.load_bytes_program(filename)


    def load_bytes_program(self, filename):

        file = open(filename, 'r')
        memaddr = 0
        while True:
            line = file.readline().strip()
            if line == '':
                break
            for byte in split_string_by_length(line, 2):
                self.mem[memaddr] = byte
                memaddr = memaddr + 1
        file.close()
        print("The program has been loaded!")

    def readmem(self, baseaddr, nbytes=8):
        datastr = ''
        for offset in range(nbytes):
            memaddr = baseaddr + offset
            datastr = datastr + self.mem[memaddr]
        # if addr not valid
        # return -1
        return lEndhex_to_dec(datastr)

    def readinst(self, baseaddr, nbytes=10):
        datastr = ''
        for offset in range(nbytes):
            memaddr = baseaddr + offset
            datastr = datastr + self.mem[memaddr]
        # if addr not valid
        # return -1
        return datastr

    def writemem(self, baseaddr, val):
        datalist = split_string_by_length(decimal_to_hex_width(val), 2)
        datalist.reverse()  # little end
        for offset in range(8):
            maddr = baseaddr + offset
            self.mem[maddr] = datalist[offset]
        # if addr not valid
        # return -1

    def set_cc(self, ifun):
        SF = self.SF
        OF = self.OF
        ZF = self.ZF
        if ifun == 0:  # no self.cndition
            self.cnd = 1
        elif ifun == 1:  # le self.cndi = (SF^OF)|ZF
            self.cnd = (SF ^ OF) | ZF
        elif ifun == 2:  # l self.cndi = SF ^ OF
            self.cnd = SF ^ OF
        if ifun == 3:  # e ZF
            self.cnd = ZF
        if ifun == 4:  # ne
            self.cnd = 1 - ZF
        if ifun == 5:  # ge
            self.cnd = 1 - (SF ^ OF)
        if ifun == 6:  # g
            self.cnd = (1 - (SF ^ OF)) & (1 - ZF)

    def cycle(self):
        self.fetch()
        self.decode()
        self.execute()
        self.memory()
        self.write_back()

    def fetch(self):

        f_pc = ''
        if self.if_id_stall:
            return
        elif self.if_id_bubble:
            self.if_id_regs.bubble()
            return

        # Select PC
        if self.ex_mm_regs.icode == ICODE.JXX and not self.ex_mm_regs.cnd:
            f_pc = self.ex_mm_regs.valA
        elif self.ex_mm_regs.icode == ICODE.RET:
            f_pc = self.ex_mm_regs.valM
        else:
            f_pc = self.wb_if_regs.predPC
        # Fetch Instruct
        iReg = self.readinst(f_pc, 10)
        # imem_error
        if iReg != -1:
            imem_error = 0
        else:
            imem_error = 1
        icode = iReg[0]
        ifun = iReg[1]
        # Instr valid
        if icode >= 0 and icode <= 11:
            instr_valid = 1
        else:
            instr_valid = 0
        # Need regids
        if icode in set(ICODE.RRMOV, ICODE.OP, ICODE.PUSH, ICODE.POP,
                        ICODE.IRMOV, ICODE.RMMOV, ICODE.MRMOV):
            need_regids = 1
        else:
            need_regids = 0
        # Need valC
        if icode in set(ICODE.IRMOV, ICODE.RMMOV, ICODE.MRMOV, ICODE.JXX,
                        ICODE.CALL):
            need_valC = 1
        else:
            need_valC = 0
        # Align
        if (not need_regids) and (not need_valC):
            rA = 15
            rB = 15
            valC = 0
        elif need_regids and (not need_valC):
            rA = iReg[2]
            rB = iReg[3]
            valC = 0
        elif (not need_regids) and need_valC:
            rA = 15
            rB = 15
            valC = lEndhex_to_dec(iReg[2: 18])
        else:
            rA = iReg[2]
            rB = iReg[3]
            valC = lEndhex_to_dec(iReg[4: 20])
        # PC inc
        valP = f_pc + 1 + need_regids + need_valC * 8
        # Predict PC
        if icode in set(ICODE.JXX, ICODE.CALL):
            self.predPC = valC
        else:
            self.predPC = valP
        # 怎么将predPC传给wb_if_regs中的predPC呢？
        # Stat
        if imem_error:
            stat = STAT.SADR
        elif not instr_valid:
            stat = STAT.SINS
        elif icode == ICODE.HALT:
            stat = STAT.SHLT
        else:
            stat = STAT.SAOK
        # update the pipeplie regs
        self.if_id_regs.update_members(stat=stat, icode=icode, ifun=ifun,
                                       rA=rA, rB=rB, valC=valC, valP=valP)
        # selectPC = self.wb_if_regs.predPC

    def decode(self, e_dstE, e_valE):
        #
        if self.id_ex_stall:
            return
        elif self.id_ex_bubble:
            self.id_ex_regs.bubble()
            return
        #
        if_id_stat = self.if_id_regs.stat
        if_id_icode = self.if_id_regs.icode
        if_id_ifun = self.if_id_regs.ifun
        if_id_rA = self.if_id_regs.rA
        if_id_rB = self.if_id_regs.rB
        if_id_valC = self.if_id_regs.valC
        if_id_valP = self.if_id_regs.valP
        # ex_mm
        ex_mm_valE = self.ex_mm_regs.valE
        ex_mm_valM = self.ex_mm_regs.valM
        ex_mm_dstE = self.ex_mm_regs.dstE
        ex_mm_dstM = self.ex_mm_regs.dstM
        # mm_wb
        mm_wb_valE = self.mm_wb_regs.valE
        mm_wb_valM = self.mm_wb_regs.valM
        mm_wb_dstE = self.mm_wb_regs.dstE
        mm_wb_dstM = self.mm_wb_regs.dstM
        # ???

        # d_dstE 写rf时，valE 写到 M[rB]
        if if_id_icode in set(ICODE.RRMOV, ICODE.IRMOV, ICODE.OP):
            d_dstE = if_id_rB
        elif if_id_icode in set(ICODE.PUSH, ICODE.POP, ICODE.CALL, ICODE.RET):
            d_dstE = RF.RSP  # rsp
        else:
            d_dstE = RF.RNONE  # RNORE
        # d_dstM 写rf时。valM 写到 M[rA]： mrmov, pop
        if if_id_icode in set(ICODE.MRMOV, ICODE.POP):
            d_dstM = if_id_rA
        else:
            d_dstM = RF.RSP
        # d_srcA 读rf的读地址
        if if_id_icode in set(ICODE.RRMOV, ICODE.RMMOV, ICODE.OP, ICODE.PUSH):
            d_srcA = if_id_rA
        elif if_id_icode in set(ICODE.RET, ICODE.POP):
            d_srcA = RF.RSP
        else:
            d_srcA = RF.RNONE
        # d_srcB
        if if_id_icode in set(ICODE.RMMOV, ICODE.MRMOV, ICODE.OP):
            d_srcB = if_id_rB
        elif if_id_icode in set(ICODE.CALL, ICODE.RET, ICODE.PUSH, ICODE.POP):
            d_srcB = RF.RSP
        else:
            d_srcB = RF.RNONE
        # read rf
        d_rvalA = self.rf[d_srcA]
        d_rvalB = self.rf[d_srcB]
        # Sel + Fwd A
        if if_id_icode in set(ICODE.CALL, ICODE.JXX):
            d_valA = if_id_valP  # 将valA 与 valP合并
        elif d_srcA == e_dstE:  # forward valE from execute
            d_valA = e_valE  # 3执行-译码数据冒险
        elif d_srcA == ex_mm_dstM:  # forward valM from memory
            d_valA = ex_mm_valM  # 2访存-译码数据冒险
        elif d_srcA == ex_mm_dstE:  # forward valE from memory
            d_valA = ex_mm_valE  # 2访存-译码数据冒险
        elif d_srcA == mm_wb_dstE:  # forward valE from write back
            d_valA = mm_wb_valE  # 1写回-译码数据冒险
        elif d_srcA == mm_wb_dstM:
            d_valA = mm_wb_valM
        else:
            d_valA = d_rvalA
        # Fwd B
        if d_srcB == e_dstE:  # forward valE from execute
            d_valB = e_valE  # 3执行-译码数据冒险
        elif d_srcB == ex_mm_dstM:  # forward valM from memory
            d_valB = ex_mm_valM  # 2访存-译码数据冒险
        elif d_srcB == ex_mm_dstE:  # forward valE from memory
            d_valB = ex_mm_valE  # 2访存-译码数据冒险
        elif d_srcB == mm_wb_dstE:  # forward valE from write back
            d_valB = mm_wb_valE  # 1写回-译码数据冒险
        elif d_srcB == mm_wb_dstM:
            d_valB = mm_wb_valM
        else:
            d_valB = d_rvalB
        # update pipeline regs
        self.id_ex_regs.update_members(stat=if_id_stat, icode=if_id_icode,
                                       ifun=if_id_ifun, valC=if_id_valC,
                                       valA=d_valA, valB=d_valB,
                                       dstE=d_dstE, dstM=d_dstM,
                                       srcA=d_srcA, srcB=d_srcB)

    def execute(self):
        if self.ex_mm_stall:
            return
        elif self.ex_mm_bubble:
            self.ex_mm_regs.bubble()
            return
        id_ex_stat = self.id_ex_regs.stat
        id_ex_icode = self.id_ex_regs.icode
        id_ex_ifun = self.id_ex_regs.ifun
        id_ex_valC = self.id_ex_regs.valC
        id_ex_valA = self.id_ex_regs.valA
        id_ex_valB = self.id_ex_regs.valB
        id_ex_dstE = self.id_ex_regs.dstE
        id_ex_dstM = self.id_ex_regs.dstM
        id_ex_srcA = self.id_ex_regs.srcA
        id_ex_srcB = self.id_ex_regs.srcB
        #
        aluA = 0
        aluB = 0
        e_dstE = 0
        e_valE = 0
        # aluA
        if id_ex_icode in set(ICODE.RRMOV, ICODE.OP):
            aluA = id_ex_valA
        elif id_ex_icode in set(ICODE.IRMOV, ICODE.RMMOV, ICODE.MRMOV):
            aluA = id_ex_valC
        elif id_ex_icode in set(ICODE.CALL, ICODE.PUSH):
            aluA = -8
        elif id_ex_icode in set(ICODE.RET, ICODE.POP):
            aluA = 8
        else:
            aluA = 0
        # aluB
        if id_ex_icode in set(ICODE.RMMOV, ICODE.MRMOV, ICODE.OP, ICODE.CALL,
                              ICODE.RET, ICODE.PUSH, ICODE.POP):
            aluB = id_ex_valB
        elif id_ex_icode in set(ICODE.RRMOV, ICODE.IRMOV):
            aluB = 0
        else:
            aluB = 0
        # alufun
        if id_ex_icode == ICODE.OP:
            alufun = id_ex_ifun
        else:
            alufun = ALUFUN.ADD  # aluadd

        # set_cc
        # 4.5.8
        if id_ex_icode == ICODE.OP:
            set_cc = 1
        else:
            set_cc = 0
        if set_cc == 1:
            self.set_cc(id_ex_ifun)
        # valE
        if alufun == ALUFUN.ADD:
            e_valE = aluA + aluB
        elif alufun == ALUFUN.SUB:
            e_valE = aluB - aluA
        elif alufun == ALUFUN.AND:
            e_valE = aluA & aluB
        elif alufun == ALUFUN.XOR:
            e_valE = aluA ^ aluB
        else:
            e_valE = 0
        # e_cnd
        # 没有仔细分析的
        if id_ex_icode in set(ICODE.JXX, ICODE.CALL):
            e_cnd = self.cond(id_ex_ifun)
        else:
            e_cnd = 1

        e_dstE = id_ex_dstE
        self.ex_mm_regs.update_members(stat=id_ex_stat, icode=id_ex_icode,
                                       cnd=e_cnd, valE=e_valE, valA=id_ex_valA,
                                       dstE=e_dstE, dstM=id_ex_dstM)

    def memory(self):
        if self.mm_wb_stall:
            return
        elif self.mm_wb_bubble:
            self.mm_wb_regs.bubble()
            return
        ex_mm_stat = self.ex_mm_regs.stat
        ex_mm_icode = self.ex_mm_regs.icode
        # ex_mm_ifun = self.ex_mm_regs.ifun
        ex_mm_cnd = self.ex_mm_regs.cnd
        ex_mm_valE = self.ex_mm_regs.valE
        ex_mm_valA = self.ex_mm_regs.valA
        ex_mm_dstE = self.ex_mm_regs.dstE
        ex_mm_dstM = self.ex_mm_regs.dstM

        mem_addr = 0
        mem_read = False
        mem_write = False
        m_valM = 0

        # mem_addr
        if ex_mm_icode in set(ICODE.RMMOV, ICODE.MRMOV, ICODE.PUSH, ICODE.CALL):
            mem_addr = ex_mm_valE
        elif ex_mm_icode in set(ICODE.POP, ICODE.RET):
            mem_addr = ex_mm_valA
        else:
            mem_addr = 0
        # mem_read
        if ex_mm_icode in set(ICODE.MRMOV, ICODE.POP, ICODE.RET):
            mem_read = 1
        else:
            mem_read = 0
        # mem_write
        if ex_mm_icode in set(ICODE.RMMOV, ICODE.CALL, ICODE.PUSH):
            mem_write = 1
        else:
            mem_write = 0
        # m_stat
        m_stat = ex_mm_stat  # need to be modify
        # read and write
        if mem_read:
            m_valM = self.readmem(mem_addr, 8)
            if m_valM == -1:  # 读存错误
                m_stat = STAT.ADR
        elif mem_write:
            tmp = self.writemem(mem_addr, ex_mm_valA)
            if tmp == -1:  # 读存错误
                m_stat = STAT.ADR

        self.mm_wb_regs.update_members(stat=m_stat, icode=ex_mm_icode,
                                       valE=ex_mm_valE, valM=m_valM,
                                       dstE=ex_mm_dstE, dstM=ex_mm_dstM)

    def write_back(self):
        #
        wb_dstE = self.mm_wb_regs.dstE
        wb_dstM = self.mM_wb_regs.dstM
        wb_E = self.mm_wb_regs.valE
        wb_M = self.mm_wb_regs.valM
        mm_wb_stat = self.mm_wb_regs.stat

        self.rf[wb_dstE] = wb_E
        self.rf[wb_dstM] = wb_M
        # update processor stat
        if mm_wb_stat == STAT.SBUB:
            self.Stat = STAT.SAOK
        else:
            self.Stat = mm_wb_stat

        # 
        if self.wb_if_stall:
            return
        self.wb_if_regs.update_members(predPC=self.predPC)

    def pipeline_control(self):


# mycpu = processor_status('sum.txt')

# def cycle(mycpu):
#     mycpu.fetch()
#     mycpu.decode()
#     mycpu.execute()
#     mycpu.memory()
#     mycpu.write_back()
#     mycpu.update_pc()
def run(file):
    mycpu = processor()
    while True:
        mycpu.cycle()

# inst = pipeline_regs(icode=1, ifun=0, valC=10, valP=20)
# print(inst.valC)  # Output: 10

# inst.update_members(valC=20, valP=30)
# print(inst.valC)  # Output: 20
# print(inst.valP)  # Output: 30
