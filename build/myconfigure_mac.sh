export PKG_CONFIG_LIBDIR=/usr/darwin32/lib/pkgconfig/
export PKG_CONFIG_PATH=/usr/darwin32/lib/pkgconfig/
LDFLAGS="-arch i386 -L/usr/lib -L/usr/darwin32/lib " CPPFLAGS="-arch i386 -I/usr/darwin32/include" CFLAGS="-arch i386 -I/usr/darwin32/include" ../configure --enable-memalign-hack --disable-outdev=sdl --arch=x86  \
--target-os=darwin --prefix=/usr/darwin32 --enable-gpl --disable-postproc --enable-shared  \
--enable-cross-compile --enable-libx264 --enable-libvpx --disable-decoder=libvpx --enable-libgsm  \
--enable-libmp3lame --enable-nonfree --disable-encoder=aac --enable-libfaac --enable-libvo-amrwbenc  \
--enable-libfreetype --enable-version3  --enable-libtheora --enable-libspeex  \
--enable-libvorbis --enable-runtime-cpudetect --enable-libsox  --enable-libopencore-amrnb  \
--enable-libopencore-amrwb --enable-libxavs --enable-openal --enable-openssl --enable-librtmp  \
--enable-libx265   \
--extra-ldflags=" -llua -lcurl -liconv" \
--extra-cflags=" -mmacosx-version-min=10.6 " \
--enable-libbluray --enable-libcelt --enable-libxvid --enable-libopenjpeg  --enable-libilbc --enable-libass --enable-libquvi
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
