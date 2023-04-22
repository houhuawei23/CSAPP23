# -*- coding: utf-8 -*-
"""
Created on Sun Apr  9 00:11:43 2023

@author: hhw
"""

class Memory:
    def __init__(self, size=1024):
        self.memory = bytearray(size)

    def read(self, address, size=4):
        if address < 0 or address + size > len(self.memory):
            raise MemoryError("Invalid memory address")
        value = 0
        for i in range(size):
            value |= self.memory[address + i] << (8 * i)
        return value

    def write(self, address, value, size=4):
        if address < 0 or address + size > len(self.memory):
            raise MemoryError("Invalid memory address")
        for i in range(size):
            self.memory[address + i] = (value >> (8 * i)) & 0xFF
