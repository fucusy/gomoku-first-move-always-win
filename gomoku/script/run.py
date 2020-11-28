import sys
import multiprocessing

from multiprocessing import Pool
import os, time
from divided_solution_manager import get_divided_tree
from divided_solution_manager import is_solved_io

def run_sub_problem(filename):
    print('Run task %s (%s)...' % (filename, os.getpid()))
    start = time.time()
    cmd_line = "../noninteractive %s" % (filename)
    print(cmd_line)
    os.system(cmd_line)
    end = time.time()
    print('task %s runs %0.2f seconds.' % (filename, (end - start)))


if __name__ == '__main__':
    selected_black_move = sys.argv[1]
    dry_run = len(sys.argv) > 2
    filenames = [f for f in get_divided_tree() if not f.endswith(".hitted_record.txt") and not f.endswith("board2action.txt")]
    selected_sub_problem = []
    for name in filenames:
        solution_filename = "%s.board2action.txt" % name
        steps = name.split("/")[-1].strip(".txt")
        if name.find(selected_black_move) > 0:
            if os.path.exists(solution_filename):
                print("skip solution existing %s" % name)
            elif is_solved_io(steps):
                print("solution is solved in sub directory %s " % name)
            else:
                selected_sub_problem.append(name)
    for f in selected_sub_problem:
        print(f)
    print(len(selected_sub_problem))

    if not dry_run:
        p = Pool(multiprocessing.cpu_count())
        for name in selected_sub_problem:
            p.apply_async(run_sub_problem, args=(name,))
        print('Waiting for all subprocesses done...')
        p.close()
        p.join()
        print('All subprocesses done.')
