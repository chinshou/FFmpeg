export PKG_CONFIG_PATH=/usr/i686-mingw32/lib/pkgconfig/
../configure --arch=x86 --target-os=mingw32 --cross-prefix=i686-w64-mingw32-  \
 --cc='i686-w64-mingw32-gcc' --disable-postproc --enable-shared --disable-static  \
 --enable-avisynth  --enable-gpl --enable-version3  --enable-avfilter \
 --enable-runtime-cpudetect --enable-nonfree --pkg-config=pkg-config \
 --enable-libgsm --enable-libfreetype --enable-libvorbis --enable-libspeex  \
 --enable-libmp3lame --enable-zlib --enable-libtheora --enable-bzlib --enable-libvpx  \
 --enable-libopencore-amrnb --enable-libopencore-amrwb \
 --enable-librtmp  --enable-libass --enable-libx264 --enable-libbluray \
 --enable-openssl --enable-libopus --enable-libsox --enable-libx265 --enable-libcdio \
 --enable-libvo-amrwbenc  --enable-libwebp  --enable-nvenc --enable-cuda --enable-cuvid \
 --disable-muxer=encx \
 --disable-muxer=encx_audio \
 --enable-libxml2 --enable-libmfx \
 --disable-outdevs \
 --disable-decoder=libvpx --disable-decoder=libopus \
 --disable-decoder=libvpx-vp9 \
 --pkg-config-flags="--static" \
 --enable-libxavs  --disable-outdev=sdl --extra-cflags="-I/usr/i686-mingw32/include"  \
 --extra-ldflags="-L/usr/i686-mingw32/lib -static -static-libgcc -static-libstdc++ "  \
 --extra-libs="-lsupc++ -lstdc++ -lx264  -lwinmm -llua -liconv -lcurl -lws2_32 -lssl -lcrypto  -lwldap32 -lgdi32 -lwsock32 -lexpat" 
#--enable-libutvideo --enable-frei0r --enable-libmodplug --enable-libilbc --enable-libflite --enable-libvo-aacenc
#--enable-pthreads -lpthread --enable-libopenjpeg --enable-libxvid  
#   \
