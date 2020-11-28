#!/usr/bin/env bash
cd gomoku
rm unittest
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  echo "I am linuX"
  export PATH=/usr/local/clang_9.0.0/bin:$PATH
  export LD_LIBRARY_PATH=/usr/local/clang_9.0.0/lib:$LD_LIBRARY_PATH
  clang++ -stdlib=libc++ -Wall -pedantic *.cpp -o unittest -std=c++17 -DUNITTEST
elif [[ "$OSTYPE" == "darwin"* ]]; then
  echo "I am mac"
  g++  *.cpp -o unittest -std=c++17 -DUNITTEST
else
  echo "this platform is not supported"
fi
./unittest

cd script
python3 test.py
