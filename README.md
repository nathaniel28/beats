# Beats
Beats is a rhythm game similar to Rhythm Plus. It's not done yet!
## Stuff that still needs to be done
* Sound! (via SDL\_mixer)
* Hold notes!
* Text rendering for score and such (more complicated than I though, via freetype)
* A pause menu to change note scroll speed, and audio/note offset, song selection...
* Non hardcoded keybindings
## Stuff that has been done
* Parses the chart binary format
* The class Chart supports arbitrary columns of notes (though the UI doesn't yet)
* The speed at which notes fall can be adjusted (though the value is hardcoded)
* Keypresses are scored by how close you got to the next note
* You can import charts from Rhythm Plus by using converter.py on the chart JSON found via a GET request to https://api.rhythm-plus.com/api/v1/sheet/get?sheetId= [your sheet id here]. Use the network tab of your browser's console to find this request. I might eventually get around to making a tool that does this automatically...
## Compiling
Run `make`.

You need development libraries for SDL, SDL\_image, and SDL\_mixer.

You will probably need freetype and OpenGL in the future.
