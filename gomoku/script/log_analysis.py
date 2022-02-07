import sys
import platform
import os
import json
from divided_solution_manager import get_all_step_str2action, normalize_steps_str, steps2board_str, board_str_hash
from divided_solution_manager import apply_all_transformation_for_steps_str, apply_transformation, find_next_steps_from_board_str_hash2action
from divided_solution_manager import move2position, position2move
from divided_solution_manager import bit_board

def has_black_line(steps_str):
    """
    :param steps_str:
    :return: bool
    >>> has_black_line("h8_h9")
    False
    >>> has_black_line("h8_i8_i7_g9_j7_h7_j6_i6_j5_j8_i4_i5_j4_j3_l4_k4_k5_l6_m3_n2")
    True
    """
    steps = steps_str.split("_")
    black_steps = []
    for i, s in enumerate(steps):
        if i % 2 == 0:
            black_steps.append(s)
    for s in black_steps:
        x, y = move2position(s)
        for d_x, d_y in [(1, 1), (1, -1), (-1, 1), (-1, -1), (0, 1), (0, -1), (1, 0), (-1, 0) ]:
            found = True
            for distance in range(1, 5):
                target_x = x + distance * d_x
                target_y = y + distance * d_y
                move = position2move(target_x, target_y)
                if move not in black_steps:
                    found = False
            if found:
                return True
    return False


def qualified_white_move(steps_str):
    """
    :param steps_str h8_h9_i9_i10
    :return: all transformation move
    >>> qualified_white_move("h8_h9")
    True
    >>> qualified_white_move("h8_i9_i7_e15")
    False
    >>> qualified_white_move("h8_h9_g9_l6")
    False
    """
    steps = steps_str.split("_")
    if len(steps) % 2 != 0:
        return False
    for i, s in enumerate(steps):
        if i == 0:
            continue
        min_distance = 10
        for previous_s in steps[:i]:
            try:
                distance = abs(ord(s[0]) - ord(previous_s[0])) + abs(int(s[1:]) - int(previous_s[1:]))
            except BaseException as e:
                print(e)
                print(steps_str)
                return False
            if distance < min_distance:
                min_distance = distance
        if min_distance >= 3:
            return False
    return True


if __name__ == "__main__":
    import doctest
    doctest.testmod()

    global board_str_hash2action
    board_str_hash2action = get_all_step_str2action()
    print("cache step_str2action size ", len(board_str_hash2action))

    def is_cached(steps_str):
        new_format_steps, trans = apply_all_transformation_for_steps_str(steps_str)
        for steps, tran in zip(new_format_steps, trans):
            norm_steps = board_str_hash(steps2board_str(steps))
            if norm_steps in board_str_hash2action:
                return True
        return False


    def qualified_black_move(steps_str):
        """
        :param steps_str h8_h9_i9_i10
        :return: False or True
        """
        steps = steps_str.split("_")
        if steps[0] != "h8":
            return False
        steps = steps_str.split("_")
        for i, s in enumerate(steps):
            if i == 0:
                continue
            if i % 2 != 0:
                # skip white move
                continue
            previous_step_str = "_".join(steps[:i])
            if is_cached(previous_step_str):
                next_moves = find_next_steps_from_board_str_hash2action(previous_step_str, board_str_hash2action)
                if s not in next_moves:
                    print("next move for %s should be %s not %s" % (previous_step_str, next_moves, s))
                    return False
        return True

    assert qualified_black_move("h8_i8_i7_j7_j6_k5_h9_h7_g9_f10_g8_g10_i10_f7")
    assert not qualified_black_move("h8_i9_e7_e15")

    dirname = sys.argv[1]
    print("parsing logs from dir %s" % dirname)
    filenames = os.listdir(dirname)


    response_time_and_line = []
    no_response_time_count = 0
    total_line = 0
    steps_str_set = set()
    qualified_steps_str = set()
    for filename in filenames:
        print(filename)
        path = "%s/%s" % (dirname, filename)
        for line in open(path, 'r'):
            total_line += 1
            line = line.strip()
            if line.endswith("ms"):
                ms = line.split(" ")[-1].strip("ms")
                prefix = "stepsString=_"
                suffix = "&color=BLACK"
                if line.find(prefix) >= 0 and line.find(suffix) >= 0:
                    steps_str = line[line.find(prefix) + len(prefix):line.find(suffix)]
                    response_time_and_line.append((float(ms), line, steps_str))
            else:
                no_response_time_count += 1

    large_response_line_count = 0
    for t in response_time_and_line:
        if t[0] > 3000:
            steps_str = t[2]
            steps_str_set.add(steps_str)
            if qualified_white_move(steps_str) and not is_cached(steps_str) \
                    and qualified_black_move(steps_str) and not has_black_line(steps_str):
                qualified_steps_str.add(steps_str)
            large_response_line_count += 1

    # h8_g8_g7_f7_f6_e5_h9_h7_i9_j10_i8_i7_j7_k6_g10_f11_h10_f9_i6_e10_d11_d9_c8_e9
    print(">3000ms response steps str")
    count = 0
    # the file needs to be append to ./web_search_cache_file.txt manually
    with open("./web_search_cache_file.txt", "a") as f:
        for s in qualified_steps_str:
            count += 1
            timeout_program = "gtimeout"
            if platform.system() == 'Linux':
                timeout_program = 'timeout'
            cmd = 'export LD_LIBRARY_PATH=/usr/local/clang_9.0.0/lib:$LD_LIBRARY_PATH&&%s 1s %s/web_search %s' % (
            timeout_program, os.getcwd(), s)
            res = os.popen(cmd).read().strip()
            move = "not_found"
            if res != "":
                res_dict = json.loads(res)
                move = position2move(res_dict["x"], res_dict["y"])
                b = bit_board()
                b.steps(s)
                board_str = str(b)
                f.write("%s:%s\n" % (board_str, move))
            print("%s:%s" % (s, move))

    print(">3000ms response steps str count: %s" % len(steps_str_set))
    print(">3000ms response qualified steps str count: %s" % len(qualified_steps_str))
    print("total_line", total_line)
    print("no_response_time_count", no_response_time_count)
    print("large_response_line_count", large_response_line_count)


