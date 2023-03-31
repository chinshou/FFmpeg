#!/bin/sh

#https://github.com/jiaoshenmene/FFmpeg-android-build-script/blob/master/build-ffmpeg.sh
#设置你自己的NDK位置
NDK_HOME=/home/hubdog/Projects/android-ndk-r17c
ISYSROOT=$NDK_HOME/sysroot
#设置你自己的平台，这上Mac上的，linux换成linux-x86_64
NDK_HOST_PLATFORM=linux-x86_64
ASM=$ISYSROOT/usr/include

ARCHS="armv7-a "

COMPILE="y"

PREFIX=android-build
EXTRA_LDFLAGS="-pie -fuse-ld=bfd -L/home/hubdog/Projects/android-ndk-r9/sources/android/arm/lib -lz -lm -logg"

COMMON_OPTIONS="
    --target-os=android \
    --disable-static \
    --enable-shared \
    --enable-small \
    --disable-programs \
    --disable-ffmpeg \
    --disable-ffplay \
    --disable-ffprobe \
    --disable-postproc \
    --disable-doc \
    --disable-symver \
    --disable-asm \
    --enable-nonfree \
    --enable-openssl \
    --enable-libfreetype \
    --enable-libspeex \
    --enable-libopencore-amrnb --enable-version3 \
    --enable-libopus \
    --enable-mediacodec \
    --disable-decoder=libvpx \
    --enable-gpl --disable-iconv \
    --enable-libx264 \
    --enable-libmp3lame \
    --enable-jni \
    --pkg-config=pkg-config \
    "

function build_armeabi {
    export PKG_CONFIG_PATH=/home/hubdog/Projects/android-ndk-r9/sources/android/arm/lib/pkgconfig/
    ./configure \
    --libdir=/home/hubdog/Projects/android-ndk-r9/sources/android/arm/lib \
    --incdir=/home/hubdog/Projects/android-ndk-r9/sources/android/arm/include \
    --pkgconfigdir=/home/hubdog/Projects/android-ndk-r9/sources/android/arm/lib/pkgconfig \
    --arch=arm \
    --cpu=armv7-a \
    --cross-prefix="${NDK_HOME}/toolchains/arm-linux-androideabi-4.9/prebuilt/${NDK_HOST_PLATFORM}/bin/arm-linux-androideabi-" \
    --sysroot="${NDK_HOME}/platforms/android-19/arch-arm/" \
    --extra-cflags="-I${ASM}/arm-linux-androideabi -isysroot $ISYSROOT -I/home/hubdog/Projects/android-ndk-r9/sources/android/arm/include -march=armv7-a -mfloat-abi=softfp -mfpu=neon" \
    --extra-ldflags="-L/home/hubdog/Projects/android-ndk-r9/sources/android/arm/lib -lz -lm -lmp3lame" \
    ${COMMON_OPTIONS}
    
    sudo make clean
    sudo make -j8 && make install
};

function build_armv8 {
    ./configure \
    --libdir=${PREFIX}/libs/arm64-v8a \
    --incdir=${PREFIX}/includes/arm64-v8a \
    --pkgconfigdir=${PREFIX}/pkgconfig/arm64-v8a \
    --arch=aarch64 \
    --cpu=armv8-a \
    --cross-prefix="${NDK_HOME}/toolchains/aarch64-linux-android-4.9/prebuilt/${NDK_HOST_PLATFORM}/bin/aarch64-linux-android-" \
    --sysroot="${NDK_HOME}/platforms/android-21/arch-arm64/" \
--extra-cflags="-I${ASM}/aarch64-linux-android -isysroot $ISYSROOT " \
    --extra-ldexeflags=-pie \
    ${COMMON_OPTIONS}
    make clean
    make -j8 && make install
}

function build_x86 {
    ./configure \
    --libdir=${PREFIX}/libs/x86 \
    --incdir=${PREFIX}/includes/x86 \
    --pkgconfigdir=${PREFIX}/pkgconfig/x86 \
    --arch=x86 \
    --cpu=i686 \
    --cross-prefix="${NDK_HOME}/toolchains/x86-4.9/prebuilt/${NDK_HOST_PLATFORM}/bin/i686-linux-android-" \
    --sysroot="${NDK_HOME}/platforms/android-19/arch-x86/" \
--extra-cflags="-I${ASM}/i686-linux-android -isysroot $ISYSROOT " \
    --extra-ldexeflags=-pie \
    ${COMMON_OPTIONS}
    make clean
    make -j8 && make install
};

function build_x86_64 {
    ./configure \
    --libdir=${PREFIX}/libs/x86_64 \
    --incdir=${PREFIX}/includes/x86_64 \
    --pkgconfigdir=${PREFIX}/pkgconfig/x86_64 \
    --arch=x86_64 \
    --cpu=x86_64 \
    --cross-prefix="${NDK_HOME}/toolchains/x86_64-4.9/prebuilt/${NDK_HOST_PLATFORM}/bin/x86_64-linux-android-" \
    --sysroot="${NDK_HOME}/platforms/android-21/arch-x86_64/" \
--extra-cflags="-I${ASM}/x86_64-linux-android -isysroot $ISYSROOT " \
    --extra-ldexeflags=-pie \
    ${COMMON_OPTIONS}
    make clean
    make -j8 && make install
};

if [ "$*" ]
then
	ARCHS="$*"
fi

if [ "$COMPILE" ]
then

	CWD=`pwd`
	for ARCH in $ARCHS
	do
		echo "building $ARCH..."
        if [ "$ARCH" = "armv8-a" ]
        then
            build_armv8
		fi

        if [ "$ARCH" = "armv7-a" ]
        then
            build_armeabi

		fi

        if [ "$ARCH" = "x86_64" ]
        then
            build_x86
		fi

        if [ "$ARCH" = "i686" ]
        then
            build_x86_64

		fi
		
	done
fi

echo Done
