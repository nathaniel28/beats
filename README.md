# Beats
Beats is a vertical scrolling rhythm game similar to [Rhythm Plus](https://github.com/henryzt/Rhythm-Plus-Music-Game). It's not done yet!
```
Usage: ./beats [OPTION]... <CHART>
Play a .chart file. Options are in milliseconds unless otherwise specified.
  -t, --view-timespan [=650]      How far into the future you should be able to
                                    see. Smaller view timespan equals faster
                                    falling notes. 650 ms is about 4x speed in
                                    Rhythm Plus.
  -a, --audio-offset [=0]         Makes notes fall earlier (if positive) or
                                    later (if negative). Use it to calibrate
                                    delay for bluetooth headphones.
  -v, --video-offset [=0]         Makes you have to hit the note earlier (if
                                    positive) or later (if negative).
  -s, --strike-timespan [=250]    Smaller strike timespan equals easier game.
  -d, --min-initial-delay [=800]  If a song has notes before this timestamp,
                                    delay playback.
  -x [=100]                       Length of each column (total screen length is
                                    4 times this if you have 4 columns).
  -y [=600]                       Height of each column.
      --note-height [=8]          Height of each non hold note
      --no-vsync                  Disable vsync.
```
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
* Textures for notes (but honestly, solid color looks pretty good)
* Clean up the code! This is difficult because OpenGL is a giant state machine, and doesn't abstract into nice classes very easily. I think helper functions are the way to go, but since they all need to access a million variables, keeping all the mess in one scope is convenient, and I don't see a state struct helping at all.
* Use less unsigned integers and more signed ones
## Stuff that has been done
* Shaders!
* Keybindings are modifiable, but the current values are hardcoded
* Command line arguments, through argparse instead of getopt/getopt\_long/something like normal due to portability concerns.
* Sound
* Up to 8 columns of notes are supported (limited by a shader)
* The speed at which notes fall can be adjusted (though the value is hardcoded)
* The audio/note offset can be adjusted (though the value is also hardcoded)
* Keypresses are scored by how close you got to the next note
## Compiling
### For Linux
For the first time, run `make all`.

Run `make shaders.h` if you update any of shaders/*

You need development libraries for SDL, OpenGL, GLEW, and libVLC (don't forget the C++ bindings by libvlcpp).

You will probably need Dear ImGui in the future.
### For Windows
See README-WIN.md

Note that this is instructions on how to compile *for* Windows, not *under* Windows. I don't know how to compile anything under Windows.

The resulting executable and dependencies are ~100 MB (about 64 times larger than the Linux build). This is due mostly to libvlc plugins, so if you have VLC installed already, you can probably do something with the VLC\_PLUGIN\_PATH environment variable to avoid needing the plugins I ship. Alternatively, create a symlink from C:\\Program Files\\VideoLAN\\VLC\\plugins to achieve the same goal.
## In defense of the code...
*This section intentionally left blank.*
