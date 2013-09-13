export PKG_CONFIG_PATH=/usr/i686-mingw32/lib/pkgconfig/
../configure --enable-memalign-hack --arch=x86 --target-os=mingw32 --cross-prefix=i686-w64-mingw32- \
--cc='i686-w64-mingw32-gcc' --disable-postproc --enable-shared --disable-static --disable-muxers \
--disable-decoder=libvpx --disable-encoder=aac --enable-avisynth  --enable-gpl --disable-encoders \
--enable-version3 --enable-pthreads --enable-avfilter --enable-runtime-cpudetect --enable-nonfree \
--pkg-config=pkg-config --enable-libquvi \
--enable-libnut --enable-libgsm --enable-libfreetype --enable-libvorbis --enable-libspeex \
--enable-zlib --enable-libtheora --enable-bzlib \
--enable-libopenjpeg --enable-libschroedinger --enable-librtmp  --enable-libass \
--enable-libbluray --enable-openssl --enable-libflite  --enable-libsox \
--disable-ffplay --enable-libcdio  --enable-libcelt \
--enable-libxavs  --disable-outdev=sdl \
--extra-cflags="-I/usr/i686-mingw32/include" --extra-ldflags="-L/usr/i686-mingw32/lib"    \
--extra-libs="-lx264 -lpthread -lwinmm -llua -liconv -lcurl -lws2_32 -lssl -lcrypto  -lwldap32 -lgdi32 -lwsock32" 
#--enable-libutvideo --enable-frei0r --enable-libmodplug --enable-libilbc
