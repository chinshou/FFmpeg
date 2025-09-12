NAME = fftools
DESC = FFmpeg converter library
#FFLIBS = avutil 

HEADERS = ffmpeg.h                       \
          version.h                      \
          version_major.h                \

OBJS = ffmpeg.o                        \
    ffmpeg_dec.o        \
    ffmpeg_demux.o      \
    ffmpeg_enc.o        \
    ffmpeg_filter.o     \
    ffmpeg_hw.o         \
    ffmpeg_mux.o        \
    ffmpeg_mux_init.o   \
    ffmpeg_opt.o        \
    ffmpeg_sched.o      \
    graph/graphprint.o        \
    sync_queue.o        \
    cmdutils.o          \
    thread_queue.o      \
    opt_common.o        \
    textformat/avtextformat.o \
    textformat/tf_compact.o   \
    textformat/tf_default.o   \
    textformat/tf_flat.o      \
    textformat/tf_ini.o       \
    textformat/tf_json.o      \
    textformat/tf_mermaid.o   \
    textformat/tf_xml.o       \
    textformat/tw_avio.o      \
    textformat/tw_buffer.o    \
    textformat/tw_stdout.o    \
#    ffplay_renderer.o   \
    
#OBJS-$(CONFIG_LIBSOXR) += soxr_resample.o

# Objects duplicated from other libraries for shared builds
#SHLIBOBJS              += log2_tab.o

# Windows resource file
#SHLIBOBJS-$(HAVE_GNU_WINDRES) += swresampleres.o

#TESTPROGS = swresample
