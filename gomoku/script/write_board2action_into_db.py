#import rocksdb
from divided_solution_manager import get_all_step_str2action



#db = rocksdb.DB("no_restrituion_gomoku.db", rocksdb.Options(create_if_missing=True))
board2action = get_all_step_str2action()

for key, value in board2action.items():
    print(int.encode(key), str.encode(value))
