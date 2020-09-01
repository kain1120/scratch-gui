#!/usr/bin/python
import os, sys, getopt

def main(argv):
    layer_num = 0
    uml_file = '' 
    try:
        opts, args = getopt.gnu_getopt(argv, "hl:", ["help", "layer="])
    except getopt.GetoptError:
        sys.exit(2)
    for opt, arg in opts:
        if opt in ("-h", "--help"):
            print 'plantuml.py -l <layer_number> -f <uml_file>'
            sys.exit()
        elif opt in ("-l", "--layer"):
            layer_num = arg

    for arg in args:
        uml_file = arg
        
    print "uml_file:", uml_file
    print "layer:", layer_num

    #cmd = "java -jar /root/tool/plantuml/plantuml.jar -tsvg " + uml_file
    #res = os.popen(cmd)
    #output = res.read()
    #print(output)

if __name__ == "__main__":
    main(sys.argv[1:])
