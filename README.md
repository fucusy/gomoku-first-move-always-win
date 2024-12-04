# Gomoku, first move always win AI 
Inspired by https://github.com/davda54/pn-search: C++ implementation of proof-number and dependency-based search for gomoku by [Louis Victor Allis (1994)](http://fragrieu.free.fr/SearchingForSolutions.pdf)

Win rule: five or more stones in a line win

# Try it 
video demo: https://www.bilibili.com/video/BV1Sf4y1i7df

try it online: http://www.bytedance.ai/gomoku.html

run it from codebase: 
```bash
bash ./compile_for_web.sh
cd gomoku/script
python3 ./tornado_server.py 80 debug
```
open url: http://127.0.0.1/web/gomoku.html


# Requirement
System
- Linux/macOS, Tested in ubuntu 18.04, macOS 13.6

Language
- Python 3.x, Tested in 3.9.0, `conda create --name gomoku python=3.9`
- C++ 17

Package
- Python Package tornado 6.1
- https://github.com/NightTsarina/python-rocksdb 0.8, pip install rocksdb==0.8
- gtimeout  `brew install coreutils` on macOS


macOS: brew install alexreg/dev/rocksdb@6 for python lib
- `brew install pkg-config`
- `brew install alexreg/dev/rocksdb@6`
- `brew install snappy`

"""
rocksdb@6 is keg-only, which means it was not symlinked into /usr/local,
because this is an alternate version of another formula.

If you need to have rocksdb@6 first in your PATH, run:
    echo 'export PATH="/usr/local/opt/rocksdb@6/bin:$PATH"' >> ~/.zshrc

For compilers to find rocksdb@6 you may need to set:
    export LDFLAGS="-L/usr/local/opt/rocksdb@6/lib"
    export CPPFLAGS="-I/usr/local/opt/rocksdb@6/include"
For pkg-config to find rocksdb@6 you may need to set:
    export PKG_CONFIG_PATH="/usr/local/opt/rocksdb@6/lib/pkgconfig"
"""

Linux: 

C++
- RocksDB 6

    git@github.com:facebook/rocksdb.git da11a5903 v6.xx

    g++ rocksdb_test.cpp -o test -I/Users/chenqiang/Documents/gthub/rocksdb/include/ -std=c++17 -lsnappy -lgflags -lz -lbz2 -llz4 -lzstd /usr/local/Cellar/jemalloc/5.2.1_1/lib/libjemalloc_pic.a  /Users/chenqiang/Documents/github/rocksdb/librocksdb.a




# AI limitation
The AI might not work if white move far away 3 steps from any existing move

# Development Guide to solve an board
1. encode your board by string,
for example: for ![](./opennings/pu_yue.png), you need to encode it to h8_i9, 
this is the board to solve,
, and last black move is i10, this is guided move
- board to solve: h8_i9
- guided move: i10

2. solve your board with guided move
```bash
./problem_comb.sh h8_i9 i10
```

during the time, in order to see more debug info, run
```bash
tail -f gomoku/script/run.py.log
```


3. validate if the board get solved
```bash
./problem_manager.sh h8_i9
```
if nothing print, the board get solved, if not then you need to solve
listed board in "AND" node

4. for the listed board, go step 1 to solve it, you need find guided move yourself

5. tested in UI by launching it locally
```bash

bash ./compile_for_web.sh
cd gomoku/script
python3 ./prepare_board2action_from_dir.py
python3 ./write_board2action_into_db.py
python3 ./tornado_server.py 80 debug
```
open url: http://127.0.0.1/web/gomoku.html

# Server Maintenance
create .plist files to start cloudflare tunnel and gomoku backend service, place them in folder /Library/LaunchDaemons/

