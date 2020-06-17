#!/bin/bash

NDK=$HOME/Projects/android-ndk-r9
SYSROOT=$NDK/platforms/android-9/arch-arm/
TOOLCHAIN=$NDK/toolchains/arm-linux-androideabi-4.8/prebuilt/linux-x86_64
# Expand the prebuilt/* path into the correct one
#TOOLCHAIN=`echo $NDK/toolchains/arm-linux-androideabi-4.4.3/prebuilt/*-x86`
export PATH=$TOOLCHAIN/bin:$PATH
CPU=arm
PREFIX=../android/$CPU 
ANDROID_SOURCE=./android-source
ANDROID_LIBS=./android-libs
ABI="armeabi-v7a"
EXTRA_CFLAGS=" -I$PREFIX/include -I/home/hubdog/Projects/android-ndk-r9/sources/android/arm/include"
#EXTRA_CFLAGS="-I$ANDROID_SOURCE/frameworks/base/include -I$ANDROID_SOURCE/system/core/include"
#EXTRA_CFLAGS="$EXTRA_CFLAGS -I$ANDROID_SOURCE/hardware/include"
#EXTRA_CFLAGS="$EXTRA_CFLAGS -I$ANDROID_SOURCE/frameworks/base/media/libstagefright"
#EXTRA_CFLAGS="$EXTRA_CFLAGS -I$ANDROID_SOURCE/frameworks/base/include/media/stagefright/openmax"
#EXTRA_CFLAGS="$EXTRA_CFLAGS -I$NDK/sources/cxx-stl/gnu-libstdc++/4.8/include -I$NDK/sources/cxx-stl/gnu-libstdc++/4.8/libs/$ABI/include"

EXTRA_CFLAGS="$EXTRA_CFLAGS -march=armv7-a -mfloat-abi=softfp -mfpu=neon"
EXTRA_LDFLAGS=" -fuse-ld=bfd -L$PREFIX/lib -L../$PREFIX/lib -lz -lm -logg"
#EXTRA_LDFLAGS="-Wl,--fix-cortex-a8 -L$ANDROID_LIBS -Wl,-rpath-link,$ANDROID_LIBS -L$NDK/sources/cxx-stl/gnu-libstdc++/4.8/libs/$ABI"
#EXTRA_CXXFLAGS="-Wno-multichar -fno-exceptions -fno-rtti"
#    --disable-decoder=h264  \

function build_one
{
export PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig/
./configure \
    --prefix=$PREFIX --host-os=android \
    --enable-shared \
    --disable-static \
    --disable-doc \
    --disable-ffmpeg \
    --disable-ffplay \
    --disable-ffprobe \
    --enable-avdevice \
    --enable-nonfree \
    --enable-openssl \
    --enable-libfreetype \
    --enable-libspeex \
    --enable-libgsm \
    --enable-jni \
    --enable-libopencore-amrnb --enable-version3 \
    --enable-libopus \
    --enable-mediacodec \
    --disable-decoder=libvpx \
    --enable-gpl \
    --enable-libx264 \
    --disable-doc \
    --disable-symver \
    --disable-indev=dv1394 \
    --disable-indev=v4l2 \
    --disable-indev=fbdev \
    --disable-protocol=libssh \
    --disable-outdevs \
    --disable-postproc \
    --disable-decoder=h264_vdpau \
    --disable-muxer=encx \
    --disable-iconv --disable-muxer=encx_audio \
    --enable-libmp3lame \
    --disable-indev=waveform  \
    --cross-prefix=$TOOLCHAIN/bin/arm-linux-androideabi- \
    --target-os=android \
    --pkg-config=pkg-config \
    --arch=arm \
    --cpu=armv7-a \
    --enable-cross-compile \
    --sysroot=$SYSROOT \
    --extra-cflags="-Os -fpic $EXTRA_CFLAGS" \
    --extra-ldflags="$EXTRA_LDFLAGS" \
    --extra-cxxflags="$EXTRA_CXXFLAGS" \
    $ADDITIONAL_CONFIGURE_FLAG

#--enable-libvpx \
#make clean
#make
#make install
#    --disable-muxers \
#     --disable-demuxer=encx \
}

ADDI_CFLAGS="-marm -march=armv7-a -mfpu=neon -mfloat-abi=softfp -mtune=cortex-a8"
#ADDI_CFLAGS="-marm -march=armv7-a -mfpu=vfp -mfloat-abi=softfp "
#ADDI_CFLAGS="-marm -march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=softfp "
#    
#    --enable-libsox \
#    
#    --enable-libvidstab \
#    --enable-libvorbis \
#    
#    
#    
#    
#    --enable-libzvbi \
#       --enable-librtmp \

build_one
