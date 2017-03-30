export PKG_CONFIG_PATH=/usr/x86_64-mingw32/lib/pkgconfig/
../configure --enable-memalign-hack --arch=x86 --target-os=mingw32 --cross-prefix=x86_64-w64-mingw32-  \
 --cc='x86_64-w64-mingw32-gcc' --disable-postproc --enable-shared --disable-static  \
 --disable-decoder=libvpx \
 --enable-avisynth  --enable-gpl --enable-version3 --enable-pthreads --enable-avfilter \
 --enable-runtime-cpudetect --enable-nonfree --pkg-config=pkg-config \
 --enable-libnut --enable-libgsm --enable-libfreetype --enable-libvorbis --enable-libspeex \
 --enable-libmp3lame --enable-zlib --enable-libtheora --enable-bzlib \
 --enable-libopencore-amrnb --enable-libopencore-amrwb \
 --enable-libx264 --enable-libbluray --enable-libvo-amrwbenc \
 --enable-openssl  --disable-ffplay --enable-libcelt \
 --enable-libwebp \
 --disable-muxer=encx \
 --disable-muxer=encx_audio \
 --disable-demuxer=encx_audio \
 --disable-demuxer=encx \
 --enable-libx265 \
 --enable-libxavs --disable-outdev=sdl --extra-cflags="-I/usr/x86_64-mingw32/include"  \
 --extra-ldflags="-L/usr/x86_64-mingw32/lib -static -static-libgcc -static-libstdc++ "  \
 --extra-libs="-lpthread -lwinmm -liconv -lws2_32 -lssl -lcrypto -lwldap32 -lgdi32 -lwsock32 -lexpat" 
# --enable-libxvid  --enable-libutvideo --enable-frei0r --enable-libmodplug --enable-libilbc --enable-libflite --enable-libvo-aacenc  --enable-libsox \
