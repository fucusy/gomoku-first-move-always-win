import rocksdb
from divided_solution_manager import get_all_step_str2action_tuple



db = rocksdb.DB("no_restrituion_gomoku.db", rocksdb.Options(create_if_missing=True))

i = 0
for key, value in get_all_step_str2action_tuple():
    db.put(str.encode(key), str.encode(value))
    #db.get(str.encode(key))
    i +=  1
    if i % 10000 == 0:
        #print(db.get(str.encode(key)))
        print(i)
