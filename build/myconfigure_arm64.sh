#!/bin/bash
export PKG_CONFIG_LIBDIR=/usr/local/arm64/lib/pkgconfig/
export PKG_CONFIG_PATH=/usr/local/arm64/lib/pkgconfig/
LDFLAGS=" -L/usr/lib -L/usr/local/arm64/lib " CPPFLAGS=" -I/usr/local/arm64/include " CFLAGS=" -I/usr/local/arm64/include -I/Users/hubdog/Documents/Projects/FFMPEG/BlackmagicDeckLinkSDK11.0/Mac/include" ../configure --disable-outdev=sdl --arch=arm64  \
--target-os=darwin --prefix=/usr/local/arm64 --enable-gpl --disable-postproc --enable-shared  \
--cc="cc -arch arm64 " \
--enable-cross-compile  \
--enable-gpl --enable-version3  --enable-avfilter \
--enable-libgsm --enable-libfreetype  --enable-libvorbis --enable-libspeex --disable-lzma \
--enable-libmp3lame  --enable-libtheora  \
--enable-libopencore-amrnb --enable-libopencore-amrwb \
--enable-libass --enable-libx264  --enable-libvo-amrwbenc \
--enable-openssl  --disable-ffplay  \
--enable-libwebp --enable-libxml2 \
--enable-libx265 \
--disable-demuxer=encx   \
--disable-demuxer=encx_audio   \
--disable-muxer=encx   \
--disable-muxer=encx_audio  --enable-runtime-cpudetect \
--disable-filter=coreimage \
--disable-outdev=sdl2 \
--disable-filter=coreimagesrc --disable-filter=yadif_videotoolbox \
--extra-ldflags=" -lcurl -liconv -lbz2 -lz -framework OpenAL -framework CoreFoundation -framework CoreServices -framework ApplicationServices" \
--extra-cflags=" -mmacosx-version-min=10.9 " \
--extra-cxxflags=" -std=c++11 -stdlib=libc++" \
--extra-libs="-lstdc++  -lsharpyuv "  
#--enable-libass 
echo Please check libsdl existence problem , uninstall all libsdl
read
#--cc="clang -m32" --extra-cflags="-fomit-frame-pointer" --extra-ldflags="-fomit-frame-pointer" --extra-cxxflags="-fomit-frame-pointer"
#--extra-cflags=-I/System/Library/Frameworks/OpenAL.framework/Headers --extra-ldflags=-L/System/Library/Frameworks/OpenAL.framework
#--enable-libutvideo
#--enable-libmodplug --enable-openal
#--enable-libcdio --enable-libbluray
#--enable-libschroedinger
#--enable-libvo-aacenc
#--enable-libflite 
#--enable-libbluray
#--enable-libcelt
#--enable-libxvid
#--enable-libgsm 
#  --enable-libilbc
#--disable-demuxer=encx   \
#--disable-demuxer=encx_audio   \
#--enable-libspeex 
#--enable-libvpx
#--enable-libx265   \
#--enable-libxavs 
#--enable-librtmp --disable-demuxer=encx   \
#--disable-encoders   \