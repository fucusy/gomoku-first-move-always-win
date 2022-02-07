import os
import pprint
import sys
import random

global is_solved_cache
is_solved_cache = {}

def is_solved(board_str, divided_folder):
    """
    1. Solution is stored in ./divided/ 's folders
    2. The solution filename ends with board2action.txt,
    each row in the file means the move for black for each board situation
    3. The folder name mean the move of black and white, the first move and the last move is black,
    those folder is produced by ../build_divide.sh
    For example to counter board situation, h8_g8_g7_f6_i7_h7, the black promising move is
    e9, f8, g5, g9, h6, i5, i6, i8, i9, j6, so you will see folder name like
    h8_g8_g7_f6_i7_h7_e9
    h8_g8_g7_f6_i7_h7_f8
    h8_g8_g7_f6_i7_h7_g5
    h8_g8_g7_f6_i7_h7_g9
    h8_g8_g7_f6_i7_h7_h6
    h8_g8_g7_f6_i7_h7_i5
    h8_g8_g7_f6_i7_h7_i6
    h8_g8_g7_f6_i7_h7_i8
    h8_g8_g7_f6_i7_h7_i9
    h8_g8_g7_f6_i7_h7_j6

    If any folder is solved, it means in board situation h8_g8_g7_f6_i7_h7, the black must win by following
    those solution

    4. The meaning of a folder is solved: All x.txt is solved. The x.txt contains black and white move,
    the first move is black, the last move is white

    There are two case about x.txt is solved
    - x.txt in the folder has a corresponding x.board2action.txt file
    - check the board situation x.txt is solved like we describe in the beginning



    :param board_str: for example h8_g8_g7_f6_i7_h7, the first is black, last is white
    :param divided_folder: all the .txt and .board2action.txt full path name
                    for example
                    [
                    "./divided/h8_g8_g7_f6_i7_h7_e9/h8_g8_g7_f6_i7_h7_e9_c7.txt",
                    "./divided/h8_g8_g7_f6_i7_h7_e9/h8_g8_g7_f6_i7_h7_e9_c8.txt",
                    ]
    :return:

    """
    if board_str in is_solved_cache:
        print("hitting cache %s" % board_str)
        return is_solved_cache[board_str]

    for f in divided_folder:
        if f.endswith("/%s.txt.board2action.txt" % board_str):
            return True
    all_folder_names2txt = {}
    for f in divided_folder:
        folder_name = f.split("/")[2]
        if folder_name in all_folder_names2txt:
            all_folder_names2txt[folder_name].append(f)
        else:
            all_folder_names2txt[folder_name] = [f]

    any_folder_solved = []
    for f in all_folder_names2txt.keys():
        if (len(f) == len(board_str) + 4 or len(f) == len(board_str) + 3) and f.startswith(board_str):
            any_folder_solved.append(f)

    is_solved_flag = False
    for folder in any_folder_solved:
        if is_solved_flag:
            break
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

    is_solved_cache[board_str] = is_solved_flag
    return is_solved_flag


def get_divided_tree():
    folders = []
    top_folder = "../divided"
    for folder in os.listdir(top_folder):
        folder_fullname = "%s/%s/" % (top_folder, folder)
        if os.path.isdir(folder_fullname):
            folders.append(folder_fullname)
    filenames = []
    for f in folders:
        for sub_txt in os.listdir(f):
            full_sub_txt = "%s%s" % (f, sub_txt)
            if full_sub_txt.endswith(".txt"):
                filenames.append(full_sub_txt)
    return filenames


def is_solved_io(board_str):
    filenames = get_divided_tree()
    return is_solved(board_str, filenames)


