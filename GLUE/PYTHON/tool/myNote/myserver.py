#!/usr/bin/python3

import os
import os.path
import sys

abspath = os.path.abspath(__file__)
absdir = os.path.dirname(abspath)
pyhttp = "python3 -m http.server"

output_type = "svg"

for arg in sys.argv[1:]:
    if len(arg) >= 2 and arg[:2] == "-t":
        output_type = arg[2:]

abspath_source_dir = os.path.abspath(".")

running_dir = abspath_source_dir
while running_dir != "/":
    if os.path.exists(running_dir+"/.mynote"):
        break
    running_dir = os.path.dirname(running_dir)

if running_dir == "/":
    running_dir = ""

output_dir = abspath_source_dir.replace(running_dir, "",1) 
running_dir = running_dir + "/output/"+output_type

print("http://127.0.0.1:8000"+output_dir)
os.system(pyhttp + " -d " + running_dir + " >/dev/null 2>&1 &")
