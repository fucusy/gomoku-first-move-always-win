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
  g++  *.cpp  -o ./script/web_search -std=c++17 -DWEB -I/usr/local/Cellar/rocksdb@6/6.29.4/include -lsnappy -lgflags -lz -lbz2 -llz4 -lzstd /usr/local/Cellar/jemalloc/5.3.0/lib/libjemalloc_pic.a /usr/local/Cellar/rocksdb@6/6.29.4/lib/librocksdb.a
else
  echo "this platform is not supported"
fi

