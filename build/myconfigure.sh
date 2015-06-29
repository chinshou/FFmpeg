export PKG_CONFIG_PATH=/usr/i686-mingw32/lib/pkgconfig/
../configure --enable-memalign-hack --arch=x86 --target-os=mingw32 --cross-prefix=i686-w64-mingw32-  \
 --cc='i686-w64-mingw32-gcc' --disable-postproc --enable-shared --disable-static  \
 --disable-decoder=libvpx --disable-encoder=aac  \
 --enable-avisynth  --enable-gpl --enable-version3 --enable-pthreads --enable-avfilter \
 --enable-runtime-cpudetect --enable-nonfree --pkg-config=pkg-config \
 --enable-libquvi --enable-libfaac  --enable-libnut --enable-libgsm --enable-libfreetype --enable-libvorbis --enable-libspeex  \
 --enable-libmp3lame --enable-zlib --enable-libtheora --enable-bzlib --enable-libvpx  \
 --enable-libxvid --enable-libopencore-amrnb --enable-libopencore-amrwb \
 --enable-libopenjpeg --enable-libschroedinger --enable-librtmp  --enable-libass --enable-libx264 --enable-libbluray \
 --enable-openssl --enable-libsox --disable-ffplay --enable-libx265 --enable-libcdio \
 --enable-libcelt --enable-libvo-amrwbenc  --enable-libwebp \
 --disable-muxer=encx \
 --disable-muxer=encx_audio \
 --disable-demuxer=encx_audio \
 --disable-demuxer=encx \
 --enable-libxavs  --disable-outdev=sdl --extra-cflags="-I/usr/i686-mingw32/include"  \
 --extra-ldflags="-L/usr/i686-mingw32/lib -static -static-libgcc -static-libstdc++ "  \
 --extra-libs="-lx264 -lpthread -lwinmm -llua -liconv -lcurl -lws2_32 -lssl -lcrypto  -lwldap32 -lgdi32 -lwsock32 -lexpat" 
#--enable-libutvideo --enable-frei0r --enable-libmodplug --enable-libilbc --enable-libflite --enable-libvo-aacenc
