import rocksdb
from divided_solution_manager import get_all_step_str2action



db = rocksdb.DB("no_restrituion_gomoku.db", rocksdb.Options(create_if_missing=True))
board2action = get_all_step_str2action()

i = 0
n = len(board2action)
for key, value in board2action.items():
    db.put(str.encode(key), str.encode(value))
    #db.get(str.encode(key))
    i +=  1
    if i % 10000 == 0:
        #print(db.get(str.encode(key)))
        print(i, n)
        print(i/n)
