#!/bin/bash

if [ "$#" -ne 2 ]; then
  echo "run-benchmark.sh <sf-interpreter path> <benchmark dir>"
  exit 1
fi

function onexit() {
  rm -rf tmp.s sf-interpreter.log sf-interpreter-cost.log
}

echo "--- Start Benchmarks ---"
set -e

trap onexit EXIT

for benchmark in $2/*/ ; do
  if [ "$benchmark" != ".git" ]; then
    source=`find $benchmark/src -name "*.ll"`
    name=`basename $benchmark`
    echo "$name: Compiling..."
    bin/sf-compiler $source tmp.s

    for input in `find $benchmark/test -name "input*"` ; do
      output=${input%"input"*}output${input##*"input"}
      input_name=`basename $input`
      output_name=`basename $output`
      echo "$name: Checking $input_name with $output_name..."
      $1 tmp.s < $input | diff $output -
    done
  fi
done

echo "--- Finished Benchmarks ---"
