NAME = fftools
DESC = FFmpeg converter library
#FFLIBS = avutil 

HEADERS = ffmpeg.h                       \
          version.h                      \
          version_major.h                \

OBJS = ffmpeg.o                        \
    ffmpeg_demux.o      \
    ffmpeg_filter.o     \
    ffmpeg_hw.o         \
    ffmpeg_mux.o        \
    ffmpeg_mux_init.o   \
    ffmpeg_opt.o        \
    objpool.o           \
    sync_queue.o        \
    cmdutils.o          \
    thread_queue.o      \
    opt_common.o        \
    
#OBJS-$(CONFIG_LIBSOXR) += soxr_resample.o

# Objects duplicated from other libraries for shared builds
#SHLIBOBJS              += log2_tab.o

# Windows resource file
#SHLIBOBJS-$(HAVE_GNU_WINDRES) += swresampleres.o

#TESTPROGS = swresample
