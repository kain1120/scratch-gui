#!/usr/bin/python3

import os
import os.path
import sys

abspath = os.path.abspath(__file__)
absdir = os.path.dirname(abspath)

my_args = []
for arg in sys.argv[1:]:
    if len(arg) >= 2 and arg[:2] == "-t":
        output_type = arg[2:]
    elif arg[0] != "-":
        source_file = arg
    else:
        my_args.append(arg)

if 'source_file' not in dir():
    print("please provide the source file") 
    sys.exit()

if 'output_type' not in dir():
    output_type = "svg"
my_args.append("-T "+output_type)

abspath_source_file = os.path.abspath(source_file)
abspath_source_dir  = os.path.dirname(abspath_source_file)

rel_path_num = 0
note_path = abspath_source_dir
while note_path != "/":
    if os.path.exists(note_path+"/.mynote"):
        break
    note_path = os.path.dirname(note_path)
    rel_path_num = rel_path_num + 1

if note_path == "/":
    note_path = ""

m4 = "m4"
elements_dir = "dot/elements"

m4_cmd = m4 + " --include=" + note_path + "/" +elements_dir 
if rel_path_num == 0:
    m4_cmd = m4_cmd + " --define=LOCATION=" + "./" + elements_dir
else:
    m4_cmd = m4_cmd + " --define=LOCATION=" + "../" * rel_path_num + elements_dir
m4_cmd = m4_cmd + " " + source_file
m4_cmd = m4_cmd + " >" + source_file + ".dot"

os.system(m4_cmd)

if "-o"not in my_args and "-output" not in my_args:
    output_dir = abspath_source_dir.replace(note_path, note_path+"/output/"+output_type,1) 
    os.system("mkdir -p "+output_dir)
    my_args.append("-o")
    my_args.append(output_dir+"/"+source_file+"."+output_type)

my_args.append(source_file + ".dot")

elems_dir = note_path + "/" + elements_dir
elems = os.listdir(elems_dir)
output_elems_dir = output_dir + "/" + "../" * rel_path_num + elements_dir
if not os.path.exists(output_elems_dir):
    os.system("mkdir -p " + output_elems_dir)
output_elems = os.listdir(output_elems_dir)

output_elems = [os.path.splitext(elem)[0] for elem in output_elems if os.path.splitext(elem)[-1] == ".png"]
dot_files = [os.path.splitext(elem)[0] for elem in elems if os.path.splitext(elem)[-1] == ".dot"]
png_files = [os.path.splitext(elem)[0] for elem in elems if os.path.splitext(elem)[-1] == ".png"]
dot_lefts = [dot for dot in dot_files if (dot not in output_elems or dot not in png_files)]

dot = "dot"

for dot_file in dot_lefts:
    os.system("mydotelem.py make " + dot_file)

dot_cmd = dot + " " + " ".join(my_args)
os.system(dot_cmd)
