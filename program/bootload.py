#!/usr/bin/python

import serial
import time
import sys

startAddress = 0x00001000
blockSize = 1024
addrSize = 4

def splitBuf(b, seclen):
    if(len(b) % seclen):
        raise Exception("Buffer's length must be divisible by seclen")
    ret = []
    for i in range(len(b) / seclen):
        ret.append(b[seclen * i : seclen * (i+1)])
    return ret

def writeBuf(addr, buf, s):
    writestr = 'S' + ('%08x' % addr) + (''.join([('%02x' % b) for b in buf])) \
               + 'T'
    print "Writing\n%s" % writestr
    s.write(writestr)
    s.flush()

def readBuf(s):
    buf = []

    print "Reading"

    while(not len(buf) == (((blockSize + addrSize)*2) + 2)):
        readchar = s.read()
        sys.stdout.write(readchar)
        buf.append(readchar)

    if(not (buf[0] == 's' and buf[-1] == 't')):
        raise Exception("Invalid block sentinels")

    buf = buf[1:-1]

    addr = int(''.join(buf[:8]), 16)

    buf = buf[8:]

    buf = [int(''.join(x), 16) for x in splitBuf(buf, 2)]

    return addr, buf

def loadBinfile(f, s):
    with open(f, "r") as binfile:
        addr = startAddress

        # Read the bytes from the binary file
        binbytes = [ord(c) for c in binfile.read()]

        # Pad the buffer up to a multiple of 1024
        rem = len(binbytes) % blockSize
        if(rem):
            binbytes += ([0] * (blockSize - rem))

        # Split the buffer into 1024 byte chunks
        for buf in splitBuf(binbytes, blockSize):
            while(1):
                print "Programming block at addr 0x%08x" % addr
                time.sleep(0.1)
                writeBuf(addr, buf, s)
                if(s.read() == 'K'):
                    if((addr, buf) == readBuf(s)):
                        s.write('k')
                    else:
                        s.write('x')
                        continue
                    addr += blockSize
                else:
                    continue
                break

if(__name__ == '__main__'):

    print "Connecting to target..."

    s = serial.Serial("/dev/rfcomm2", 115200)

    while(not s.read() == '.'):
        pass

    s.write(' ')

    time.sleep(2)

    s.flushInput()

    loadBinfile("main.bin", s)

    s.write('q')
    s.close()

    print "Done!"

