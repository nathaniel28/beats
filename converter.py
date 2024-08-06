#!/usr/bin/python3

# under GPL-2.0

import ctypes
import json
import sys

OUTPUT_MAGIC = ctypes.c_uint64(0xF1E0007472616863)
OUTPUT_VERSION = ctypes.c_uint32(0)

KEY_CHART = {"d": 1, "f": 2, "j": 4, "k": 8}

def reencode(src, dst):
    with open(src, "r") as inputfp, open(dst, "bw") as outputfp:
        chart = json.load(inputfp)
        if chart.get("version") != 1:
            print("unrecognized version")
            return
        mapping = chart.get("mappings")
        if mapping == None:
            print("no mapping found")
            return
        outputfp.write(OUTPUT_MAGIC)
        outputfp.write(OUTPUT_VERSION)
        outputfp.write(ctypes.c_uint32(len(mapping)))
        for beat in mapping:
            timestamp = beat.get("t")
            if timestamp == None:
                print("beat missing timestamp")
                return
            keys = beat.get("k")
            if keys == None:
                print("beat missing keys")
                return
            # we're ignoring held keys (in beat.get("h")) for now
            columns = 0
            for key in keys:
                c = KEY_CHART.get(key)
                if c == None:
                    print("beat contains unknown key")
                    return
                columns |= c
            timestamp = round(timestamp * 1000) # seconds to milliseconds
            outputfp.write(ctypes.c_uint64(timestamp))
            outputfp.write(ctypes.c_uint32(columns))

if len(sys.argv) != 3:
    print("expected 2 arguments: src, dst")
else:
    reencode(sys.argv[1], sys.argv[2])
