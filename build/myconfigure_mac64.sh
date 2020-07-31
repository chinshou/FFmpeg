#!/bin/bash
export PKG_CONFIG_LIBDIR=/usr/local/darwin64/lib/pkgconfig/
export PKG_CONFIG_PATH=/usr/local/darwin64/lib/pkgconfig/
LDFLAGS=" -L/usr/lib -L/usr/local/darwin64/lib " CPPFLAGS=" -I/usr/local/darwin64/include " CFLAGS=" -I/usr/local/darwin64/include -I/Users/hubdog/Documents/Projects/FFMPEG/BlackmagicDeckLinkSDK11.0/Mac/include" ../configure --disable-outdev=sdl --arch=x64  \
--target-os=darwin --prefix=/usr/local/darwin64 --enable-gpl --disable-postproc --enable-shared  \
--cc="cc " \
--enable-cross-compile --enable-libx264  --disable-decoder=libvpx  \
--enable-libmp3lame --enable-nonfree --enable-libvo-amrwbenc --disable-lzma \
--enable-libfreetype --enable-version3  --enable-libtheora  \
--enable-libvorbis --enable-runtime-cpudetect --enable-libsox  --enable-libopencore-amrnb  \
--enable-libopencore-amrwb --enable-openal --enable-openssl --enable-libsrt --enable-decklink  \
--enable-libwebp   \
--disable-muxer=encx   \
--disable-demuxer=encx   \
--disable-demuxer=encx_audio   \
--disable-muxer=encx_audio   \
--disable-filter=coreimage \
--disable-outdev=sdl2 \
--disable-filter=coreimagesrc \
--extra-ldflags=" -lcurl -liconv -lbz2 -lz -framework OpenAL" \
--extra-cflags=" -mmacosx-version-min=10.9 " \
--extra-cxxflags=" -std=c++11 -stdlib=libc++" \
 --enable-libass 
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
#--enable-libgsm 
#  --enable-libilbc
#--enable-libspeex 
#--enable-libvpx
#--enable-libx265   \
#--enable-libxavs 
#--enable-librtmp --disable-demuxer=encx   \
#--disable-encoders   \