def compress_and_or_tree(tree):
    key, values = list(tree.keys())[0], list(tree.values())[0]
    if key == "AND":
        if len(values) == 1:
            item = values[0]
            if type(item) is dict:
                item_key, item_values = list(item.keys())[0], list(item.values())[0]
                if item_key == "OR":
                    return compress_and_or_tree(item)

    if key == "OR":
        for item in values:
            if type(item) is dict:
                item_key, item_values = list(item.keys())[0], list(item.values())[0]
                if item_key == "AND" and len(item_values) == 0:
                    # Do compress
                    return {"AND": []}
    else:
        return_empty = True
        for item in values:
            if type(item) is dict:
                item_key, item_values = list(item.keys())[0], list(item.values())[0]
                if item_key != "AND" or len(item_values) > 0:
                    return_empty = False
            else:
                return_empty = False

        if return_empty:
            return {"AND": []}

    return tree


def next_steps_suggestion(board_str, divided_folder):
    """
    two ways to organize the result, it is AND OR tree
    ("OR", list of string or tuple) to represent OR, any suggestion in the list resolve the situation resolved
    ("AND", list of string or tuple) to represent AND, all suggestion in the tuple resolve the sigutaion resolved

    Return optional suggestions:
    1. a list of item, the item is string or a list of string, the suggestion folders or situations
    Any item resolved in the optional suggestions, the board_str resolved

    The related folders status
    1. No explored:
        1) there is no corresponding x.board2action.txt for each x.txt in the folder
        2) there is no any x_{black_step} folder for each x.txt in the folder
    For No explored folder, add it as item in the return suggestions

    2. Fully explored:
        Some sub problem may be fully explored

    3. Partly explored, except 1), 2) it is partly explored
        1) some x.txt has corresponding x.board2action.txt, Ignore them
        2) some x.txt hasn't
            1. no corresponding folder, No explored
            2. has corresponding folder, check as we described in the beginning
             get the suggestions from them and merge them together as an item

    """
    return_suggestion = []
    all_folder_names2txt = {}
    for f in divided_folder:
        folder_name = f.split("/")[2]
        if folder_name in all_folder_names2txt:
            all_folder_names2txt[folder_name].append(f)
        else:
            all_folder_names2txt[folder_name] = [f]
    any_folder_solved = []
    for f in all_folder_names2txt.keys():
        if (len(f) == len(board_str) + 4 or len(f) == len(board_str) + 3) and f.startswith(board_str):
            any_folder_solved.append(f)

    for folder in any_folder_solved:
        filenames = all_folder_names2txt[folder]
        assert (len(filenames) > 0)

        board_situations = []
        solved_situations_solutions = []
        for name in filenames:
            if not name.endswith(".board2action.txt") and not name.endswith(".hitted_record.txt"):
                board_situations.append(name)
            else:
                solved_situations_solutions.append(name)
        if len(solved_situations_solutions) == 0:
            return_suggestion.append(folder)
        else:
            unsolved_suggestion = []
            unsolved_board_situation = []
            for b in board_situations:
                if not ("%s.board2action.txt" % b) in solved_situations_solutions:
                    unsolved_board_situation.append(b)
            for un_s in unsolved_board_situation:
                un_s_str = un_s.split("/")[3].strip(".txt")
                found = False
                for k in all_folder_names2txt.keys():
                    if found:
                        break
                    if k.startswith(un_s_str):
                        found = True
                if not found:
                    unsolved_suggestion.append(un_s)
                else:
                    t = next_steps_suggestion(un_s_str, divided_folder)
                    unsolved_suggestion.append(t)
            return_suggestion.append(compress_and_or_tree({"AND": unsolved_suggestion}))

    return compress_and_or_tree({"OR": return_suggestion})


def next_steps_suggestion_io(board_str):
    return next_steps_suggestion(board_str, get_divided_tree())


def and_or_tree_is_same(a, b):
    if a.keys() != b.keys():
        return False
    res = True
    if len(a.values()) != len(b.values()):
        return False
    for a_item, b_item in zip(a.values(), b.values()):
        if not res:
            break
        if a_item is tuple and b_item is tuple:
            res = and_or_tree_is_same(a_item, b_item)
        elif a_item is str and b_item is str:
            res = a_item == b_item
    return res


def move2position(move):
    """

    :param move:
    :return:
    >>> move2position("g8")
    (6, 7)
    """
    y = int(move[1:]) - 1
    x = ord(move[0]) - ord('a')
    return x, y


