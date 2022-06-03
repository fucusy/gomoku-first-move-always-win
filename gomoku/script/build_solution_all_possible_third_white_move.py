import os
import json


from divided_solution_manager import apply_all_transformation_for_steps_str, apply_transformation, find_next_steps_from_board_str_hash2action
from divided_solution_manager import get_all_step_str2action, normalize_steps_str, steps2board_str, board_str_hash, move2position

board_str_hash2action = get_all_step_str2action()


def get_next_black_move(step_str):
    """

    :param step_str: h8_h5
    :return:
    """
    board_str = steps2board_str(step_str)
    if board_str in board_str_hash2action:
        return board_str_hash2action[board_str]
    else:
        cmd = 'export LD_LIBRARY_PATH=/usr/local/clang_9.0.0/lib:$LD_LIBRARY_PATH&&%s/web_search %s' % (os.getcwd(), step_str)
        print(cmd)
        res = json.loads(os.popen(cmd).read().strip("\n"))
        next_move = chr(res['x'] + ord('a')) + str(res['y'] + 1)
        return next_move

if __name__ == '__main__':
    # 1. start with 35 board,
    two_positions = ["h8_h5",
    "h8_h4",
    "h8_h3",
    "h8_h2",
    "h8_h1",
    "h8_g5",
    "h8_g4",
    "h8_g3",
    "h8_g2",
    "h8_g1",
    "h8_f5",
    "h8_f4",
    "h8_f3",
    "h8_f2",
    "h8_f1",
    "h8_e5",
    "h8_e4",
    "h8_e3",
    "h8_e2",
    "h8_e1",
    "h8_d4",
    "h8_d3",
    "h8_d2",
    "h8_d1",
    "h8_c3",
    "h8_c2",
    "h8_c1",
    "h8_b2",
    "h8_b1",
    "h8_a1",
    "h8_i7",
    "h8_i6",
    "h8_h7",
    "h8_h6",
    "h8_j6"]


    # 2. find each black step by web_search
    three_position = []
    for step_str in two_positions:
        next_move = get_next_black_move(step_str)
        three_position.append("%s_%s" % (step_str, next_move))
    print(three_position)

    # 3. loop all white possible move, 35 * 2xx four position
    four_position = []
    for step_str in three_position:
        for i in range(15):
            for j in range(15):
                positions = step_str.split("_")
                possible_white_move = chr(i + ord('a')) + str(j + 1)
                if possible_white_move not in positions:
                    four_position.append("%s_%s" % (step_str, possible_white_move))

    # 4. find each black step by web_search, 35 * 2xx four position + guided black step
    # generate problem_comb four_position + guilded black step

    with open("../../third_move_all_solutions_all.sh", "w") as f:
        for step_str in sorted(four_position):
            next_move = get_next_black_move(step_str)
            cmd = "./problem_comb.sh %s %s" % (step_str, next_move)
            print(cmd)
            f.write(cmd + "\n")

