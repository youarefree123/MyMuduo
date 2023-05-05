#!/bin/bash

set -e

# 如果没有build文件夹，就创建
if [ ! -d `pwd`/build ]; then
    mkdir `pwd`/build
fi

# 清空
# rm -rf `pwd`/build/*

# 编译
cd `pwd`/build &&
    cmake .. && 
    make -j8
# cd ..