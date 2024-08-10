# Beats
Beats is a rhythm game similar to [Rhythm Plus](https://github.com/henryzt/Rhythm-Plus-Music-Game). It's not done yet!
## Stuff that still needs to be done
* proper hold note scoring and note misses
* A pause menu to change note scroll speed, and audio/note offset, song selection...
* Charts and the accompanying music are kept in different files; I need a better method of finding the music given a chart as the current solution is just looking for foo given the chart foo.chart. This will likely mean another update to the .chart file format so it can include a music file to look for.
* A config file for keybindings etc.
* Windows release build, when I get around to finishing the majority of this section
* Textures for notes (but honestly, solid color looks pretty good)
* Clean up the code! This is difficult because OpenGL is a giant state machine, and doesn't abstract into nice classes very easily. I think helper functions are the way to go, but since they all need to access a million variables, keeping all the mess in one scope is convenient, and I don't see a state struct helping at all.
## Stuff that has been done
* Shaders!
* Keybindings are modifiable, but the current values are hardcoded
* Sound
* Up to 8 columns of notes are supported (limited by a shader)
* The speed at which notes fall can be adjusted (though the value is hardcoded)
* The audio/note offset can be adjusted (though the value is also hardcoded)
* Keypresses are scored by how close you got to the next note
* You can import charts from Rhythm Plus by using converter.py on the chart JSON found via a GET request to https://api.rhythm-plus.com/api/v1/sheet/get?sheetId= [your sheet id here]. Use the network tab of your browser's console to find this request. I might eventually get around to making a tool that does this automatically...
## Compiling
For the first time, run `make all`.

Run `make shaders.h` if you update any of shaders/*

You need development libraries for SDL, OpenGL, GLEW, and libVLC (don't forget the C++ bindings).

You will probably need Dear ImGui in the future.
## In defense of the code...
*This section intentionally left blank.*
