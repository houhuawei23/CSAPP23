'''
多周期y86-64模拟器的主要枚举类，使用类
通用寄存器类，流水线寄存器类，内存类，Processor类
'''


from enum import Enum
from utils import *
import re
rf_name = ['rax', 'rcx', 'rdx', 'rbx', 'rsp', 'rbp', 'rsi', 'rdi',
           'r8', 'r9', 'r10', 'r11', 'r12', 'r13', 'r14', 'no']


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
    RST = 0
    AOK = 1
    ADR = 2
    INS = 3
    HLT = 4
    BUB = 5


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


class Memory:
    def __init__(self):
        self.mem = ['00'] * 1024

    # 读取内存
    def read(self, baseaddr, nbytes=8) -> str:
        # 如果地址不合法，抛出异常，return -1

        # 如果地址合法，读取内存
        datastr = ''
        for offset in range(nbytes):
            memaddr = baseaddr + offset
            datastr = datastr + self.mem[memaddr]
        # datastr = datastr[::-1]  # little end
        return datastr

    def read_xilinx_coe_file(self, filepath):
        with open(filepath, 'r') as f:
            content = f.readlines()

        data_lines =''
        read_data = False
        radix = 16
        width = 12
        for line in content:
            if 'Radix' in line:
                radix = int(line.split("=", 1)[1].strip()[:-1])
            elif 'Coefficient_Width' in line:
                width = int(line.split("=", 1)[1].strip()[:-1])
            elif "CoefData" in line:
                read_data = True
                data_line = line.split("=", 1)[1].strip()[:-1]
                data_lines += data_line
            elif read_data:
                    data_lines+=line.strip()[:-1]
        if radix == 2:
            data_lines = hex(int(data_lines, 2))[2:]
        elif radix == 10:
            data_lines = hex(int(data_lines, 10))[2:]

        memaddr = 0
        for byte in split_string_by_length(data_lines, 2):
            self.mem[memaddr] = byte
            memaddr = memaddr + 1

        print("The COE file program has been loaded into memory.")
    def read_data(self, baseaddr, nbytes=8) -> int:
        # 如果地址不合法，抛出异常，return -1

        # 如果地址合法，读取内存
        datastr = ''
        for offset in range(nbytes):
            memaddr = baseaddr + offset
            datastr = self.mem[memaddr] + datastr
        return int(datastr, 16)

    def write(self, baseaddr, data):
        # 如果地址或数据不合法，抛出异常

        # 如果地址合法，写入内存
        datalist = split_string_by_length(dec_to_lend_hex_width(data, 16), 2)
        datalist.reverse()  # little end
        for offset in range(8):
            maddr = baseaddr + offset
            self.mem[maddr] = datalist[offset]
        return 1

    def load(self, filename):
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


class ComRegs:
    def __init__(self):
        self.regs = [0] * 16
        self.ZF = 0
        self.SF = 0
        self.OF = 0
        self.PC = 0
        self.iReg = ''

    def readrf(self, dstX):
        if dstX == RF.RNONE.value:
            return 0
        else:
            return self.regs[dstX]

    def writerf(self, dstX, valX):
        if dstX != RF.RNONE.value:
            self.regs[dstX] = valX

    def print(self):
        print("PC: 0x%x" % self.PC, end='| ')
        print("iReg: %s" % self.iReg)
        print("ZF: %d" % self.ZF, end='|')
        print("SF: %d" % self.SF, end='|')
        print("OF: %d" % self.OF)
        for i in range(16):
            print("%-3x " % i, end='')
        print()
        for i in range(16):
            print("%-3s " % rf_name[i], end='')
        print()
        for i in range(16):
            print("%-3d " % self.regs[i], end='')
        print()


class WBIF:
    def __init__(self, predPC):
        self.predPC = predPC

    def update(self, stall, predPC):
        if stall:
            self.predPC = self.predPC
        else:
            self.predPC = predPC

    def print(self):
        print("wbif(predPC: 0x%x)" % self.predPC)


