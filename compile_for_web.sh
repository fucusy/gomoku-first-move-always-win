#!/usr/bin/env bash
cd gomoku


if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  echo "I am linuX"
  # TODO how to install clang_9.0.0
  export PATH=/usr/local/clang_9.0.0/bin:$PATH
  export LD_LIBRARY_PATH=/usr/local/clang_9.0.0/lib:$LD_LIBRARY_PATH
  clang++ -stdlib=libc++ -Wall -pedantic *.cpp -o ./script/web_search -std=c++17 -DWEB
elif [[ "$OSTYPE" == "darwin"* ]]; then
  echo "I am mac"
  g++  *.cpp  -o ./script/web_search -std=c++17 -DWEB
else
  echo "this platform is not supported"
fi

