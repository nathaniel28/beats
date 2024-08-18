I might add these to a repository eventually but I don't want to think about all their licenses right now.

Anyway, you will need these files:

```
windepend
├── include
│   ├── GL
│   │   ├── eglew.h
│   │   ├── glew.h
│   │   ├── glxew.h
│   │   └── wglew.h
│   ├── vlc
│   │   ├── deprecated.h
│   │   ├── libvlc_dialog.h
│   │   ├── libvlc_events.h
│   │   ├── libvlc.h
│   │   ├── libvlc_media_discoverer.h
│   │   ├── libvlc_media.h
│   │   ├── libvlc_media_library.h
│   │   ├── libvlc_media_list.h
│   │   ├── libvlc_media_list_player.h
│   │   ├── libvlc_media_player.h
│   │   ├── libvlc_renderer_discoverer.h
│   │   ├── libvlc_version.h
│   │   ├── libvlc_vlm.h
│   │   ├── plugins
│   │   │   ├── vlc_about.h
│   │   │   ├── vlc_access.h
│   │   │   ├── vlc_actions.h
│   │   │   ├── vlc_addons.h
│   │   │   ├── vlc_aout.h
│   │   │   ├── vlc_aout_volume.h
│   │   │   ├── vlc_arrays.h
│   │   │   ├── vlc_atomic.h
│   │   │   ├── vlc_avcodec.h
│   │   │   ├── vlc_bits.h
│   │   │   ├── vlc_block.h
│   │   │   ├── vlc_block_helper.h
│   │   │   ├── vlc_boxes.h
│   │   │   ├── vlc_charset.h
│   │   │   ├── vlc_codec.h
│   │   │   ├── vlc_common.h
│   │   │   ├── vlc_config_cat.h
│   │   │   ├── vlc_config.h
│   │   │   ├── vlc_configuration.h
│   │   │   ├── vlc_cpu.h
│   │   │   ├── vlc_demux.h
│   │   │   ├── vlc_dialog.h
│   │   │   ├── vlc_epg.h
│   │   │   ├── vlc_es.h
│   │   │   ├── vlc_es_out.h
│   │   │   ├── vlc_events.h
│   │   │   ├── vlc_filter.h
│   │   │   ├── vlc_fingerprinter.h
│   │   │   ├── vlc_fourcc.h
│   │   │   ├── vlc_fs.h
│   │   │   ├── vlc_gcrypt.h
│   │   │   ├── vlc_httpd.h
│   │   │   ├── vlc_http.h
│   │   │   ├── vlc_image.h
│   │   │   ├── vlc_inhibit.h
│   │   │   ├── vlc_input.h
│   │   │   ├── vlc_input_item.h
│   │   │   ├── vlc_interface.h
│   │   │   ├── vlc_interrupt.h
│   │   │   ├── vlc_keystore.h
│   │   │   ├── vlc_main.h
│   │   │   ├── vlc_md5.h
│   │   │   ├── vlc_media_library.h
│   │   │   ├── vlc_memstream.h
│   │   │   ├── vlc_messages.h
│   │   │   ├── vlc_meta_fetcher.h
│   │   │   ├── vlc_meta.h
│   │   │   ├── vlc_mime.h
│   │   │   ├── vlc_modules.h
│   │   │   ├── vlc_mouse.h
│   │   │   ├── vlc_mtime.h
│   │   │   ├── vlc_network.h
│   │   │   ├── vlc_objects.h
│   │   │   ├── vlc_opengl.h
│   │   │   ├── vlc_picture_fifo.h
│   │   │   ├── vlc_picture.h
│   │   │   ├── vlc_picture_pool.h
│   │   │   ├── vlc_playlist.h
│   │   │   ├── vlc_plugin.h
│   │   │   ├── vlc_probe.h
│   │   │   ├── vlc_rand.h
│   │   │   ├── vlc_renderer_discovery.h
│   │   │   ├── vlc_services_discovery.h
│   │   │   ├── vlc_sout.h
│   │   │   ├── vlc_spu.h
│   │   │   ├── vlc_stream_extractor.h
│   │   │   ├── vlc_stream.h
│   │   │   ├── vlc_strings.h
│   │   │   ├── vlc_subpicture.h
│   │   │   ├── vlc_text_style.h
│   │   │   ├── vlc_threads.h
│   │   │   ├── vlc_timestamp_helper.h
│   │   │   ├── vlc_tls.h
│   │   │   ├── vlc_url.h
│   │   │   ├── vlc_variables.h
│   │   │   ├── vlc_video_splitter.h
│   │   │   ├── vlc_viewpoint.h
│   │   │   ├── vlc_vlm.h
│   │   │   ├── vlc_vout_display.h
│   │   │   ├── vlc_vout.h
│   │   │   ├── vlc_vout_osd.h
│   │   │   ├── vlc_vout_window.h
│   │   │   ├── vlc_xlib.h
│   │   │   └── vlc_xml.h
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
│   ├── libgcc_s_seh-1.dll
│   ├── libstdc++-6.dll
│   ├── libvlccore.dll
│   ├── libvlc.dll
│   ├── libwinpthread-1.dll
│   ├── opengl32.dll
│   └── SDL2.dll
└── plugins
    ├── access
    │   ├── libaccess_concat_plugin.dll
    │   ├── libaccess_imem_plugin.dll
    │   ├── libaccess_mms_plugin.dll
    │   ├── libaccess_realrtsp_plugin.dll
    │   ├── libaccess_srt_plugin.dll
    │   ├── libaccess_wasapi_plugin.dll
    │   ├── libattachment_plugin.dll
    │   ├── libbluray-awt-j2se-1.3.2.jar
    │   ├── libbluray-j2se-1.3.2.jar
    │   ├── libcdda_plugin.dll
    │   ├── libdcp_plugin.dll
    │   ├── libdshow_plugin.dll
    │   ├── libdtv_plugin.dll
    │   ├── libdvdnav_plugin.dll
    │   ├── libdvdread_plugin.dll
    │   ├── libfilesystem_plugin.dll
    │   ├── libftp_plugin.dll
    │   ├── libhttp_plugin.dll
    │   ├── libhttps_plugin.dll
    │   ├── libidummy_plugin.dll
    │   ├── libimem_plugin.dll
    │   ├── liblibbluray_plugin.dll
    │   ├── liblive555_plugin.dll
    │   ├── libnfs_plugin.dll
    │   ├── librist_plugin.dll
    │   ├── librtp_plugin.dll
    │   ├── libsatip_plugin.dll
    │   ├── libscreen_plugin.dll
    │   ├── libsdp_plugin.dll
    │   ├── libsftp_plugin.dll
    │   ├── libshm_plugin.dll
    │   ├── libsmb_plugin.dll
    │   ├── libtcp_plugin.dll
    │   ├── libtimecode_plugin.dll
    │   ├── libudp_plugin.dll
    │   ├── libvcd_plugin.dll
    │   ├── libvdr_plugin.dll
    │   └── libvnc_plugin.dll
    ├── audio_output
    │   ├── libadummy_plugin.dll
    │   ├── libafile_plugin.dll
    │   ├── libamem_plugin.dll
    │   ├── libdirectsound_plugin.dll
    │   ├── libmmdevice_plugin.dll
    │   ├── libwasapi_plugin.dll
    │   └── libwaveout_plugin.dll
    └── codec
        ├── liba52_plugin.dll
        ├── libadpcm_plugin.dll
        ├── libaes3_plugin.dll
        ├── libaom_plugin.dll
        ├── libaraw_plugin.dll
        ├── libaribsub_plugin.dll
        ├── libavcodec_plugin.dll
        ├── libcc_plugin.dll
        ├── libcdg_plugin.dll
        ├── libcrystalhd_plugin.dll
        ├── libcvdsub_plugin.dll
        ├── libd3d11va_plugin.dll
        ├── libdav1d_plugin.dll
        ├── libdca_plugin.dll
        ├── libddummy_plugin.dll
        ├── libdmo_plugin.dll
        ├── libdvbsub_plugin.dll
        ├── libdxva2_plugin.dll
        ├── libedummy_plugin.dll
        ├── libfaad_plugin.dll
        ├── libflac_plugin.dll
        ├── libfluidsynth_plugin.dll
        ├── libg711_plugin.dll
        ├── libjpeg_plugin.dll
        ├── libkate_plugin.dll
        ├── liblibass_plugin.dll
        ├── liblibmpeg2_plugin.dll
        ├── liblpcm_plugin.dll
        ├── libmft_plugin.dll
        ├── libmpg123_plugin.dll
        ├── liboggspots_plugin.dll
        ├── libopus_plugin.dll
        ├── libpng_plugin.dll
        ├── libqsv_plugin.dll
        ├── librawvideo_plugin.dll
        ├── librtpvideo_plugin.dll
        ├── libschroedinger_plugin.dll
        ├── libscte18_plugin.dll
        ├── libscte27_plugin.dll
        ├── libsdl_image_plugin.dll
        ├── libspdif_plugin.dll
        ├── libspeex_plugin.dll
        ├── libspudec_plugin.dll
        ├── libstl_plugin.dll
        ├── libsubsdec_plugin.dll
        ├── libsubstx3g_plugin.dll
        ├── libsubsusf_plugin.dll
        ├── libsvcdsub_plugin.dll
        ├── libt140_plugin.dll
        ├── libtextst_plugin.dll
        ├── libtheora_plugin.dll
        ├── libttml_plugin.dll
        ├── libtwolame_plugin.dll
        ├── libuleaddvaudio_plugin.dll
        ├── libvorbis_plugin.dll
        ├── libvpx_plugin.dll
        ├── libwebvtt_plugin.dll
        ├── libx26410b_plugin.dll
        ├── libx264_plugin.dll
        ├── libx265_plugin.dll
        └── libzvbi_plugin.dll
```

Everything in windepend/include is regular header stuff.

For lib/plugin, you can find everything in what VLC ships.

Create a directory called winbuild. This is where you will find the executable.

After you have everything here, then you can build with `make windows`.

You will need to ship the executable with everything in windepend/lib but opengl32.dll.
