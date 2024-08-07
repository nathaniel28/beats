#!/usr/bin/python3

# under GPL-2.0

import ctypes
import json
import sys

OUTPUT_MAGIC = ctypes.c_uint64(0xF1E0007472616863)
OUTPUT_VERSION = ctypes.c_uint32(1)

KEY_CHART = {"d": 1, "f": 2, "j": 4, "k": 8, "l": 16}
def columns(keys):
    columns = 0
    for key in keys:
        c = KEY_CHART.get(key)
        if c == None:
            print("beat contains unknown key")
            return 0
        columns |= c
    return columns

# returns the number of beats emitted
# this is greater than 1 if beats have different hold durations
def serialize(beat, fp):
    timestamp = beat.get("t")
    if timestamp == None:
        print("beat missing timestamp")
        return 0
    timestamp = round(timestamp * 1000) # seconds to milliseconds
    keys = beat.get("k")
    if keys == None:
        print("beat missing keys")
        return 0
    held = beat.get("h")
    if held:
        n = {}
        for key in keys:
            # strangely, some held durations are strings of floats and others
            # just floats, so I must convert just in case
            duration = round(float(held.get(key, 0)) * 1000)
            n[duration] = n.get(duration, "") + key
        i = 0
        for dur, k in n.items():
            cols = columns(k)
            if cols == 0:
                return i
            fp.write(ctypes.c_uint64(timestamp))
            fp.write(ctypes.c_uint32(dur - timestamp))
            fp.write(ctypes.c_uint32(cols))
            i += 1
        return i
    cols = columns(keys)
    if cols == 0:
        return 0
    fp.write(ctypes.c_uint64(timestamp))
    fp.write(ctypes.c_uint32(0))
    fp.write(ctypes.c_uint32(cols))
    return 1

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
        total = 0
        for beat in mapping:
            total += serialize(beat, outputfp)
        outputfp.seek(12) # to repace the length with the accurate one
        outputfp.write(ctypes.c_uint32(total))

if len(sys.argv) != 3:
    print("expected 2 arguments: src, dst")
else:
    reencode(sys.argv[1], sys.argv[2])