def position2move(x, y):
    """

    :param x:
    :param y:
    :return:
    >>> position2move(6, 7)
    'g8'
    """
    return "%s%s" % (chr(x + ord('a')), y + 1)


class bit_board():
    black_mask = 0b01
    white_mask = 0b10

    def __init__(self):
        self.horizontal_ = []
        for i in range(15):
            self.horizontal_.append(0x3FFFFFFF)

    @staticmethod
    def init_from_board_str(board_str):
        b = bit_board()
        horizontal_ints = [int(x) for x in board_str.split(",")]
        for i in range(15):
            b.horizontal_[i] = 0x3FFFFFFF & horizontal_ints[i]
        return b

    def get_move(self, x, y):
        """

        :param x:
        :param y:
        :return:
        >>> b = bit_board()
        >>> b.place_move("a5", b.black_mask)
        >>> b.get_move(0, 4) == b.black_mask
        True
        >>> b.get_move(0, 1) == 3
        True
        """
        return self.horizontal_[y] >> (x * 2) & 3

    def place_move(self, move, mask):
        """
        :param move:
        :return:
        black mask should be 0b01
        white mask should be 0b10
        """
        x, y = move2position(move)
        final_mask = ~((mask ^ 3) << (2*x)) & 0xffffffff

        self.horizontal_[y] &= final_mask

    def __str__(self):
        res = ""
        for h in self.horizontal_:
            res += ",%s" % h
        return res[1:]

    def steps(self, step_str):
        mask = self.black_mask
        for s in step_str.split("_"):
            self.place_move(s, mask)
            if mask == self.black_mask:
                mask = self.white_mask
            else:
                mask = self.black_mask

    def to_step_str(self):
        black_position = []
        white_position = []
        for y in range(15):
            for x in range(15):
                mask = 0b11 << (2 * x)
                fig = (mask & self.horizontal_[y]) >> (2 * x)
                if fig == self.black_mask:
                    black_position.append((x, y))
                elif fig == self.white_mask:
                    white_position.append((x, y))

        steps_strs = []
        for i in range(len(black_position) + len(white_position)):
            if i % 2 == 0:
                x, y = black_position[int(i / 2)]
            else:
                x, y = white_position[int((i - 1)/ 2)]
            steps_strs.append(position2move(x, y))
        return "_".join(steps_strs)


def board_str2steps(board_str):
    """

    :param board_str:
    :return:
    >>> board_str2steps("1073741823,1073741823,1073741823,1073741823,1073741823,1073741823,1073725439,1073709055,1073741823,1073741823,1073741823,1073741823,1073741823,1073741823,1073741823")
    'h8_h7'

    >>> res = board_str2steps("1073741823,1073741823,1073741823,1073733631,1073739711,1073731007,1073719231,1073634047,1073733631,1073740799,1073741823,1073741823,1073741823,1073741823,1073741823")
    >>> normalize_steps_str(res) == normalize_steps_str("h8_h7_g8_i8_g6_g7_f7_e8_e6_d5_g9_f10_f6_d6_f5_f8_g4_d7")
    True
    """
    b = bit_board.init_from_board_str(board_str=board_str)
    return b.to_step_str()


def steps2board_str(step_str):
    """

    :param step_str: h8_g8_g7_f6_i7_h7
    :return: 1073741823,1073741823,1073741823,1073733631,1073739711,1073731007,1073719231,1073634047,1073733631,1073740799,1073741823,1073741823,1073741823,1073741823,1073741823

    >>> steps2board_str("h8_h7")
    '1073741823,1073741823,1073741823,1073741823,1073741823,1073741823,1073725439,1073709055,1073741823,1073741823,1073741823,1073741823,1073741823,1073741823,1073741823'

    >>> steps2board_str("h8_h7_g8_i8_g6_g7_f7_e8_e6_d5_g9_f10_f6_d6_f5_f8_g4_d7")
    '1073741823,1073741823,1073741823,1073733631,1073739711,1073731007,1073719231,1073634047,1073733631,1073740799,1073741823,1073741823,1073741823,1073741823,1073741823'
    """
    b = bit_board()
    b.steps(step_str)
    return str(b)


