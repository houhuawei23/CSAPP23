mem = [''] * 1000
reg = [0] * 10
pReg = 0
iReg = ''


def loadProgram(file):
    global pReg, iReg, reg, mem
    fil = open(file, 'r')
    first = True
    while True:
        line = fil.readline()
        if line == '':
            break
        fids = line.split()
        address = int(fids[0])
        instruc = fids[1]
        for fld in fids[2: len(fids)]:
            instruc = instruc + ' ' + fld
        mem[address] = instruc
        if first:
            pReg = address
            first = False
    fil.close()


def cycle():
    global pReg, iReg, reg, mem

    # 取指令
    iReg = mem[pReg]
    pReg = pReg + 1

    # 译码
    flds = iReg.split()
    opcode = flds[0].lower()    # 操作码
    if len(flds) > 1:
        op1 = int(flds[1])      # 操作数1
    if len(flds) > 2:
        op2 = int(flds[2])      # 操作数2

    # 执行和写结果
    if opcode == 'mrmov':        # mrmovq
        reg[op1] = mem[op2]
    elif opcode == 'rmmov':      # rmmovq
        mem[op1] = reg[op2]
    elif opcode == 'immov':      # immovq
        reg[op1] = op2
    elif opcode == 'add':
        reg[op1] = reg[op1] + reg[op2]
    elif opcode == 'sub':
        reg[op1] = reg[op1] - reg[op2]
    elif opcode == 'mul':
        reg[op1] = reg[op1] * reg[op2]
    elif opcode == 'div':
        reg[op1] = reg[op1] // reg[op2]
    elif opcode == 'jmp':
        pReg = op1
    elif opcode == 'jz':
        if reg[op1] == 0:
            pReg = op2
    elif opcode == 'in':
        reg[op1] = int(input('input:'))
    elif opcode == 'out':
        print('output:', reg[op1])
    elif opcode == 'halt':
        return False
    else:
        print(f'Unknown opcode {opcode}')

    return True


def run(file):
    global pReg, iReg, reg, mem
    loadProgram(file)

    while True:
        hasNextInstruc = cycle()
        if hasNextInstruc == False:
            break


run('test.toy2')
