export PKG_CONFIG_PATH=/usr/i686-mingw32/lib/pkgconfig/
../configure --enable-memalign-hack --arch=x86 --target-os=mingw32 --cross-prefix=i686-w64-mingw32-  \
 --cc='i686-w64-mingw32-gcc' --disable-postproc --enable-shared --disable-static  \
 --disable-decoder=libvpx \
 --disable-encoders \
 --enable-gpl --enable-version3 --enable-pthreads --enable-avfilter \
 --enable-runtime-cpudetect --enable-nonfree --pkg-config=pkg-config \
 --enable-libfaac  --enable-libnut --enable-libfreetype \
 --enable-zlib --enable-bzlib  \
 --enable-libx264 \
 --enable-encoder=libx264 \
 --enable-encoder=libfaac \
 --disable-ffplay \
 --disable-muxer=encx \
 --disable-muxer=encx_audio \
 --disable-demuxer=encx_audio \
 --disable-demuxer=encx \
 --enable-libmfx \
 --pkg-config-flags="--static" \
 --disable-outdev=sdl --extra-cflags="-I/usr/i686-mingw32/include"  \
 --extra-ldflags="-L/usr/i686-mingw32/lib -static -static-libgcc -static-libstdc++ "  \
 --extra-libs="-lx264 -lpthread -lwinmm -llua -liconv -lcurl -lws2_32 -lssl -lcrypto  -lwldap32 -lgdi32 -lwsock32 -lexpat" 
#--enable-libutvideo --enable-frei0r --enable-libmodplug --enable-libilbc --enable-libflite --enable-libvo-aacenc