def read_board2action_filenames():
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
    return board2action_filenames


def symmetry_horizontal(move):
    """

    :param move:
    :return:
    >>> symmetry_horizontal("i7")
    'i9'
    """
    x, y = move2position(move)
    new_y = y - (y - 7) * 2
    return position2move(x, new_y)


def symmetry_vertical(move):
    """

    :param move:
    :return:
    >>> symmetry_vertical("i7")
    'g7'
    """
    x, y = move2position(move)
    new_x = x - (x - 7) * 2
    return position2move(new_x, y)


def symmetry_anti_diagonal(move):
    """

    :param move:
    :return:
    >>> symmetry_anti_diagonal("i7")
    'g9'
    """
    x, y = move2position(move)
    new_x = y
    new_y = x
    return position2move(new_x, new_y)


def apply_transformation(move, trans, reverse=False):
    """

    :param move:
    :param trans: a three tuple [is_symmetry_horizontal, is_symmetry_vertical, is_symmetry_anti_diagonal]

    the element is bool, indicate the symmetry type
    :return:
    >>> apply_transformation("a1", (False, False, False))
    'a1'
    >>> apply_transformation("j9", (True, False, True), reverse=True)
    'i6'
    >>> apply_transformation(apply_transformation("j9", (True, False, True), reverse=True), (True, False, True), False) == "j9"
    True
    >>> apply_transformation("j12", (True, False, True))
    'd10'
    >>> apply_transformation("j12", (True, False, True), reverse=True)
    'l6'
    """
    is_symmetry_horizontal, is_symmetry_vertical, is_symmetry_anti_diagonal = trans

    if not reverse:
        if is_symmetry_horizontal:
            move = symmetry_horizontal(move)
        if is_symmetry_vertical:
            move = symmetry_vertical(move)
        if is_symmetry_anti_diagonal:
            move = symmetry_anti_diagonal(move)
    else:
        if is_symmetry_anti_diagonal:
            move = symmetry_anti_diagonal(move)
        if is_symmetry_vertical:
            move = symmetry_vertical(move)
        if is_symmetry_horizontal:
            move = symmetry_horizontal(move)
    return move


def normalize_steps_str(steps_str):
    """

    :param steps_str:
    :return:
    >>> normalize_steps_str("i7_h7_h8_g8")
    'h8_g8_i7_h7'
    """
    moves = steps_str.split("_")
    black_move = []
    white_move = []
    for i, v in enumerate(moves):
        if i % 2 == 0:
            black_move.append(v)
        else:
            white_move.append(v)
    normalized_black_move = sorted(black_move)
    normalize_white_move = sorted(white_move)
    res_moves = []
    for i in range(len(moves)):
        if i % 2 == 0:
            res_moves.append(normalized_black_move[int(i / 2)])
        else:
            res_moves.append(normalize_white_move[int((i - 1) / 2)])
    return "_".join(res_moves)


def apply_all_transformation_for_steps_str(steps_str):
    """
    :param move:
    :return: all transformation move
    >>> (res, trans) = apply_all_transformation_for_steps_str("h8_h7_i7_g8_g9_f10")
    >>> norm_res = [normalize_steps_str(x) for x in list(res)]
    >>> normalize_steps_str("h8_h7_i7_g8_g9_j6") in norm_res
    True
    """
    moves = steps_str.split("_")
    all_steps_str_trans = ([], [])
    trans = []
    for is_symmetry_horizontal in [True, False]:
        for is_symmetry_vertical in [True, False]:
            for is_symmetry_anti_diagonal in [True, False]:
                trans.append((is_symmetry_horizontal, is_symmetry_vertical, is_symmetry_anti_diagonal))
    for t in trans:
        new_moves = []
        for m in moves:
            new_moves.append(apply_transformation(m, t))
        steps_str = "_".join(new_moves)
        all_steps_str_trans[0].append(steps_str)
        all_steps_str_trans[1].append(t)
    return all_steps_str_trans


