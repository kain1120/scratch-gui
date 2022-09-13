#!/usr/bin/python3

import os
import os.path
import sys
import re

abspath = os.path.abspath(__file__)
absdir = os.path.dirname(abspath)

my_args = []
for arg in sys.argv[1:]:
    if len(arg) >= 2 and arg[:2] == "-t":
        output_type = arg[2:]
    elif arg == "make":
        action = arg
    elif arg == "clean":
        action = arg
    elif arg == "all":
        source_file="*"
    else:
        source_file=arg

if 'output_type' not in dir():
    output_type = "svg"

if 'source_file' not in dir():
    source_file = "*"

if 'action' not in dir():
    action = "clean"

abspath_source_file = os.path.abspath(".")
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

image_dot_re = re.compile(r'image="\./(.*)\.png')
def find_dep(dot_file):
    result = []
    with open(dot_file) as f:
        result = re.findall(image_dot_re, f.read())
        f.close()
    return result;

def make_dot(dot_file, elems_dir, output_elems_dir):
    print(dot_file)
    dep_dots = find_dep(elems_dir+"/"+dot_file+".dot")
    for dep in dep_dots:
        make_dot(dep, elems_dir, output_elems_dir)
    os.system("cd " + elems_dir + " && " + dot + " " + "-T png " + elems_dir + "/" + dot_file + ".dot -o " + dot_file + ".png") 
    os.system("cd " + elems_dir + " && " + dot + " " + "-T png " + elems_dir + "/" + dot_file + ".dot -o " + output_elems_dir + "/" + dot_file + ".png") 


dot = "dot"
elements_dir = "dot/elements"
elems_dir = note_path + "/" + elements_dir
output_elems_dir = note_path + "/output/" + output_type + "/" + elements_dir
if not os.path.exists(output_elems_dir):
    os.system("mkdir -p " + output_elems_dir)

if action == "clean":
    os.system("rm "+output_elems_dir + "/" + source_file + ".png")
    os.system("rm "+elems_dir + "/" + source_file + ".png")
elif action == "make":
    if source_file == "*":
        elems = os.listdir(elems_dir)
        dots = [os.path.splitext(elem)[0] for elem in elems if os.path.splitext(elem)[-1] == ".dot"]
        for dot_file in dots:
            make_dot(dot_file, elems_dir, output_elems_dir)
    else:
        make_dot(source_file, elems_dir, output_elems_dir)
