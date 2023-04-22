# -*- coding: utf-8 -*-
"""
Created on Sat Apr  1 10:12:28 2023

@author: hhw

The pipeline simulator of y86-64.

"""
# utils
def split_string_by_length(input_str, length):
    return [input_str[i:i+length] for i in range(0, len(input_str), length)]

def lendhex_to_dec(string):
    # input
    res = ''
    for i in range(1, len(string) - 1, 2):
        res = string[i-1:i+1] + res
    return int(res, 16)

def decimal_to_hex_width(decimal_num, width=16):
    hex_string = hex(decimal_num).replace("0x", "")
    hex_string = hex_string.zfill(width)
    return hex_string

    


class pipeline_regs:
    def __init__(self,  predPC=0, stat="", icode=0, ifun=0, rA=0, rB=0,
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
    
    # def set_cc(self, ifun):
    #     SF = pro_st.SF
    #     OF = pro_st.OF
    #     ZF = pro_st.ZF
    #     if ifun == 0:  # no self.cndition
    #         self.cnd = 1
    #     elif ifun == 1:  # le self.cndi = (SF^OF)|ZF
    #         self.cnd = (SF ^ OF) | ZF
    #     elif ifun == 2:  # l self.cndi = SF ^ OF
    #         self.cnd = SF ^ OF
    #     if ifun == 3:  # e ZF
    #         self.cnd = ZF
    #     if ifun == 4:  # ne
    #         self.cnd = 1 - ZF
    #     if ifun == 5:  # ge
    #         self.cnd = 1 - (SF ^ OF)
    #     if ifun == 6:  # g
    #         self.cnd = (1 - (SF ^ OF) )& (1 - ZF) 

# processor
class processor:
    mem = ['00'] * 500

    pc = 0
    rf = [0] * 16
    # cc
    ZF = 0
    SF = 0
    OF = 0
    # init the pipline regs between stages
    wb_if_regs = pipeline_regs()
    
    if_id_regs = pipeline_regs()
    
    id_ex_regs = pipeline_regs()
    
    ex_mem_regs = pipeline_regs()
    
    mem_wb_regs = pipeline_regs()
    
    def __init__(self, filename):
        self.load_bytes_program(filename)
        
        pass

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
    
    def readmem(self, baseaddr):
        datastr = ''
        for offset in range(8):
            memaddr = baseaddr + offset
            datastr = datastr + self.mem[memaddr]
        return lendhex_to_dec(datastr)
    
    def writemem(self, baseaddr, val):
        datalist = split_string_by_length(decimal_to_hex_width(val), 2)
        datalist.reverse()  # little end
        for offset in range(8):
            maddr = baseaddr + offset
            self.mem[maddr] = datalist[offset]
    
    def cycle(self):
        self.fetch()
        self.decode()
        self.execute()
        self.memory()
        self.write_back()
        self.update_pc()
    
    def fetch(self):
        pass
    
    def decode(self):
        pass
    
    def execute(self):
        pass
    
    def memory(self):
        pass
    
    def write_back(self):
        pass
    
    def update_pc(self):
        pass
        

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



