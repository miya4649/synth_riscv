#!/usr/bin/python3
# -*- coding: utf-8 -*-
import sys, os, struct, re, codecs, math

filename = 'out.txt'
tablesize = 256
valuesize = 16383

file = codecs.open(filename, 'w', 'utf-8')
file.write('short sintable[] = {\n')
for i in range(tablesize >> 3):
  file.write('  ')
  for j in range(8):
    k = i * 8 + j
    value = math.floor(math.sin(math.pi * 2.0 / tablesize * k) * valuesize)
    file.write(str(value) + ',')
  file.write('\n')
file.write('};\n')
file.close()
