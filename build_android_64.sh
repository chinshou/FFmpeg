#!/bin/bash
#NDK路径
export ANDROID_NDK_ROOT=$HOME/Projects/android-ndk-r19c
export AOSP_TOOLCHAIN_SUFFIX=4.9
export AOSP_API="android-23"
#架构
ARCH=arm64-v8a
#根据架构配置环境变量
TOOLCHAIN_BASE="aarch64-linux-android"
TOOLNAME_BASE="aarch64-linux-android"
AOSP_ABI="arm64-v8a"
AOSP_ARCH="arch-arm64"
HOST="aarch64-linux"

PREFIX=$HOME/Projects/android-ndk-r17c/sources/android/$AOSP_ARCH
TOOLCHAIN=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64
SYSROOT=$ANDROID_NDK_ROOT/platforms/$AOSP_API/$AOSP_ARCH

export ANDROID_SDK=${HOME}/android-sdk
CROSS_COMPILE=$TOOLCHAIN/bin/aarch64-linux-android-
export PATH=$PATH:$ANDROID_SDK/tools:$ANDROID_SDK/platform-tools
export ARM_LIB=$PREFIX/lib

CFLAGS="-O3 -fPIC -march=armv8-a -D__ANDROID_API__=23 -D__ANDROID__ -D__ARM_ARCH_8__ -D__ARM_ARCH_8A__ -I$ANDROID_NDK_ROOT/sysroot/usr/include -I$ANDROID_NDK_ROOT/sysroot/usr/include/aarch64-linux-android -I$PREFIX/include "

CXXFLAGS="-O3 -fPIC -march=armv8-a -D__ANDROID_API__=23 -D__ANDROID__ -D__ARM_ARCH_8__ -D__ARM_ARCH_8A__ -I$ANDROID_NDK_ROOT/sysroot/usr/include -I$ANDROID_NDK_ROOT/sysroot/usr/include/aarch64-linux-android -I$PREFIX/include"

#export LDFLAGS=" -lz "

function build_one
{

export PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig/
echo $PKG_CONFIG_PATH
./configure \
    --prefix=$PREFIX \
    --enable-shared \
    --disable-static \
    --disable-doc \
    --disable-ffmpeg \
    --disable-ffplay \
    --disable-ffprobe \
    --enable-avdevice \
    --enable-nonfree \
    --enable-jni \
    --enable-version3 \
    --enable-mediacodec \
    --enable-libfreetype --enable-libopencore-amrnb \
    --enable-libopus --enable-libmp3lame \
    --enable-libx264 --enable-libspeex  \
    --disable-decoder=libvpx --enable-openssl \
    --enable-gpl \
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
    --disable-indev=waveform  \
    --cross-prefix=$CROSS_COMPILE \
    --cc=$TOOLCHAIN/bin/aarch64-linux-android23-clang \
    --target-os=android \
    --pkg-config=pkg-config \
    --arch=aarch64 \
    --enable-cross-compile \
    --sysroot=${SYSROOT} \
    --extra-cflags="-O3 -fpic $CFLAGS" \
    --extra-ldflags=" -L$ARM_LIB -lz -lm -logg "
 
#--enable-libvpx \
#make clean
#make
#make install
#    --disable-muxers \
#     --disable-demuxer=encx \
}

#ADDI_CFLAGS="-marm -march=armv7-a -mfpu=neon -mfloat-abi=softfp -mtune=cortex-a8"
#ADDI_CFLAGS="-marm -march=armv7-a -mfpu=vfp -mfloat-abi=softfp "
#ADDI_CFLAGS="-marm -march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=softfp "
#    
#    --enable-libsox \
#    
#    --enable-libvidstab \
#     \
#    
#    
#    
#        --enable-libgsm      \           \
#       \      \     \
#    --enable-libzvbi --enable-libvorbis \
#       --enable-librtmp \

build_one
