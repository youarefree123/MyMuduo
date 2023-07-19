#!/bin/bash

set -e

# 清空
# rm -rf `pwd`/build/*

# 编译googletest
cd `pwd`/3rdparty/googletest

if [ ! -d `pwd`/build ]; then
    mkdir `pwd`/build
    cd build && 
    cmake .. && 
    make -j8 && 
    sudo make install &&
    sudo ldconfig
    

    # ubuntu 会额外创建一个/usr/local/lib/x86_64-linux-gnu文件夹
    # 所以需要把库文件移出来
    sudo cp -r /usr/local/lib/x86_64-linux-gnu/*.a /usr/local/lib/

    # 回到主目录
    cd ../../../
fi




# 如果没有build文件夹，就创建
if [ ! -d `pwd`/build ]; then
    mkdir `pwd`/build
fi

# 编译
cd `pwd`/build &&
    cmake .. && 
    make -j8
cd ../