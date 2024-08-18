I might add these to a repository eventually but I don't want to think about all their licenses right now.

Anyway, you will need these files:

windepend
├── include
│   ├── GL
│   │   ├── eglew.h
│   │   ├── glew.h
│   │   ├── glxew.h
│   │   └── wglew.h
│   ├── vlc
│   │   ├── deprecated.h
│   │   ├── libvlc\_dialog.h
│   │   ├── libvlc\_events.h
│   │   ├── libvlc.h
│   │   ├── libvlc\_media\_discoverer.h
│   │   ├── libvlc\_media.h
│   │   ├── libvlc\_media\_library.h
│   │   ├── libvlc\_media\_list.h
│   │   ├── libvlc\_media\_list\_player.h
│   │   ├── libvlc\_media\_player.h
│   │   ├── libvlc\_renderer\_discoverer.h
│   │   ├── libvlc\_version.h
│   │   ├── libvlc\_vlm.h
│   │   ├── plugins
│   │   │   ├── vlc\_about.h
│   │   │   ├── vlc\_access.h
│   │   │   ├── vlc\_actions.h
│   │   │   ├── vlc\_addons.h
│   │   │   ├── vlc\_aout.h
│   │   │   ├── vlc\_aout\_volume.h
│   │   │   ├── vlc\_arrays.h
│   │   │   ├── vlc\_atomic.h
│   │   │   ├── vlc\_avcodec.h
│   │   │   ├── vlc\_bits.h
│   │   │   ├── vlc\_block.h
│   │   │   ├── vlc\_block\_helper.h
│   │   │   ├── vlc\_boxes.h
│   │   │   ├── vlc\_charset.h
│   │   │   ├── vlc\_codec.h
│   │   │   ├── vlc\_common.h
│   │   │   ├── vlc\_config\_cat.h
│   │   │   ├── vlc\_config.h
│   │   │   ├── vlc\_configuration.h
│   │   │   ├── vlc\_cpu.h
│   │   │   ├── vlc\_demux.h
│   │   │   ├── vlc\_dialog.h
│   │   │   ├── vlc\_epg.h
│   │   │   ├── vlc\_es.h
│   │   │   ├── vlc\_es\_out.h
│   │   │   ├── vlc\_events.h
│   │   │   ├── vlc\_filter.h
│   │   │   ├── vlc\_fingerprinter.h
│   │   │   ├── vlc\_fourcc.h
│   │   │   ├── vlc\_fs.h
│   │   │   ├── vlc\_gcrypt.h
│   │   │   ├── vlc\_httpd.h
│   │   │   ├── vlc\_http.h
│   │   │   ├── vlc\_image.h
│   │   │   ├── vlc\_inhibit.h
│   │   │   ├── vlc\_input.h
│   │   │   ├── vlc\_input\_item.h
│   │   │   ├── vlc\_interface.h
│   │   │   ├── vlc\_interrupt.h
│   │   │   ├── vlc\_keystore.h
│   │   │   ├── vlc\_main.h
│   │   │   ├── vlc\_md5.h
│   │   │   ├── vlc\_media\_library.h
│   │   │   ├── vlc\_memstream.h
│   │   │   ├── vlc\_messages.h
│   │   │   ├── vlc\_meta\_fetcher.h
│   │   │   ├── vlc\_meta.h
│   │   │   ├── vlc\_mime.h
│   │   │   ├── vlc\_modules.h
│   │   │   ├── vlc\_mouse.h
│   │   │   ├── vlc\_mtime.h
│   │   │   ├── vlc\_network.h
│   │   │   ├── vlc\_objects.h
│   │   │   ├── vlc\_opengl.h
│   │   │   ├── vlc\_picture\_fifo.h
│   │   │   ├── vlc\_picture.h
│   │   │   ├── vlc\_picture\_pool.h
│   │   │   ├── vlc\_playlist.h
│   │   │   ├── vlc\_plugin.h
│   │   │   ├── vlc\_probe.h
│   │   │   ├── vlc\_rand.h
│   │   │   ├── vlc\_renderer\_discovery.h
│   │   │   ├── vlc\_services\_discovery.h
│   │   │   ├── vlc\_sout.h
│   │   │   ├── vlc\_spu.h
│   │   │   ├── vlc\_stream\_extractor.h
│   │   │   ├── vlc\_stream.h
│   │   │   ├── vlc\_strings.h
│   │   │   ├── vlc\_subpicture.h
│   │   │   ├── vlc\_text\_style.h
│   │   │   ├── vlc\_threads.h
│   │   │   ├── vlc\_timestamp\_helper.h
│   │   │   ├── vlc\_tls.h
│   │   │   ├── vlc\_url.h
│   │   │   ├── vlc\_variables.h
│   │   │   ├── vlc\_video\_splitter.h
│   │   │   ├── vlc\_viewpoint.h
│   │   │   ├── vlc\_vlm.h
│   │   │   ├── vlc\_vout\_display.h
│   │   │   ├── vlc\_vout.h
│   │   │   ├── vlc\_vout\_osd.h
│   │   │   ├── vlc\_vout\_window.h
│   │   │   ├── vlc\_xlib.h
│   │   │   └── vlc\_xml.h
│   │   └── vlc.h
│   └── vlcpp
│       ├── common.hpp
│       ├── Dialog.hpp
│       ├── Equalizer.hpp
│       ├── EventManager.hpp
│       ├── Instance.hpp
│       ├── Internal.hpp
│       ├── MediaDiscoverer.hpp
│       ├── Media.hpp
│       ├── MediaLibrary.hpp
│       ├── MediaList.hpp
│       ├── MediaListPlayer.hpp
│       ├── MediaPlayer.hpp
│       ├── Picture.hpp
│       ├── RendererDiscoverer.hpp
│       ├── structures.hpp
│       └── vlc.hpp
├── lib
│   ├── glew32.dll
│   ├── libgcc\_s\_seh-1.dll
│   ├── libstdc++-6.dll
│   ├── libvlccore.dll
│   ├── libvlc.dll
│   ├── libwinpthread-1.dll
│   ├── opengl32.dll
│   └── SDL2.dll
└── plugins
    ├── access
    │   ├── libaccess\_concat\_plugin.dll
    │   ├── libaccess\_imem\_plugin.dll
    │   ├── libaccess\_mms\_plugin.dll
    │   ├── libaccess\_realrtsp\_plugin.dll
    │   ├── libaccess\_srt\_plugin.dll
    │   ├── libaccess\_wasapi\_plugin.dll
    │   ├── libattachment\_plugin.dll
    │   ├── libbluray-awt-j2se-1.3.2.jar
    │   ├── libbluray-j2se-1.3.2.jar
    │   ├── libcdda\_plugin.dll
    │   ├── libdcp\_plugin.dll
    │   ├── libdshow\_plugin.dll
    │   ├── libdtv\_plugin.dll
    │   ├── libdvdnav\_plugin.dll
    │   ├── libdvdread\_plugin.dll
    │   ├── libfilesystem\_plugin.dll
    │   ├── libftp\_plugin.dll
    │   ├── libhttp\_plugin.dll
    │   ├── libhttps\_plugin.dll
    │   ├── libidummy\_plugin.dll
    │   ├── libimem\_plugin.dll
    │   ├── liblibbluray\_plugin.dll
    │   ├── liblive555\_plugin.dll
    │   ├── libnfs\_plugin.dll
    │   ├── librist\_plugin.dll
    │   ├── librtp\_plugin.dll
    │   ├── libsatip\_plugin.dll
    │   ├── libscreen\_plugin.dll
    │   ├── libsdp\_plugin.dll
    │   ├── libsftp\_plugin.dll
    │   ├── libshm\_plugin.dll
    │   ├── libsmb\_plugin.dll
    │   ├── libtcp\_plugin.dll
    │   ├── libtimecode\_plugin.dll
    │   ├── libudp\_plugin.dll
    │   ├── libvcd\_plugin.dll
    │   ├── libvdr\_plugin.dll
    │   └── libvnc\_plugin.dll
    ├── audio\_output
    │   ├── libadummy\_plugin.dll
    │   ├── libafile\_plugin.dll
    │   ├── libamem\_plugin.dll
    │   ├── libdirectsound\_plugin.dll
    │   ├── libmmdevice\_plugin.dll
    │   ├── libwasapi\_plugin.dll
    │   └── libwaveout\_plugin.dll
    └── codec
        ├── liba52\_plugin.dll
        ├── libadpcm\_plugin.dll
        ├── libaes3\_plugin.dll
        ├── libaom\_plugin.dll
        ├── libaraw\_plugin.dll
        ├── libaribsub\_plugin.dll
        ├── libavcodec\_plugin.dll
        ├── libcc\_plugin.dll
        ├── libcdg\_plugin.dll
        ├── libcrystalhd\_plugin.dll
        ├── libcvdsub\_plugin.dll
        ├── libd3d11va\_plugin.dll
        ├── libdav1d\_plugin.dll
        ├── libdca\_plugin.dll
        ├── libddummy\_plugin.dll
        ├── libdmo\_plugin.dll
        ├── libdvbsub\_plugin.dll
        ├── libdxva2\_plugin.dll
        ├── libedummy\_plugin.dll
        ├── libfaad\_plugin.dll
        ├── libflac\_plugin.dll
        ├── libfluidsynth\_plugin.dll
        ├── libg711\_plugin.dll
        ├── libjpeg\_plugin.dll
        ├── libkate\_plugin.dll
        ├── liblibass\_plugin.dll
        ├── liblibmpeg2\_plugin.dll
        ├── liblpcm\_plugin.dll
        ├── libmft\_plugin.dll
        ├── libmpg123\_plugin.dll
        ├── liboggspots\_plugin.dll
        ├── libopus\_plugin.dll
        ├── libpng\_plugin.dll
        ├── libqsv\_plugin.dll
        ├── librawvideo\_plugin.dll
        ├── librtpvideo\_plugin.dll
        ├── libschroedinger\_plugin.dll
        ├── libscte18\_plugin.dll
        ├── libscte27\_plugin.dll
        ├── libsdl\_image\_plugin.dll
        ├── libspdif\_plugin.dll
        ├── libspeex\_plugin.dll
        ├── libspudec\_plugin.dll
        ├── libstl\_plugin.dll
        ├── libsubsdec\_plugin.dll
        ├── libsubstx3g\_plugin.dll
        ├── libsubsusf\_plugin.dll
        ├── libsvcdsub\_plugin.dll
        ├── libt140\_plugin.dll
        ├── libtextst\_plugin.dll
        ├── libtheora\_plugin.dll
        ├── libttml\_plugin.dll
        ├── libtwolame\_plugin.dll
        ├── libuleaddvaudio\_plugin.dll
        ├── libvorbis\_plugin.dll
        ├── libvpx\_plugin.dll
        ├── libwebvtt\_plugin.dll
        ├── libx26410b\_plugin.dll
        ├── libx264\_plugin.dll
        ├── libx265\_plugin.dll
        └── libzvbi\_plugin.dll

Everything in windepend/include is regular header stuff.

For lib/plugin, you can find everything in what VLC ships.

Create a directory called winbuild. This is where you will find the executable.

After you have everything here, then you can build with `make windows`.
