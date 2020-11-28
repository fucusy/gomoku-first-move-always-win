import sys
import multiprocessing

from multiprocessing import Pool
import os, time
from divided_solution_manager import bit_board, read_board2action_filenames


if __name__ == '__main__':
    solution_filenames = []
    top_dir_name = "../divided/"
    for sub_dir in os.listdir(top_dir_name):
        full_dirname = "%s%s/" % (top_dir_name, sub_dir)
        if not os.path.isdir(full_dirname):
            continue
        for filename in os.listdir(full_dirname):
            if filename.endswith("board2action.txt"):
                full_filename = "%s%s" % (full_dirname, filename)
                solution_filenames.append(full_filename)

    # board_str -> ([actions], [filenames])
    board2action_filenames = {}
    for filename in solution_filenames:
        for line in open(filename, 'r'):
            board_str, action = line.strip("\n").split(":")
            if board_str not in board2action_filenames:
                board2action_filenames[board_str] = ([action], [filename])
            elif filename not in board2action_filenames[board_str][1]:
                board2action_filenames[board_str][0].append(action)
                board2action_filenames[board_str][1].append(filename)

    wrong_board_str = set()
    for filename in open("../../wrong_solutions_filenames.txt"):
        filename = "../" + filename.strip("\n")
        for line in open(filename, 'r'):
            board_str, action = line.strip("\n").split(":")
            wrong_board_str.add(board_str)


    hitted_filenames = []
    for sub_dir in os.listdir(top_dir_name):
        full_dirname = "%s%s/" % (top_dir_name, sub_dir)
        if not os.path.isdir(full_dirname):
            continue
        for filename in os.listdir(full_dirname):
            if filename.endswith(".hitted_record.txt"):
                full_filename = "%s%s" % (full_dirname, filename)
                hitted_filenames.append(full_filename)

    extra_wrong_filenames = []
    for hit_filename in hitted_filenames:
        for line in open(hit_filename, 'r'):
            board_str, action = line.strip("\n").split(":")
            if board_str in wrong_board_str:
                extra_wrong_filenames.append(hit_filename)
                break

    for f in extra_wrong_filenames:
        f = f.strip("\n")
        os.system("rm %s" % f)
        wrong_solution_f = f.replace(".hitted_record.txt", ".board2action.txt")
        print(wrong_solution_f)
        os.system("rm %s" % wrong_solution_f)

    for filename in open("../../wrong_solutions_filenames.txt"):
        filename = "../" + filename.strip("\n")
        os.system("rm %s" % filename)

        wrong_solution_f = filename.replace(".board2action.txt",  ".hitted_record.txt")
        os.system("rm %s" % wrong_solution_f)

