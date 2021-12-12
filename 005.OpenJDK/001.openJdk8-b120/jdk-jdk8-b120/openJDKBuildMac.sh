#!/bin/bash
export COMPILER_WARNINGS_FATAL=false
# OpenJDK构建官方教程: https://openjdk.java.net/groups/build/doc/building.html
# build之前，需要安装freetype,使用如下命令安装
# sudo apt-get install libpng12-dev zlib1g-dev
# sudo apt-get install libpng-dev
# sudo apt-get install libx11-dev libxext-dev libxrender-dev libxtst-dev libxt-dev
# sudo apt-get install libfreetype6-dev

# Step1. 清除之前构建的
make clean CONF=linux-x86_64-normal-server-slowdebug

# freetype路径配置
FREETYPEINCLUDE='/opt/homebrew/include/freetype2'
FREETYPELIB='/opt/homebrew/Cellar/freetype/2.11.0/lib'
# BootJDK配置
BOOTJDK='/Library/Java/JavaVirtualMachines/jdk1.7.0_80.jdk/Contents/Home'
# 构建文件输出目录
BUILDOUTPUTDIR=`pwd`/build

# Step2. 运行configure,可能会缺依赖，缺什么就装什么.使用‘\’换行后，注意注释的写法
./configure  --with-debug-level=slowdebug    `# 指定可以生成最多的调试信息` \
    --with-boot-jdk=${BOOTJDK}  \
    --with-freetype-include=${FREETYPEINCLUDE} \
    --with-freetype-lib=${FREETYPELIB} \
    --with-target-bits=32   `#指定编译64位系统的JDK；` \
    --enable-debug-symbols ZIP_DEBUGINFO_FILES=0 `#ZIP_DEBUGINFO_FILES：生成调试的符号信息，并且不压缩,这样才可以进行源码调试；` \
    OBJCOPY=gobjcopy

# 判断一下configure的执行结果
if [[ $? -ne 0 ]]; then
    echo "configure 执行失败，即构建失败"
    exit
fi

# Step3. 构建OpenJdk
#语言选项，这个必须设置，否则编译好后会出现一个HashTable的NPE错
export LANG=C
# java文件编码设置
export JAVA_TOOL_OPTIONS=-Dfile.encoding=UTF-8
#Bootstrap JDK的安装路径。必须设置
export ALT_BOOTDIR=${BOOTJDK}
#允许自动下载依赖
export ALLOW_DOWNLOADS=true
#并行编译的线程数，设置为和CPU内核数量一致即可
export HOTSPOT_BUILD_JOBS=8
export ALT_PARALLEL_COMPILE_JOBS=8
#比较本次build出来的映像与先前版本的差异。这对我们来说没有意义，
#必须设置为false，否则sanity检查会报缺少先前版本JDK的映像的错误提示。
#如果已经设置dev或者DEV_ONLY=true，这个不显式设置也行
export SKIP_COMPARE_IMAGES=true
#使用预编译头文件，不加这个编译会更慢一些
export USE_PRECOMPILED_HEADER=true
#要编译的内容
export BUILD_LANGTOOLS=true
export BUILD_HOTSPOT=true
export BUILD_JDK=true
#把它设置为false可以避开javaws和浏览器Java插件之类的部分的build
BUILD_DEPLOY=false
#把它设置为false就不会build出安装包。因为安装包里有些奇怪的依赖，但即便不build出它也已经能得到完整的JDK映像，所以还是别build它好了
BUILD_INSTALL=false
#编译结果所存放的路径
export ALT_OUTPUTDIR=${BUILDOUTPUTDIR}
#这两个环境变量必须去掉，不然会有很诡异的事情发生，Makefile脚本检查到有这2个变量就会提示警告）
unset JAVA_HOME
unset CLASSPATH
export COMPILER_WARNINGS_FATAL=false
# 将make all修改为make images,不构建docs，减少构建时间
# ZIP_DEBUGINFO_FILES=0 参数很重要，即不对debuginfo进行压缩，只有不进行压缩，才可以进行源码调试
make images CONF=macosx-arm-normal-server-slowdebug ZIP_DEBUGINFO_FILES=0  2>&1|tee $ALT_OUTPUTDIR/build.log

# 判断一下构建结果
if [[ $? -ne 0 ]]; then
    echo "构建失败"
    exit
else
    echo "构建成功"
    exit
fi

# ./configure  --with-target-bits=64 --with-freetype-include=/usr/X11/include/freetype2 --with-freetype-lib=/usr/X11/lib --disable-warnings-as-errors --with-debug-level=slowdebug  --with-native-debug-symbols=internal