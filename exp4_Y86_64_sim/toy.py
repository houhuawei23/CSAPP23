mem = [0]*1000
reg = [0]*10
iReg = 0
pReg = 0

def loadProgram(file):
    global mem, reg, iReg, pReg
    fil = open(file,'r')
    first = True
    lineno = 0
    while True:
        line = fil.readline()
        lineno += 1
        if line == '':
            break
        fids = line.split()
        try:
            address = int(fids[0])
            instruc = int(fids[1])
            if first:
                pReg = address
                first = False
            mem[address] = instruc
        except:
            print(f'File {file} line {lineno} has error')
            pass
    fil.close

def cycle():
    global mem, reg, iReg, pReg

    iReg = mem[pReg]
    pReg = pReg + 1

    opcode = (iReg//10000)
    r = (iReg//1000) % 10
    addr = (iReg) % 1000

    if opcode == 0:
        return False
    elif opcode == 1:       # mov1 Rx Ay
        reg[r] = mem[addr]
    elif opcode == 2:       # mov2 Ay Rx
        mem[addr] = reg[r]
    elif opcode == 3:       # mov3 Rx n
        reg[r] = addr
    elif opcode == 4:       # mov4 Rx (Ry)
        reg[r] = mem[reg[addr]]
    elif opcode == 5:       # add Rx Ry
        reg[r] = reg[r] + reg[addr]
    elif opcode == 6:       # sub Rx Ry
        reg[r] = reg[r] - reg[addr]
    elif opcode == 7:       # mul Rx Ry
        reg[r] = reg[r] * reg[addr]
    elif opcode == 8:       # div Rx Ry
        reg[r] = reg[r] // reg[addr]
    elif opcode == 10:      # jmp Ax
        pReg = addr
    elif opcode == 11:      # jz Rx Ay
        if reg[r] == 0:
            pReg = addr
    else:
        print(f'Unknow opcode {opcode}')
    return True

def run(file):
    global mem, reg, iReg, pReg

    loadProgram(file)

    while True:
        if not cycle():
            break

run('test.toy')


