#!/bin/bash

if [ "$#" -ne 1 ]; then
  echo "configure.sh <clang dir>"
  echo "ex)  ./configure.sh ~/llvm-12.0-releaseassert/bin"
  exit 1
fi

if [[ "$OSTYPE" == "darwin"* ]]; then
  ISYSROOT="-isysroot `xcrun --show-sdk-path`"
else
  ISYSROOT=
fi

LLVMCONFIG=$1/llvm-config

CXXFLAGS=`$LLVMCONFIG --cxxflags`
SRCROOT=`$LLVMCONFIG --src-root`
CXXFLAGS="$CXXFLAGS -std=c++17 -g -I${SRCROOT}/include ${ISYSROOT}"
CXXFLAGS="$CXXFLAGS -I${SRCROOT}/utils/unittest/googletest/include"
CXXFLAGS="$CXXFLAGS -I${SRCROOT}/utils/unittest/googletest/"

GTESTSRC="${SRCROOT}/utils/unittest/googletest/src/gtest-all.cc"

LDFLAGS=`$LLVMCONFIG --ldflags`
LIBS=`$LLVMCONFIG --libs core irreader bitreader support analysis asmparser passes --system-libs`
LDFLAGS="$LDFLAGS -Wl,-rpath,`$LLVMCONFIG --libdir`"
LDFLAGS="$LDFLAGS $LIBS -lpthread -lm -fPIC"

echo "CXX=$1/clang++" >Makefile
echo "CXXFLAGS=${CXXFLAGS}" >>Makefile
echo "LDFLAGS=${LDFLAGS}" >>Makefile
echo "GTESTSRC=${GTESTSRC}" >> Makefile
echo "FILECHECK_PATH=$1/FileCheck" >> Makefile
cat Makefile.template >>Makefile