class IFID:
    def __init__(self, stat, icode, ifun, rA, rB, valC, valP):
        self.icode = icode
        self.stat = stat
        self.ifun = ifun
        self.rA = rA
        self.rB = rB
        self.valC = valC
        self.valP = valP

    def update(self, stall, bubble, stat, icode, ifun, rA, rB, valC, valP):
        if stall:
            self.stat = self.stat
            self.icode = self.icode
            self.ifun = self.ifun
            self.rA = self.rA
            self.rB = self.rB
            self.valC = self.valC
            self.valP = self.valP
        elif bubble:
            self.stat = STAT.BUB.value
            self.icode = ICODE.NOP.value
            self.ifun = 0
            self.rA = RF.RNONE.value
            self.rB = RF.RNONE.value
            self.valC = 0
            self.valP = 0
        else:
            self.stat = stat
            self.icode = icode
            self.ifun = ifun
            self.rA = rA
            self.rB = rB
            self.valC = valC
            self.valP = valP

    def print(self):
        # print("%d"%self.stat)
        print("ifid(stat: %d, icode: %d, ifun: %d, rA: %d, rB: %d, valC: 0x%x, valP: 0x%x)" % (
            self.stat, self.icode, self.ifun, self.rA, self.rB, self.valC, self.valP))


class IDEX:
    def __init__(self, stat, icode, ifun, valC, valA, valB, dstE, dstM, srcA, srcB):
        self.stat = stat
        self.icode = icode
        self.ifun = ifun
        self.valC = valC
        self.valA = valA
        self.valB = valB
        self.dstE = dstE
        self.dstM = dstM
        self.srcA = srcA
        self.srcB = srcB

    def update(self, bubble, stat, icode, ifun, valC, valA, valB, dstE, dstM, srcA, srcB):
        if bubble:
            self.stat = STAT.BUB.value
            self.icode = ICODE.NOP.value
            self.ifun = 0
            self.valC = 0
            self.valA = 0
            self.valB = 0
            self.dstE = RF.RNONE.value
            self.dstM = RF.RNONE.value
            self.srcA = RF.RNONE.value
            self.srcB = RF.RNONE.value
        else:
            self.stat = stat
            self.icode = icode
            self.ifun = ifun
            self.valC = valC
            self.valA = valA
            self.valB = valB
            self.dstE = dstE
            self.dstM = dstM
            self.srcA = srcA
            self.srcB = srcB

    def print(self):
        print(
            "idex(stat: %d, icode: %d, ifun: %d, valC: 0x%x, valA: 0x%x, valB: 0x%x, dstE: %d, dstM: %d, srcA: %d, srcB: %d)" % (
                self.stat, self.icode, self.ifun, self.valC, self.valA, self.valB, self.dstE, self.dstM, self.srcA,
                self.srcB))


class EXMEM:
    def __init__(self, stat, icode, Cnd, valE, valA, dstE, dstM):
        self.stat = stat
        self.icode = icode
        self.Cnd = Cnd
        self.valE = valE
        self.valA = valA
        self.dstE = dstE
        self.dstM = dstM

    def update(self, bubble, stat, icode, Cnd, valE, valA, dstE, dstM):
        if bubble:
            self.stat = STAT.BUB.value
            self.icode = ICODE.NOP.value
            self.Cnd = 0
            self.valE = 0
            self.valA = 0
            self.dstE = RF.RNONE.value
            self.dstM = RF.RNONE.value
        else:
            self.stat = stat
            self.icode = icode
            self.Cnd = Cnd
            self.valE = valE
            self.valA = valA
            self.dstE = dstE
            self.dstM = dstM

    def print(self):
        print("exmem(stat: %d, icode: %d, Cnd: %d, valE: 0x%x, valA: 0x%x, dstE: %d, dstM: %d)" % (
            self.stat, self.icode, self.Cnd, self.valE, self.valA, self.dstE, self.dstM))


