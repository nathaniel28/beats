# Beats
Beats is a rhythm game similar to Rhythm Plus. It's not done yet!
## Stuff that still needs to be done
* Hold notes!
* Text rendering for score and such (more complicated than I though, likely via freetype)
* A pause menu to change note scroll speed, and audio/note offset, song selection...
* Support for more audio file formats
* Charts and the accompanying music are kept in different files; I need a better method of finding the music given a chart as the current solution is just looking for foo given the chart foo.chart
* Textures for notes
* A config file for keybindings etc.
* Clean up the code! This is difficult because OpenGL is a giant state machine, and doesn't abstract into nice classes very easily. I think helper functions are the way to go, but since they all need to access a million variables, keeping all the mess in one scope is convenient, and I don't see a state struct helping at all.
## Stuff that has been done
* Shaders!
* Keybindings are modifiable, but the current values are hardcoded
* Sound (some file formats are unsupported, like m4a; this is a limitation of SDL2\_mixer, so I may at some point move to an ffmpeg-based solution)
* The class Chart supports up to 32 columns of notes (though the UI doesn't yet)
* The speed at which notes fall can be adjusted (though the value is hardcoded)
* The audio/note offset can be adjusted (though the value is also hardcoded)
* Keypresses are scored by how close you got to the next note
* You can import charts from Rhythm Plus by using converter.py on the chart JSON found via a GET request to https://api.rhythm-plus.com/api/v1/sheet/get?sheetId= [your sheet id here]. Use the network tab of your browser's console to find this request. I might eventually get around to making a tool that does this automatically...
## Compiling
For the first time, run `make all`.

Run `make shaders.h` if you update any of shaders/*

You need development libraries for SDL, SDL\_image, SDL\_mixer, OpenGL, and GLEW.

You will probably need Dear ImGui in the future.
## In defense of the code...
*This section intentionally left blank.*
