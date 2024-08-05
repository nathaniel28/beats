#!/usr/bin/python3

import sys
import re

def process(src, outputfp):
    with open(src, "r") as inputfp:
        contents = inputfp.read()

        src = src.split('/')[-1]
        shaderName = re.sub("\\.", "_", src)
        shaderSrcName = shaderName + "_src"
        shaderFileName = shaderName.upper() + "_H"

        res = ""
        for ch in contents:
            match ch:
                case '\n':
                    res += '\\n"\n"'
                case '"':
                    res += '\\"'
                case _:
                    # if you have weird chars in your shader code,
                    # that's your problem
                    res += ch

        outputfp.write("const char ")
        outputfp.write(shaderSrcName)
        outputfp.write('[] =\n"')
        outputfp.write(res[:len(res) - 2]) # don't write the last " and \n
        outputfp.write(";\n")

# must be run from the project's main directory, sorry!
with open("shaders/sources.h", "w") as outputfp:
    outputfp.write("// this file was auto generated by process_shader.py\n// do not modify this file\n#ifndef SOURCES_H\n#define SOURCES_H\n")
    for arg in sys.argv[1:]:
        process(arg, outputfp)
    outputfp.write("#endif\n")
