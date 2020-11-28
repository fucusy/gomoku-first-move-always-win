import sys
import multiprocessing

from multiprocessing import Pool
import os, time
from divided_solution_manager import bit_board, read_board2action_filenames


if __name__ == '__main__':

    steps = sys.argv[1]
    print("steps", steps)
    b = bit_board()
    b.steps(steps)
    board_str = str(b)
    print("board str for given steps %s is %s" % (steps, board_str))
    board2action_filenames = read_board2action_filenames()
    duplication_count = 0
    total_count = len(board2action_filenames)
    print(board2action_filenames[board_str])
