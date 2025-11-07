export PKG_CONFIG_LIBDIR=arm64/lib/pkgconfig/
./configure \
--prefix=arm64 \
--enable-openssl \
--disable-debug \
--disable-programs \
--disable-muxers \
--disable-encoders \
--disable-postproc \
--disable-doc --disable-filter=nlmeans  --disable-filter=yadif_videotoolbox \
--enable-cross-compile \
--target-os=darwin --disable-outdev=audiotoolbox \
--cc="xcrun -sdk iphoneos clang" \
--ranlib='/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ranlib' \
--extra-cxxflags='-arch arm64 -mios-version-min=9.0 -fno-objc-msgsend-selector-stubs -I/usr/ios/include -Iarm64/include' \
--extra-cflags="-arch arm64 -miphoneos-version-min=9.0 -fno-objc-msgsend-selector-stubs -I/usr/ios/include -Iarm64/include" \
--extra-ldflags="-arch arm64 -miphoneos-version-min=9.0 -fno-objc-msgsend-selector-stubs -L/usr/ios/lib -Larm64/lib -lbz2 -lz" \
--arch=arm64 \
--enable-pic --enable-libass  \
--enable-gpl \
--enable-libmp3lame \
--enable-libspeex  \
--enable-libfreetype \
--enable-libx264 \
--enable-nonfree \
--enable-libopencore-amrnb \
--enable-libopencore-amrwb \
--enable-libvo-amrwbenc \
--disable-libtheora \
--enable-version3 \
--disable-libvorbis \

#--disable-demuxer=encx \
#--disable-demuxer=encx \
#--enable-libvorbis \
#--enable-libvpx 
#--disable-decoder=libvpx 
#--enable-librtmp \
# \
#--disable-demuxer=encx \
#--disable-demuxer=encx_audio \
#--disable-demuxer=encx \
