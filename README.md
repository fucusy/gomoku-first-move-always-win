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


# Requirement
System
- Linux/macOS, Tested in ubuntu 18.04, macOS 10.15.4

Language
- Python 3.x, Tested in 3.9.0
- C++ 17

Package
- Python Package tornado 6.1

# Solved opening

# Development Guide to solve an opening
1.encode your board by string,
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

4. for the listed board, go 1) step to solve it, you need find guided move yourself

5. tested in UI by launching it locally
```bash

bash ./compile_for_web.sh
cd gomoku/script
python3 ./prepare_board2action_from_dir.py
python3 ./tornado_server.py 80 debug
```