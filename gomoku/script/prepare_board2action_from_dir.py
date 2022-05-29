from divided_solution_manager import bit_board, apply_all_transformation_for_steps_str, apply_transformation
from divided_solution_manager import get_divided_tree, is_solved_io, is_solved

def generate_board_str2action(step_str, divided_folder):
    """

    :param step_str:
    :param divided_folder:
    :return: return "" if not solved
    """
    all_folder_names2txt = {}
    for f in divided_folder:
        folder_name = f.split("/")[2]
        if folder_name in all_folder_names2txt:
            all_folder_names2txt[folder_name].append(f)
        else:
            all_folder_names2txt[folder_name] = [f]

    any_folder_solved = []
    for f in all_folder_names2txt.keys():
        if (len(f) == len(step_str) + 4 or len(f) == len(step_str) + 3) and f.startswith(step_str):
            any_folder_solved.append(f)

    for folder in any_folder_solved:
        filenames = all_folder_names2txt[folder]
        board_situations = []
        for name in filenames:
            if not name.endswith(".board2action.txt") and not name.endswith(".hitted_record.txt"):
                board_situations.append(name)
        all_board_solved = True
        for board_s in board_situations:
            board_s_str = board_s.split("/")[3].strip(".txt")
            if not all_board_solved:
                break
            all_board_solved = ("%s.board2action.txt" % board_s) in filenames or is_solved(board_s_str, divided_folder)
        is_solved_flag = all_board_solved
        action = folder.split("_")[-1]
        if is_solved_flag:
            return action
    return ""


def find_step_str2action_from_directory(divided_folder):
    step_strs = set()
    for f in divided_folder:
        folder_name = f.split("/")[2]
        step_str = "_".join(folder_name.split("_")[:-1])
        step_strs.add(step_str)
    step_str2action = {}
    for i, s in enumerate(step_strs):
        print("process %d/%d %s" % (i, len(step_strs), s))
        action = generate_board_str2action(s, divided_folder)
        if action != "":
            step_str2action[s] = action
    return step_str2action


def find_step_str2action_from_directory_io():
    filenames = get_divided_tree()
    step2action = find_step_str2action_from_directory(filenames)
    return step2action


if __name__ == '__main__':
    step2action = find_step_str2action_from_directory_io()
    print("cache step2action size ", len(step2action))
    # /h8_h7_g7_g6_f6_e5_i8_g8_i9_j10_h9_i6_f9_g9_g10/
    # add them to board2action
    with open("board2action_from_dir.txt", 'w') as f:
        for steps, action in sorted(step2action.items()):
            f.write("%s:%s\n" % (steps, action))