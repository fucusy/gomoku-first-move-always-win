import rocksdb
from divided_solution_manager import get_all_step_str2action_tuple, board_str2steps



db = rocksdb.DB("no_restrituion_gomoku.db", rocksdb.Options(create_if_missing=True))

i = 0
for key, value in get_all_step_str2action_tuple():
    db_value = db.get(str.encode(key))
    if(db_value != str.encode(value)):
        print(board_str2steps(key), "db value", db_value, 'folder result value', value)
    i += 1
    if i % 10000 == 0:
        print(db_value, value)
        print(i)
