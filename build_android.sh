#!/bin/bash

NDK=$HOME/Projects/android-ndk-r9
SYSROOT=$NDK/platforms/android-9/arch-arm/
TOOLCHAIN=$NDK/toolchains/arm-linux-androideabi-4.8/prebuilt/linux-x86_64
# Expand the prebuilt/* path into the correct one
#TOOLCHAIN=`echo $NDK/toolchains/arm-linux-androideabi-4.4.3/prebuilt/*-x86`
export PATH=$TOOLCHAIN/bin:$PATH
ANDROID_SOURCE=./android-source
ANDROID_LIBS=./android-libs
ABI="armeabi-v7a"
EXTRA_CFLAGS=""
EXTRA_CFLAGS="-I$ANDROID_SOURCE/frameworks/base/include -I$ANDROID_SOURCE/system/core/include"
EXTRA_CFLAGS="$EXTRA_CFLAGS -I$ANDROID_SOURCE/hardware/include"
EXTRA_CFLAGS="$EXTRA_CFLAGS -I$ANDROID_SOURCE/frameworks/base/media/libstagefright"
EXTRA_CFLAGS="$EXTRA_CFLAGS -I$ANDROID_SOURCE/frameworks/base/include/media/stagefright/openmax"
EXTRA_CFLAGS="$EXTRA_CFLAGS -I$NDK/sources/cxx-stl/gnu-libstdc++/4.8/include -I$NDK/sources/cxx-stl/gnu-libstdc++/4.8/libs/$ABI/include"

EXTRA_CFLAGS="$EXTRA_CFLAGS -march=armv7-a -mfloat-abi=softfp -mfpu=neon"
EXTRA_LDFLAGS=""
EXTRA_LDFLAGS="-Wl,--fix-cortex-a8 -L$ANDROID_LIBS -Wl,-rpath-link,$ANDROID_LIBS -L$NDK/sources/cxx-stl/gnu-libstdc++/4.8/libs/$ABI"
EXTRA_CXXFLAGS="-Wno-multichar -fno-exceptions -fno-rtti"

function build_one
{
./configure \
    --prefix=$PREFIX \
    --enable-shared \
    --disable-static \
    --disable-doc \
    --disable-ffmpeg \
    --disable-ffplay \
    --disable-ffprobe \
    --disable-ffserver \
    --enable-avdevice \
    --disable-encoders \
    --disable-muxers \
    --disable-doc \
    --disable-symver \
    --disable-decoder=h264  \
    --disable-decoder=h264_vdpau \
    --enable-libstagefright-h264  \
    --cross-prefix=$TOOLCHAIN/bin/arm-linux-androideabi- \
    --target-os=linux \
    --arch=arm \
    --cpu=armv7-a \
    --enable-cross-compile \
    --sysroot=$SYSROOT \
    --extra-cflags="-Os -fpic $EXTRA_CFLAGS" \
    --extra-ldflags="$EXTRA_LDFLAGS" \
    --extra-cxxflags="$EXTRA_CXXFLAGS" \
    $ADDITIONAL_CONFIGURE_FLAG

#make clean
#make
#make install
}

CPU=arm
PREFIX=$(pwd)/android/$CPU 
#ADDI_CFLAGS="-marm -march=armv7-a -mfpu=neon -mfloat-abi=softfp -mtune=cortex-a8"
#ADDI_CFLAGS="-marm -march=armv7-a -mfpu=vfp -mfloat-abi=softfp "
#ADDI_CFLAGS="-marm -march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=softfp "

build_one
