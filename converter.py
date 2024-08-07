#!/usr/bin/python3

# under GPL-2.0

import ctypes
import json
import sys

OUTPUT_MAGIC = ctypes.c_uint64(0xF1E0007472616863)
OUTPUT_VERSION = ctypes.c_uint32(2)

class Note:
    def __init__(self, t, d):
        self.timestamp = t
        self.hold_duration = d

KEY_CHART = {"d": 0, "f": 1, "j": 2, "k": 3, "l": 4}
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

        totalColumns = 0
        notes = {}
        for note in mapping:
            timestamp = note.get("t")
            if timestamp == None:
                print("note missing timestamp")
                continue
            timestamp = round(timestamp * 1000) # seconds to milliseconds
            keys = note.get("k")
            if keys == None:
                print("note missing keys")
                continue
            held = note.get("h")
            for key in keys:
                col = KEY_CHART.get(key)
                if col == None:
                    print("note contains unrecognized key", key)
                    continue
                if col > totalColumns:
                    totalColumns = col
                duration = 0
                if held:
                    # strangely, some held durations are strings of floats and
                    # others are just floats, so I must convert just in case
                    duration = round(float(held.get(key, 0)) * 1000)
                if duration > timestamp:
                    duration -= timestamp
                notes.setdefault(col, []).append(Note(timestamp, duration))
        totalColumns += 1

        outputfp.write(OUTPUT_MAGIC)
        outputfp.write(OUTPUT_VERSION)
        outputfp.write(ctypes.c_uint32(totalColumns))
        for index, times in notes.items():
            outputfp.write(ctypes.c_uint32(index))
            outputfp.write(ctypes.c_uint32(len(times)))
            for time in times:
                outputfp.write(ctypes.c_uint32(time.timestamp))
                outputfp.write(ctypes.c_uint32(time.hold_duration))
            """
            print(key, end=" ")
            for time in times:
                print(f"({time.timestamp} {time.hold_duration})", end=" ")
            print()
            """

if len(sys.argv) != 3:
    print("expected 2 arguments: src, dst")
else:
    reencode(sys.argv[1], sys.argv[2])