class MEMWB:
    def __init__(self, stat, icode, valE, valM, dstE, dstM):
        self.stat = stat
        self.icode = icode
        self.valE = valE
        self.valM = valM
        self.dstE = dstE
        self.dstM = dstM

    def update(self, stall, stat, icode, valE, valM, dstE, dstM):
        if stall:
            self.stat = self.stat
            self.icode = self.icode
            self.valE = self.valE
            self.valM = self.valM
            self.dstE = self.dstE
            self.dstM = self.dstM
        else:
            self.stat = stat
            self.icode = icode
            self.valE = valE
            self.valM = valM
            self.dstE = dstE
            self.dstM = dstM

    def print(self):
        print("memwb(stat: %d, icode: %d, valE: 0x%x, valM: 0x%x, dstE: %d, dstM: %d)" % (
            self.stat, self.icode, self.valE, self.valM, self.dstE, self.dstM))


class Processor:
    def __init__(self):
        self.mem = Memory()
        self.wbif = WBIF(0)
        self.ifid = IFID(STAT.AOK.value, ICODE.NOP.value, 0, RF.RNONE.value, RF.RNONE.value, 0, 0)
        self.idex = IDEX(STAT.AOK.value, ICODE.NOP.value, 0, 0, 0, 0, RF.RNONE.value, RF.RNONE.value, RF.RNONE.value,
                         RF.RNONE.value)
        self.exmem = EXMEM(STAT.AOK.value, ICODE.NOP.value, 0, 0, 0, RF.RNONE.value, RF.RNONE.value)
        self.memwb = MEMWB(STAT.AOK.value, ICODE.NOP.value, 0, 0, RF.RNONE.value, RF.RNONE.value)
        self.comregs = ComRegs()
        self.Stat = STAT.AOK.value

        #

    def setcc(self, valE):

        if valE == 0:
            self.comregs.ZF = 1
        else:
            self.comregs.ZF = 0

        if valE < 0:
            self.comregs.SF = 1
        else:
            self.comregs.SF = 0

        if valE > 0x7FFFFFFF:
            self.comregs.OF = 1
        else:
            self.comregs.OF = 0

    def cond(self, ifun):
        ZF = self.comregs.ZF
        SF = self.comregs.SF
        OF = self.comregs.OF
        cnd = 0
        if ifun == 0:  # no condition
            cnd = 1
        if ifun == 1:  # le cond = (SF^OF)|ZF
            cnd = (SF ^ OF) | ZF
        if ifun == 2:  # l cond = SF ^ OF
            cnd = SF ^ OF
        if ifun == 3:  # e ZF
            cnd = ZF
        if ifun == 4:  # ne
            cnd = 1 - ZF
        if ifun == 5:  # ge
            cnd = 1 - (SF ^ OF)
        if ifun == 6:  # g
            cnd = (1 - (SF ^ OF)) & (1 - ZF)
        return cnd

    def print(self):
        self.comregs.print()
        self.wbif.print()
        self.ifid.print()
        self.idex.print()
        self.exmem.print()
        self.memwb.print()

    def ifetch(self):
        f_pc = ''
        f_iReg = ''
        instr_valid = 0
        need_regids = 0
        need_valC = 0
        # Select PC
        if self.exmem.icode == ICODE.JXX.value and not self.exmem.Cnd:
            f_pc = self.exmem.valA
        elif self.memwb.icode == ICODE.RET.value:
            f_pc = self.memwb.valM
        else:
            f_pc = self.wbif.predPC
        # Read 10 bytes at PC
        f_iReg = self.mem.read(f_pc, 10)
        if f_iReg == -1:
            imem_error = 1
        else:
            imem_error = 0
        f_icode = int(f_iReg[0], 16)
        f_ifun = int(f_iReg[1], 16)
        # Instruction valid
        if 0 <= f_icode <= 11:
            instr_valid = 1
        # Need regids
        if f_icode in {ICODE.RRMOV.value, ICODE.OP.value, ICODE.PUSH.value, ICODE.POP.value, ICODE.IRMOV.value,
                       ICODE.RMMOV.value, ICODE.MRMOV.value}:
            need_regids = 1
        # Need valC
        if f_icode in {ICODE.IRMOV.value, ICODE.RMMOV.value, ICODE.MRMOV.value, ICODE.JXX.value, ICODE.CALL.value}:
            need_valC = 1
        # Align
        if (not need_regids) and (not need_valC):
            f_rA = RF.RNONE.value
            f_rB = RF.RNONE.value
            f_valC = 0
        elif need_regids and (not need_valC):
            f_rA = int(f_iReg[2], 16)
            f_rB = int(f_iReg[3], 16)
            f_valC = 0
        elif (not need_regids) and need_valC:
            f_rA = RF.RNONE.value
            f_rB = RF.RNONE.value
            f_valC = lend_hex_to_dec(f_iReg[2: 18])
        else:
            f_rA = int(f_iReg[2], 16)
            f_rB = int(f_iReg[3], 16)
            f_valC = lend_hex_to_dec(f_iReg[4: 20])
        # PC inc
        f_valP = f_pc + 1 + need_regids + need_valC * 8
        f_iReg = f_iReg[0:(f_valP - f_pc) * 2]
        # Predict PC
        if f_icode in {ICODE.JXX.value, ICODE.CALL.value}:
            f_predPC = f_valC
        else:
            f_predPC = f_valP
        # Stat
        if imem_error:
            f_stat = STAT.ADR.value
        elif not instr_valid:
            f_stat = STAT.INS.value
        elif f_icode == ICODE.HALT.value:
            f_stat = STAT.HLT.value
        else:
            f_stat = STAT.AOK.value

        self.comregs.iReg = f_iReg
        self.comregs.PC = f_pc
        # self.f_pc = f_pc
        next_ifid = (f_stat, f_icode, f_ifun, f_rA, f_rB, f_valC, f_valP)
        forwarding = (f_predPC)
        return next_ifid, forwarding

    def idecode(self, e_dstE, e_valE, m_valM):
        d_stat = self.ifid.stat
        d_icode = self.ifid.icode
        d_ifun = self.ifid.ifun
        d_valC = self.ifid.valC
        d_valA = 0
        d_valB = 0
        d_dstE = RF.RNONE.value
        d_dstM = RF.RNONE.value
        d_srcA = RF.RNONE.value
        d_srcB = RF.RNONE.value

        ifid_stat = self.ifid.stat
        ifid_icode = self.ifid.icode
        ifid_ifun = self.ifid.ifun
        ifid_rA = self.ifid.rA
        ifid_rB = self.ifid.rB
        ifid_valC = self.ifid.valC
        ifid_valP = self.ifid.valP
        # ex_mm
        exmem_valE = self.exmem.valE
        # exmm_valM = self.exmem.valM
        exmem_dstE = self.exmem.dstE
        exmem_dstM = self.exmem.dstM
        # mm_wb
        memwb_valE = self.memwb.valE
        memwb_valM = self.memwb.valM
        memwb_dstE = self.memwb.dstE
        memwb_dstM = self.memwb.dstM
        # d_dstE; 写rf时，valE 写到 M[rB]
        if self.ifid.icode in {ICODE.RRMOV.value, ICODE.IRMOV.value, ICODE.OP.value}:
            d_dstE = self.ifid.rB
        elif self.ifid.icode in {ICODE.PUSH.value, ICODE.POP.value, ICODE.CALL.value, ICODE.RET.value}:
            d_dstE = RF.RSP
        # d_dstM; 写rf时。valM 写到 M[rA]： mrmov, pop
        if self.ifid.icode in {ICODE.MRMOV.value, ICODE.POP.value}:
            d_dstM = self.ifid.rA
        # d_srcA; 读rf时，valA 读自 M[rA]
        if self.ifid.icode in {ICODE.RRMOV.value, ICODE.RMMOV.value, ICODE.OP.value, ICODE.PUSH.value}:
            d_srcA = self.ifid.rA
        elif self.ifid.icode in {ICODE.POP.value, ICODE.RET.value}:
            d_srcA = RF.RSP
        # d_srcB; 读rf时，valB 读自 M[rB]
        # ICODE.RMMOV, ICODE.MRMOV, ICODE.OP
        if self.ifid.icode in {ICODE.RMMOV.value, ICODE.MRMOV.value, ICODE.OP.value}:
            d_srcB = self.ifid.rB
        elif self.ifid.icode in {ICODE.PUSH.value, ICODE.POP.value, ICODE.CALL.value, ICODE.RET.value}:
            d_srcB = RF.RSP
        # read rf
        d_rvalA = self.comregs.readrf(d_srcA)
        d_rvalB = self.comregs.readrf(d_srcB)
        # Sel + FwdA
        if self.ifid.icode in {ICODE.CALL.value, ICODE.JXX.value}:
            d_valA = self.ifid.valP  # 将valA 与 valP合并
        elif d_srcA == e_dstE:  # forward valE from execute
            d_valA = e_valE  # 3执行-译码数据冒险
        elif d_srcA == exmem_dstM:  # forward valM from memory
            d_valA = m_valM  # 2访存-译码数据冒险
        elif d_srcA == exmem_dstE:  # forward valE from memory
            d_valA = exmem_valE  # 2访存-译码数据冒险
        elif d_srcA == memwb_dstE:  # forward valE from write back
            d_valA = memwb_valE  # 1写回-译码数据冒险
        elif d_srcA == memwb_dstM:
            d_valA = memwb_valM
        else:
            d_valA = d_rvalA
        # FwdB
        if d_srcB == e_dstE:  # forward valE from execute
            d_valB = e_valE  # 3执行-译码数据冒险
        elif d_srcB == exmem_dstM:  # forward valM from memory
            d_valB = m_valM  # 2访存-译码数据冒险
        elif d_srcB == exmem_dstE:  # forward valE from memory
            d_valB = exmem_valE  # 2访存-译码数据冒险
        elif d_srcB == memwb_dstE:  # forward valE from write back
            d_valB = memwb_valE  # 1写回-译码数据冒险
        elif d_srcB == memwb_dstM:
            d_valB = memwb_valM
        else:
            d_valB = d_rvalB
        next_idex = (d_stat, d_icode, d_ifun, d_valC, d_valA, d_valB, d_dstE, d_dstM, d_srcA, d_srcB)
        forwarding = (d_srcA, d_srcB)
        return next_idex, forwarding

    def execute(self, m_stat):
        # id_ex
        idex_stat = self.idex.stat
        idex_icode = self.idex.icode
        idex_ifun = self.idex.ifun
        idex_valC = self.idex.valC
        idex_valA = self.idex.valA
        idex_valB = self.idex.valB
        idex_dstE = self.idex.dstE
        idex_dstM = self.idex.dstM
        idex_srcA = self.idex.srcA
        idex_srcB = self.idex.srcB
        # # ex_mm
        # exmem_valE = self.exmem.valE
        # exmem_valM = self.exmem.valM
        # exmem_dstE = self.exmem.dstE
        # exmem_dstM = self.exmem.dstM
        # # mm_wb
        # memwb_valE = self.memwb.valE
        # memwb_valM = self.memwb.valM
        # memwb_dstE = self.memwb.dstE
        # memwb_dstM = self.memwb.dstM

        # aluA
        if idex_icode in {ICODE.RRMOV.value, ICODE.OP.value}:
            aluA = idex_valA
        elif idex_icode in {ICODE.IRMOV.value, ICODE.RMMOV.value, ICODE.MRMOV.value}:
            aluA = idex_valC
        elif idex_icode in {ICODE.PUSH.value, ICODE.POP.value}:
            aluA = -8
        elif idex_icode in {ICODE.CALL.value, ICODE.RET.value}:
            aluA = 8
        else:
            aluA = 0
        # aluB
        if idex_icode in {ICODE.RMMOV.value, ICODE.MRMOV.value, ICODE.OP.value, ICODE.CALL.value,
                          ICODE.RET.value, ICODE.PUSH.value, ICODE.POP.value}:
            aluB = idex_valB
        elif idex_icode in {ICODE.RRMOV.value, ICODE.IRMOV.value}:
            aluB = 0
        else:
            aluB = 0
        # alufun
        if idex_icode == ICODE.OP.value:
            alufun = idex_ifun
        else:
            alufun = ALUFUN.ADD.value

        memexp = m_stat in {STAT.ADR.value, STAT.INS.value, STAT.HLT.value}
        wbexp = self.memwb.stat in {STAT.ADR.value, STAT.INS.value, STAT.HLT.value}
        # set_cc 仅在OP指令中设置条件码
        if self.idex.icode == ICODE.OP.value and not memexp and not wbexp:
            # and m_stat not in {STAT.ADR, STAT.INS, STAT.HLT}:
            # and w_stat not in {STAT.ADR, STAT.INS, STAT.HLT}:
            set_cc = 1
        else:
            set_cc = 0
        # valE
        if alufun == ALUFUN.ADD.value:
            e_valE = aluA + aluB
        elif alufun == ALUFUN.SUB.value:
            e_valE = aluB - aluA
        elif alufun == ALUFUN.AND.value:
            e_valE = aluA & aluB
        elif alufun == ALUFUN.XOR.value:
            e_valE = aluA ^ aluB
        else:
            e_valE = 0
        # 设置条件码
        if set_cc == 1:
            self.setcc(e_valE)  # set ZF SF OF

        # e_cnd 根据ifun和cc条件码设置cond
        # 若cnd == 1, 则跳转; cnd == 0, 则不跳转
        e_cnd = self.cond(idex_ifun)

        # e_dstE 对于条件传送指令，条件成立时，才能写回寄存器
        e_dstE = idex_dstE
        if idex_icode == ICODE.RRMOV.value and e_cnd == 0:
            e_dstE = RF.RNONE.value
        next_exmem = (idex_stat, idex_icode, e_cnd, e_valE, idex_valA, e_dstE, idex_dstM)
        forwarding = (e_dstE, e_valE, e_cnd)
        return next_exmem, forwarding

    def memory(self):

        exmem_stat = self.exmem.stat
        exmem_icode = self.exmem.icode
        exmem_valE = self.exmem.valE
        exmem_valA = self.exmem.valA
        exmem_dstE = self.exmem.dstE
        exmem_dstM = self.exmem.dstM

        # mem_addr
        if exmem_icode in {ICODE.RMMOV.value, ICODE.MRMOV.value, ICODE.PUSH.value, ICODE.CALL.value}:
            mem_addr = exmem_valE
        elif exmem_icode in {ICODE.POP.value, ICODE.RET.value}:
            mem_addr = exmem_valA
        else:
            mem_addr = 0
        # mem_read
        if exmem_icode in {ICODE.MRMOV.value, ICODE.POP.value, ICODE.RET.value}:
            mem_read = 1
        else:
            mem_read = 0
        # mem_write
        if exmem_icode in {ICODE.RMMOV.value, ICODE.PUSH.value, ICODE.CALL.value}:
            mem_write = 1
        else:
            mem_write = 0

        dmem_error = 0
        # mem
        m_valM = 0
        if mem_read == 1:
            # data_str = self.mem.read(mem_addr)
            # m_valM = split_string_by_length(data_str, 2).reverse()
            m_valM = self.mem.read_data(mem_addr)
            if m_valM == -1:
                dmem_error = 1

        if mem_write == 1:
            tmp = self.mem.write(mem_addr, exmem_valA)
            if tmp == -1:
                dmem_error = 1
        # m_stat
        if dmem_error == 1:
            m_stat = STAT.ADR.value
        else:
            m_stat = exmem_stat
        next_mwmwb = (m_stat, exmem_icode, exmem_valE, m_valM, exmem_dstE, exmem_dstM)
        forwarding = (m_valM, m_stat)
        return next_mwmwb, forwarding

    def writeback(self):
        self.comregs.writerf(self.memwb.dstM, self.memwb.valM)
        self.comregs.writerf(self.memwb.dstE, self.memwb.valE)

        # P312
        if self.memwb.stat == STAT.BUB:
            self.Stat = STAT.AOK
        else:
            self.Stat = self.memwb.stat
        next_wbif = ()
        forwarding = (self.memwb.icode, self.memwb.valE, self.memwb.valM, self.memwb.dstE, self.memwb.dstM)
        return next_wbif, forwarding

    def control_logic(self, d_srcA, d_srcB, e_cnd, m_stat):
        wbif_stall = 0
        ifid_bubble = 0
        ifid_stall = 0
        idex_bubble = 0
        exmem_bubble = 0
        memwb_stall = 0
        isloaduse = self.idex.icode in {ICODE.MRMOV.value, ICODE.POP.value} and self.idex.dstM in {d_srcA, d_srcB}
        isret = ICODE.RET.value in {self.idex.icode, self.exmem.icode, self.ifid.icode}
        ismisbranch = self.idex.icode == ICODE.JXX.value and not e_cnd
        # 1 当 加载使用冒险 或 ret，wbif阶段stall
        if isloaduse or isret:
            wbif_stall = 1
        else:
            wbif_stall = 0
        # 2 当 分支预测错误 或 ret，ifid阶段bubble
        # 2 当 加载使用冒险 且 ret，ifid阶段not bubble
        if isloaduse and isret:
            ifid_bubble = 0
        elif ismisbranch or isret:
            ifid_bubble = 1
        else:
            ifid_bubble = 0
        # if ismisbranch or not (isloaduse and isret):
        #     ifid_bubble = 1
        # else:
        #     ifid_bubble = 0

        # 3 当 加载使用冒险时
        if isloaduse:
            ifid_stall = 1
        else:
            ifid_stall = 0
        # 4当 加载使用冒险 或 分支预测错误时
        if ismisbranch or isloaduse:
            idex_bubble = 1
        else:
            idex_bubble = 0
        # 5 当 mem 或 wb 阶段有异常存在时，exmem阶段bubble
        memexp = m_stat in {STAT.ADR.value, STAT.INS.value, STAT.HLT.value}
        wbexp = self.memwb.stat in {STAT.ADR.value, STAT.INS.value, STAT.HLT.value}
        if memexp or wbexp:
            exmem_bubble = 1
        else:
            exmem_bubble = 0

        if wbexp:
            memwb_stall = 1
        else:
            memwb_stall = 0

        return wbif_stall, ifid_bubble, ifid_stall, idex_bubble, exmem_bubble, memwb_stall

    def run(self):
        next_wbif, fdwb = self.writeback()
        next_mwmwb, fdmem = self.memory()
        next_exmem, fdex = self.execute(fdmem[1]) # m_stat
        next_idex, fdid = self.idecode(fdex[0], fdex[1], fdmem[0])
        next_ifid, fdif = self.ifetch()

        wbif_stall, ifid_bubble, ifid_stall, idex_bubble, exmem_bubble, memwb_stall = self.control_logic(fdid[0],
                                                                                                         fdid[1],
                                                                                                         fdex[2],
                                                                                                         fdmem[1])

        self.wbif.update(wbif_stall, fdif)
        self.ifid.update(ifid_bubble, ifid_stall, *next_ifid)
        self.idex.update(idex_bubble, *next_idex)
        self.exmem.update(exmem_bubble, *next_exmem)
        self.memwb.update(memwb_stall, *next_mwmwb)
