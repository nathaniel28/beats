# Beats
Beats is a vertical scrolling rhythm game similar to [Rhythm Plus](https://github.com/henryzt/Rhythm-Plus-Music-Game). It's not done yet!
## Acquiring Charts
This software does not have the capability to create charts. Instead, you must convert a chart from another rhythm game into the custom .chart format (further documentation by the Chart::deserialize method).

Currently, there is only support (via converter.py) to convert the JSON format used by Rhythm Plus into the .chart format. The import process goes as follows: Download the the chart JSON found via a GET request to https://api.rhythm-plus.com/api/v1/sheet/get?sheetId= [your sheet id here]. Use the network tab of your browser's console to find this request. Select it, then select the "Response" tab, right click the "mapping" section, select "Copy Value", then paste it in a file and save it. Then, run `./converter.py <name-of-input.json> <name-of-output.chart>`, and you'll have a proper .chart file.

Next, you'll need the music for it. If you have the file already, skip the next step.

I do not promote nor condone downloading music unlawfully. On a completely unrelated note, it's pretty easy to download a YouTube video: I recommend using [yt-dlp](https://github.com/yt-dlp/yt-dlp), in which case you would use the command `yt-dlp -x <video URL>` to download the audio only.

As a limitation of my code that will eventually be removed, you must name the music file whatever you named the .chart file, without the .chart part (ie `foo.chart` would have its music in `foo`). This means removing the .mp3/.ogg/whatever file extension the music file had.
## Stuff that still needs to be done
* Adjust and verify scoring notes is done properly.
* A pause menu to change note scroll speed, and audio/note offset, song selection...
* Charts and the accompanying music are kept in different files; I need a better method of finding the music given a chart as the current solution is just looking for foo given the chart foo.chart. This will likely mean another update to the .chart file format so it can include a music file to look for.
* A config file for keybindings etc.
* Windows release build, when I get around to finishing the majority of this section
* Textures for notes (but honestly, solid color looks pretty good)
* Clean up the code! This is difficult because OpenGL is a giant state machine, and doesn't abstract into nice classes very easily. I think helper functions are the way to go, but since they all need to access a million variables, keeping all the mess in one scope is convenient, and I don't see a state struct helping at all.
* Use less unsigned integers and more signed ones
## Stuff that has been done
* Shaders!
* Keybindings are modifiable, but the current values are hardcoded
* Sound
* Up to 8 columns of notes are supported (limited by a shader)
* The speed at which notes fall can be adjusted (though the value is hardcoded)
* The audio/note offset can be adjusted (though the value is also hardcoded)
* Keypresses are scored by how close you got to the next note
## Compiling
### On Linux
For the first time, run `make all`.

Run `make shaders.h` if you update any of shaders/*

You need development libraries for SDL, OpenGL, GLEW, and libVLC (don't forget the C++ bindings).

You will probably need Dear ImGui in the future.
### On Windows
The code should be cross platform. However, I don't know how to build it on Windows, so you're on your own. Good luck!
## In defense of the code...
*This section intentionally left blank.*
