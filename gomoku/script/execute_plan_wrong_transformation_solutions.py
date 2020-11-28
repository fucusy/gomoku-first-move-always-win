import os


import sys
import platform
import datetime
from divided_solution_manager import board_str2steps, apply_all_transformation_for_steps_str, bit_board, apply_transformation

def creation_date(path_to_file):
    """
    Try to get the date that a file was created, falling back to when it was
    last modified if that isn't possible.
    See http://stackoverflow.com/a/39501288/1709587 for explanation.
    """
    return datetime.datetime.fromtimestamp(os.path.getmtime(path_to_file))

if __name__ == '__main__':
    action_filename = sys.argv[1]
    board_str2action = {}
    for line in open(action_filename):
        board_str_with_issue, steps_url, action_with_issue, next_move = line.rstrip('\n').split("\t")
        board_str2action[board_str_with_issue] = next_move

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
            created_filenames.append(row)

    # the solution with hitted record may have some problem
    probmatic_solved_step_strs = set()
    for f, c_time in created_filenames:
        if f.endswith("hitted_record.txt") or f.endswith("board2action.txt"):
            if f.endswith("/h8_h7_i7_f8.txt.board2action.txt"):
                print(f)
            tmp_filename = "%s.tmp" % f
            with open(tmp_filename, 'w') as tmp_f:
                for line in open(f):
                    existing_board_str, existing_action = line.rstrip('\n').split(":")
                    corrected_action = existing_action
                    if existing_board_str in board_str2action:
                        correct_action = board_str2action[existing_board_str]
                        if correct_action != existing_action:
                            print("correct from %s to %s for filename:%s, line board str %s, " % (existing_action, correct_action, f, existing_board_str))
                            corrected_action = correct_action
                    tmp_f.write("%s:%s\n" % (existing_board_str, corrected_action))
            os.system("mv %s %s" % (tmp_filename, f))
