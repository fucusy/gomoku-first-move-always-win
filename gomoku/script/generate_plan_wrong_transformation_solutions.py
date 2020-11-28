import os


import platform
import datetime
from divided_solution_manager import board_str2steps, apply_all_transformation_for_steps_str, bit_board, apply_transformation

def creation_date(path_to_file):
    """
    Try to get the date that a file was created, falling back to when it was
    last modified if that isn't possible.
    See http://stackoverflow.com/a/39501288/1709587 for explanation.
    """
    if platform.system() == 'Windows':
        return os.path.getctime(path_to_file)
    else:
        stat = os.stat(path_to_file)
        try:
            return datetime.datetime.fromtimestamp(stat.st_birthtime)
        except AttributeError:
            # We're probably on Linux. No easy way to get creation dates here,
            # so we'll settle for when its content was last modified.
            return datetime.datetime.fromtimestamp(stat.st_mtime)

if __name__ == '__main__':

    issue_timing = datetime.datetime(year=2020, month=10, day=23)
    folders = []
    top_folder = "../divided"
    for folder in os.listdir(top_folder):
        folder_fullname = "%s/%s/" % (top_folder, folder)
        if os.path.isdir(folder_fullname):
            folders.append(folder_fullname)
    created_filenames = []
    for f in folders:
        for sub_txt in os.listdir(f):
            full_sub_txt = "%s%s" % (f, sub_txt)
            creation_time = creation_date(full_sub_txt)
            row = (full_sub_txt, creation_time)
            if creation_time > issue_timing:
                created_filenames.append(row)

    # the solution with hitted record may have some problem
    probmatic_solved_step_strs = set()
    for f, c_time in created_filenames:
        if f.endswith(".txt.hitted_record.txt"):
            step_str = f.rstrip(".txt.hitted_record.txt")
            probmatic_solved_step_strs.add(step_str)
    print(len(probmatic_solved_step_strs))

    # load no problem board2action
    solution_filenames = []
    top_dir_name = "../divided/"
    for sub_dir in os.listdir(top_dir_name):
        full_dirname = "%s%s/" % (top_dir_name, sub_dir)
        if not os.path.isdir(full_dirname):
            continue
        for filename in os.listdir(full_dirname):
            if filename.endswith("board2action.txt"):
                full_filename = "%s%s" % (full_dirname, filename)
                creation_time = creation_date(full_filename)
                if creation_time < issue_timing:
                    solution_filenames.append(full_filename)
    board2action = {}
    for filename in solution_filenames:
        for line in open(filename, 'r'):
            board_str, action = line.strip("\n").split(":")
            if board_str not in board2action:
                board2action[board_str] = action
    print("cache board2action size ", len(board2action))

    action_file = "./wrong2correct.txt"
    with open(action_file, 'w') as action_handler:
        for f in probmatic_solved_step_strs:
            hitted_filename = "%s.txt.hitted_record.txt" % f
            board2action_filename = "%s.txt.board2action.txt" % f
            for line in open(hitted_filename):
                board_str_with_issue, action_with_issue = line.strip("\n").split(":")
                steps_url = board_str2steps(board_str_with_issue)
                new_format_steps, trans = apply_all_transformation_for_steps_str(steps_url)
                next_move = ""
                before_trans = ""
                tran_action = ()
                for steps, tran in zip(new_format_steps, trans):
                    b = bit_board()
                    b.steps(steps)
                    board_str = str(b)
                    if board_str in board2action:
                        before_trans = board2action[board_str]
                        tran_action = tran
                        next_move = apply_transformation(before_trans, tran, reverse=True)
                        break
                if next_move != "" and action_with_issue != next_move:
                    print("board_str", board_str_with_issue, "steps", steps_url, "wrong_action", action_with_issue, "correct_action", next_move, "before_action", before_trans, "trans", tran_action)
                    action_handler.write("%s\t%s\t%s\t%s\n" % (board_str_with_issue, steps_url, action_with_issue, next_move))
