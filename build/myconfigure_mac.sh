#!/bin/bash
export PKG_CONFIG_LIBDIR=/usr/local/darwin32/lib/pkgconfig/
export PKG_CONFIG_PATH=/usr/local/darwin32/lib/pkgconfig/
LDFLAGS=" -L/usr/lib -L/usr/local/darwin32/lib " CPPFLAGS=" -I/usr/local/darwin32/include" CFLAGS=" -I/usr/local/darwin32/include" ../configure --disable-outdev=sdl --arch=x86  \
--target-os=darwin --prefix=/usr/local/darwin32 --enable-gpl --disable-postproc --enable-shared  \
--cc="cc -m32" \
--enable-cross-compile --enable-libx264 --enable-libvpx --disable-decoder=libvpx --enable-libgsm  \
--enable-libmp3lame --enable-nonfree --enable-libvo-amrwbenc  \
--enable-libfreetype --enable-version3  --enable-libtheora --enable-libspeex  \
--enable-libvorbis --enable-runtime-cpudetect --enable-libsox  --enable-libopencore-amrnb  \
--enable-libopencore-amrwb --enable-libxavs --enable-openal --enable-openssl --enable-librtmp  \
--enable-libx265   \
--enable-libwebp   \
--disable-muxer=encx   \
--disable-muxer=encx_audio   \
--disable-filter=coreimage \
--disable-filter=coreimagesrc \
--extra-ldflags=" -lcurl -liconv -lbz2 -lz -framework OpenAL" \
--extra-cflags=" -mmacosx-version-min=10.8 " \
  --enable-libilbc --enable-libass 
echo Please check libsdl existence problem , uninstall all libsdl
read
#--cc="clang -m32" --extra-cflags="-fomit-frame-pointer" --extra-ldflags="-fomit-frame-pointer" --extra-cxxflags="-fomit-frame-pointer"
#--extra-cflags=-I/System/Library/Frameworks/OpenAL.framework/Headers --extra-ldflags=-L/System/Library/Frameworks/OpenAL.framework
#--enable-libutvideo
#--enable-libmodplug
#--enable-libcdio
#--enable-libschroedinger
#--enable-libvo-aacenc
#--enable-libflite 
#--enable-libbluray
#--enable-libcelt
#--enable-libxvid