def get_all_board2action():
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

    # board_str -> action
    board2action = {}
    # prepared by script prepare_board2action_from_dir.py
    solution_filenames.append("./board2action_from_dir.txt")
    for filename in solution_filenames:
        for line in open(filename, 'r'):
            board_str, action = line.strip("\n").split(":")
            if board_str not in board2action:
                board2action[board_str] = action
    return board2action


"""
ZOBRIST HASH

Hashes from all possible board states (3^225) into uint_64 (2^64).

The algorithm is best described here - https://chessprogramming.wikispaces.com/Zobrist+Hashing
"""

class ZobristHash():
    def __init__(self):
        self.seed = 1000
        # board_size * board_size * 4
        self.random_res = []
        random.seed(self.seed)
        for i in range(15):
            self.random_res.append([])
            for j in range(15):
                self.random_res[-1].append([])
                for z in range(4):
                    self.random_res[-1][-1].append(random.randint(0, 2**64 - 1))

    def hash(self, board_str):
        b = bit_board.init_from_board_str(board_str=board_str)
        hash_res = 0
        for i in range(15):
            for j in range(15):
                for z in range(3):
                    hash_res ^= self.random_res[i][j][b.get_move(i, j)]
        return hash_res

global z
z = ZobristHash()
def board_str_hash(board_str):
    return z.hash(board_str)


def find_next_steps_from_board_str_hash2action(step_str, board_str_hash2action):
    """

    :param step_str: h8_i8
    :param board_str_hash2action: result from get_all_step_str2action
    :return:
    """
    new_format_steps, trans = apply_all_transformation_for_steps_str(step_str)
    possible_moves = []
    for steps, tran in zip(new_format_steps, trans):
        norm_steps = board_str_hash(steps2board_str(steps))
        if norm_steps in board_str_hash2action:
            before_trans = board_str_hash2action[norm_steps]
            next_move = apply_transformation(before_trans, tran, reverse=True)
            possible_moves.append(next_move)
    return possible_moves


def get_all_step_str2action(test=False):
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

    # hash_res -> set(board_str_list)
    hash_res2set = {}

    # hash_res -> action
    hash_rec2action = {}
    # prepared by script prepare_board2action_from_dir.py
    solution_filenames.append("./board2action_from_dir.txt")
    solution_filenames.append("./web_search_cache_file.txt")

    for i, filename in enumerate(solution_filenames):
        for line in open(filename, 'r'):
            board_str, action = line.strip("\n").split(":")
            hash_res = board_str_hash(board_str)
            hash_rec2action[hash_res] = action
            if test:
                if hash_res not in hash_res2set:
                    hash_res2set[hash_res] = set()
                hash_res2set[hash_res].add(board_str)
                if len(hash_res2set[hash_res]) > 1:
                    raise Exception("Found conflict result of key:%s, Existing board str are %s" %\
                                    (hash_res, " and ".join(hash_res2set[hash_res])))
        progress = i / len(solution_filenames)
        print("processing files %0.2f %dth/%d" % (progress, i, len(solution_filenames)))

    # the manually added move has the higher priority
    for line in open("./manually_added_step_str2move.txt", 'r'):
        step_str, action = line.strip("\n").split(":")
        b = bit_board()
        b.steps(step_str)
        board_str = str(b)
        hash_res = board_str_hash(board_str)
        hash_rec2action[hash_res] = action
        if test:
            if hash_res not in hash_res2set:
                hash_res2set[hash_res] = set()
            hash_res2set[hash_res].add(board_str)
            if len(hash_res2set[hash_res]) > 1:
                raise Exception("Found conflict result of key:%s, Existing board str are %s" %\
                                (hash_res, " and ".join(hash_res2set[hash_res])))


    return hash_rec2action


if __name__ == '__main__':
    import doctest
    doctest.testmod()
    board_str_to_resolved = sys.argv[1]
    if not is_solved_io(board_str_to_resolved):
        res = next_steps_suggestion_io(board_str_to_resolved)
        pprint.pprint(res, width=120)
