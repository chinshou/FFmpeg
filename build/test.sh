export PKG_CONFIG_LIBDIR=/usr/darwin32/lib/pkgconfig/ 
pkg-config --exists liasd
echo $?
pkg-config --exists freetype2
echo $?
pkg-config --exists librtmp
echo $?

