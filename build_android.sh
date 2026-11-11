#!/bin/bash

# --- 1. 配置区 (基于您的 NDK r20b 范例) ---

# 【关键修改】: 使用您指定的 NDK r20b
export NDK_HOME=$HOME/Projects/android-ndk-r20b

# 设置目标 Android API 级别 (与您范例保持一致或更高)
API=23

# 设置编译产物的输出根目录 (FFmpeg 自身的库将安装在这里)
PREFIX=$(pwd)/android-build

# 设置编译工具链的主机平台
HOST_PLATFORM=linux-x86_64

# --- 2. 依赖库路径配置 (独立目录, 已从 android-ndk-r9 迁出) ---
# 第三方依赖库 (x264/openssl/lame 等) 现在位于独立目录
DEPS_BASE_PATH=/home/hubdog/Projects/android-deps-arm
DEPS_LIB_PATH=$DEPS_BASE_PATH/lib
DEPS_INCLUDE_PATH=$DEPS_BASE_PATH/include
DEPS_PKG_CONFIG_PATH=$DEPS_BASE_PATH/lib/pkgconfig

# --- 3. armv7-a 架构专属配置 (模仿您的 armv8 脚本) ---
build_armv7_a() {
    echo "--- Building for armv7-a using NDK r20b Clang toolchain ---"
    
    ARCH_NAME="armeabi-v7a"
    OUTPUT_DIR=$PREFIX/$ARCH_NAME
    
    # 【关键修改】: 使用 NDK r20b 的 LLVM/Clang 工具链
    TOOLCHAIN=$NDK_HOME/toolchains/llvm/prebuilt/$HOST_PLATFORM
    
    # 【关键修改】: 采用 NDK r20b 的编译器命名规则，适配 armv7-a
    # 范例: aarch64-linux-android23-clang
    # 适配: armv7a-linux-androideabi23-clang
    CC=$TOOLCHAIN/bin/armv7a-linux-androideabi$API-clang
    CXX=$TOOLCHAIN/bin/armv7a-linux-androideabi$API-clang++
    
    # 【关键修改】: 适配 armv7-a 的交叉编译工具链前缀
    CROSS_PREFIX=$TOOLCHAIN/bin/arm-linux-androideabi-
    
    # 【关键修改】: 现代 NDK (r19+) 将 sysroot 放在工具链内部，这是最可靠的路径
    SYSROOT=$TOOLCHAIN/sysroot
    
    # 设置 PKG_CONFIG_PATH 环境变量，指向 NDK r9 的路径
    export PKG_CONFIG_PATH=$DEPS_PKG_CONFIG_PATH
    
    # 配置 CFLAGS 和 LDFLAGS，包含 NDK r9 的路径
    CFLAGS="-fPIC -march=armv7-a -mfloat-abi=softfp -mfpu=neon -I$DEPS_INCLUDE_PATH -D__ANDROID_API__=$API"
    # 16 KB page size 兼容: 将 ELF 段对齐提升到 16 KB, 避免 16 KB 页面设备上崩溃
    LDFLAGS="-L$DEPS_LIB_PATH -lz -lm --target=armv7a-linux-androideabi$API -Wl,-z,max-page-size=16384 -Wl,-z,common-page-size=16384"
    
    make clean

    # 执行 configure
    echo "Using Compiler: $CC"
    echo "Using Sysroot: $SYSROOT"
    echo "Using PKG_CONFIG_PATH: $PKG_CONFIG_PATH"
    ./configure \
        --prefix=$OUTPUT_DIR \
        --target-os=android \
        --arch=arm \
        --cpu=armv7-a \
        --enable-cross-compile \
        \
        --cc=$CC \
        --cxx=$CXX \
        --cross-prefix=$CROSS_PREFIX \
        --sysroot=$SYSROOT \
        \
        --pkg-config=pkg-config \
        \
        --extra-cflags="$CFLAGS" \
        --extra-ldflags="$LDFLAGS" \
        \
        --disable-static \
        --enable-shared \
        --enable-jni \
        --enable-mediacodec \
        --enable-small \
        --disable-programs \
        --disable-doc \
        --disable-muxer=encx --disable-muxer=encx_audio \
        --disable-iconv --disable-demuxer=encx --disable-demuxer=encx_audio \
        --enable-nonfree \
        --enable-gpl \
        --enable-version3 \
        --enable-openssl \
        --enable-libx264 \
        --enable-libmp3lame \
        --enable-libfreetype \
        --enable-libspeex \
        --enable-libopencore-amrnb \
        --enable-libopus

    if [ $? -ne 0 ]; then
        echo "Configure failed for armv7-a. Please check the logs."
        exit 1
    fi
    
    sudo make -j$(nproc) && sudo make install
    
    echo "--- Build for armv7-a finished successfully ---"
}

# --- 4. 执行编译 ---
mkdir -p $PREFIX
build_armv7_a
echo "Done."
