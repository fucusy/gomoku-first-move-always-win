#!/usr/bin/env bash
cd gomoku


if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  echo "I am linuX"
  clang++ -stdlib=libc++ -Wall -pedantic *.cpp -o ./script/web_search -std=c++17 -DWEB
elif [[ "$OSTYPE" == "darwin"* ]]; then
  echo "I am mac"
  g++  *.cpp  -o ./script/web_search -std=c++17 -DWEB
else
  echo "this platform is not supported"
fi

