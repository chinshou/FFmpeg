export PKG_CONFIG_LIBDIR=/usr/local/darwin32/lib/pkgconfig/
export PKG_CONFIG_PATH=/usr/local/darwin32/lib/pkgconfig/
LDFLAGS=" -L/usr/lib -L/usr/local/darwin32/lib " CPPFLAGS=" -I/usr/local/darwin32/include" CFLAGS=" -I/usr/local/darwin32/include" ../configure --enable-memalign-hack --disable-outdev=sdl --arch=x86  \
--target-os=darwin --prefix=/usr/local/darwin32 --enable-gpl --disable-postproc --enable-shared  \
--cc="cc -m32" \
--enable-cross-compile --disable-decoder=libvpx  \
--enable-nonfree   \
--enable-libfreetype --enable-version3  \
--enable-runtime-cpudetect  \
--enable-openal \
--disable-muxers  \
--disable-encoders   \
--disable-demuxer=encx   \
--extra-ldflags=" -liconv" \
--extra-cflags=" -mmacosx-version-min=10.6 " --disable-indev=avfoundation \
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