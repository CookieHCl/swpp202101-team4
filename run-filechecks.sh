#!/bin/bash

if [ "$#" -ne 1 ]; then
  echo "run-filecheck.sh <FileCheck path>"
  exit 1
fi

echo "--- Start FileCheck ---"
set -e

for i in `find ./filechecks -name "*.ll"` ; do
  opt=${i%Pass*}
  if [ "$opt" = "$i" ]; then
    echo "$i: Test all optimizations..."
    bin/sf-compiler $i | $1 $i
  else
    opt=`basename $opt`
    echo "$i: Test $opt optimization..."
    bin/sf-compiler --passes=$opt $i | $1 $i
  fi
done

echo "--- Finished FileCheck ---"