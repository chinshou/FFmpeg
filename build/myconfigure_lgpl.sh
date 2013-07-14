export PKG_CONFIG_LIBDIR=/usr/darwin32/lib/pkgconfig/
export PKG_CONFIG_PATH=/usr/darwin32/lib/pkgconfig/
LDFLAGS="-arch i386 -L/usr/lib -L/usr/darwin32/lib" CPPFLAGS="-arch i386 -I/usr/darwin32/include" CFLAGS="-arch i386 -I/usr/darwin32/include" ../configure --enable-memalign-hack --disable-outdev=sdl --arch=x86 --target-os=darwin --prefix=/usr/darwin32 --disable-postproc --enable-shared --enable-cross-compile --enable-libvpx --disable-decoder=libvpx --enable-libgsm --enable-libmp3lame --disable-encoder=aac --enable-libfreetype --enable-libtheora --enable-libspeex --enable-libvorbis --enable-runtime-cpudetect --enable-libsox --enable-openal --enable-openssl --enable-librtmp --enable-libbluray --enable-libflite  --enable-libcelt --enable-libopenjpeg  --enable-libilbc --enable-libass
echo Please check libsdl existence problem , uninstall all libsdl
read
#--cc="clang -m32" --extra-cflags="-fomit-frame-pointer" --extra-ldflags="-fomit-frame-pointer" --extra-cxxflags="-fomit-frame-pointer"
#--extra-cflags=-I/System/Library/Frameworks/OpenAL.framework/Headers --extra-ldflags=-L/System/Library/Frameworks/OpenAL.framework
#--enable-libutvideo
#--enable-libmodplug
#--enable-libcdio
#--enable-libschroedinger
