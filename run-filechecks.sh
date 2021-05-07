#!/bin/bash

if [ "$#" -ne 1 ]; then
  echo "run-filecheck.sh <FileCheck path>"
  exit 1
fi

mkdir -p test

echo "--- Start FileCheck.. ---"
set -e

for i in `find ./filechecks -name "*.ll"` ; do
  echo "--- $i ---"
  bin/sf-compiler $i test/tmp.s
  $1 $i < test/tmp.s
done
