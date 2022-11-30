#!/bin/sh

pycrc --width 16 --poly 0x3D65 --reflect-in True --xor-in 0x0000 --reflect-out True --xor-out 0xFFFF --algorithm table-driven --generate h -o crc-table.h 
pycrc --width 16 --poly 0x3D65 --reflect-in True --xor-in 0x0000 --reflect-out True --xor-out 0xFFFF --algorithm table-driven --generate c -o crc-table.c
pycrc --width 16 --poly 0x3D65 --reflect-in True --xor-in 0x0000 --reflect-out True --xor-out 0xFFFF --algorithm  bit-by-bit-fast --generate h -o crc-shift.h
pycrc --width 16 --poly 0x3D65 --reflect-in True --xor-in 0x0000 --reflect-out True --xor-out 0xFFFF --algorithm  bit-by-bit-fast --generate c -o crc-shift.c
