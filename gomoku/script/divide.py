import os
import sys


def possible_white(step_list):
    steps = set()
    dirs = []
    # support white move at all position
    for i in range(-7, 8):
        for j in range(-7, 8):
            dirs.append([i, j])
    for s in step_list:
        x = ord(s[0]) - ord('a')
        y = int(s[1:]) - 1
        for d in dirs:
            xn = x + d[0]
            yn = y + d[1]
            if 0 <= xn < 15 and 0 <= yn < 15:
                next_step = "%s%s" % (chr(xn + ord('a')), str(yn+1))
                steps.add(next_step)
    for s in step_list:
        if s in steps:
            steps.remove(s)
    return list(steps)


if __name__ == '__main__':
    part_board_name = sys.argv[1]
    next_black_name = sys.argv[2]

    step_list = []
    step_list_plus_black = []
    for line in open(part_board_name, "r"):
        step_list.append((line.strip()))

    for line in open(next_black_name, "r"):
        new_list = step_list[:]
        new_list.append(line.strip())
        step_list_plus_black.append(new_list)

    for item in step_list_plus_black:
        top_dir_name = "divided/"
        if not os.path.isdir(top_dir_name):
            os.mkdir(top_dir_name)
        dir_name = top_dir_name + "_".join(item)
        if not os.path.isdir(dir_name):
            os.mkdir(dir_name)
        for white in possible_white(item):
            new_item_list = item[:]
            new_item_list.append(white)
            filename = "./%s/%s%s" % (dir_name, "_".join(new_item_list), ".txt")
            print(filename)
            with open(filename, "w") as f:
                for step in new_item_list:
                    f.write(step + '\n')
