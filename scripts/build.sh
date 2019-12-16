#!/usr/bin/env bash

if [[ ! -d "../build/cmake-build-debug" ]]; then
  mkdir ../build/cmake-build-debug
fi

cd ../build/cmake-build-debug
cmake ../..
cd ../../scripts
cmake --build ../build/cmake-build-debug --target v2x -- -j 2