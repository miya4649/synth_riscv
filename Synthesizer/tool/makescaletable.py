#!/usr/bin/python3
# -*- coding: utf-8 -*-
import sys, os, struct, re, codecs, math

filename = 'out.txt'
table_size = 16
wave_buffer_size = 256

file = codecs.open(filename, 'w', 'utf-8')
file.write('int scaletable[] = {\n')
for i in range(table_size >> 3):
  file.write('  ')
  for j in range(8):
    k = i * 8 + j
    value = math.floor(math.pow(2.0, float(k) / 12.0) * 440.0 * (float(0x100000000) / 48000.0 / float(wave_buffer_size)));
    file.write(str(value) + ',')
  file.write('\n')
file.write('};\n')
file.close()
