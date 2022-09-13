#!/usr/bin/python3

import os
import os.path
import sys

abspath = os.path.abspath(__file__)
absdir = os.path.dirname(abspath)
puml = "java -jar " + absdir + "/plantuml.jar"

my_args = []

for arg in sys.argv[1:]:

    if len(arg) >= 2 and arg[:2] == "-t":
        output_type = arg[2:]
        my_args.append(arg)
    elif arg[0] != "-":
        if len(my_args) > 0 and (my_args[-1] == "-o" or my_args[-1] == "-output"):
            my_args.append(arg)
        else:
            source_file = arg
    else:
        my_args.append(arg)

if 'output_type' not in dir():
    output_type = "svg"
    my_args.append("-t"+output_type)

if 'source_file' not in dir():
    print("please provide the source file") 
    os.system(puml + " -h")
    sys.exit()

abspath_source_file = os.path.abspath(source_file)
abspath_source_dir  = os.path.dirname(abspath_source_file)

iuml_dir = abspath_source_dir
while iuml_dir.rfind("plantuml") != -1:
    if os.path.exists(iuml_dir+"/common.iuml"):
        my_args.append("-I"+iuml_dir+"/common.iuml")
        break
    else:
        iuml_dir = os.path.dirname(iuml_dir)

my_args.append("-Dhttp=\"http://127.0.0.1:8000\"")
rel_pos = abspath_source_dir.rfind("plantuml")
if rel_pos != -1:
    rel_pos += len("plantuml") +1
    if rel_pos < len(abspath_source_dir):
        rel_dir = abspath_source_dir[rel_pos:]
        my_args.append("-Dlocation="+rel_dir)

if "-o"not in my_args and "-output" not in my_args:
    output_dir = abspath_source_dir.replace("plantuml","plantuml/output/"+output_type,1) 
    os.system("mkdir -p "+output_dir)
    my_args.append("-o")
    my_args.append(output_dir)

my_args.append(source_file)

puml_cmd = puml + " " + " ".join(my_args)
os.system(puml_cmd)
