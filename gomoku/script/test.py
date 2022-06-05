from __future__ import print_function
from divided_solution_manager import is_solved, next_steps_suggestion, and_or_tree_is_same, steps2board_str
from divided_solution_manager import get_all_step_str2action


from sys import getsizeof, stderr
from itertools import chain
from collections import deque
try:
    from reprlib import repr
except ImportError:
    pass

# https://code.activestate.com/recipes/577504/
def total_size(o, handlers={}, verbose=False):
    """ Returns the approximate memory footprint an object and all of its contents.

    Automatically finds the contents of the following builtin containers and
    their subclasses:  tuple, list, deque, dict, set and frozenset.
    To search other containers, add handlers to iterate over their contents:

        handlers = {SomeContainerClass: iter,
                    OtherContainerClass: OtherContainerClass.get_elements}

    """
    dict_handler = lambda d: chain.from_iterable(d.items())
    all_handlers = {tuple: iter,
                    list: iter,
                    deque: iter,
                    dict: dict_handler,
                    set: iter,
                    frozenset: iter,
                   }
    all_handlers.update(handlers)     # user handlers take precedence
    seen = set()                      # track which object id's have already been seen
    default_size = getsizeof(0)       # estimate sizeof object without __sizeof__

    def sizeof(o):
        if id(o) in seen:       # do not double count the same object
            return 0
        seen.add(id(o))
        s = getsizeof(o, default_size)

        if verbose:
            print(s, type(o), repr(o), file=stderr)

        for typ, handler in all_handlers.items():
            if isinstance(o, typ):
                s += sum(map(sizeof, handler(o)))
                break
        return s

    return sizeof(o)



if __name__ == '__main__':
    res = get_all_step_str2action()
    print("Hash bytes for %d keys : %d" % (len(res), total_size(res)))

    board_str = "h8_g8_g7_f6_i7_h7"
    divided_folder = [
        "./divided/h8_g8_g7_f6_i7_h7_e9/h8_g8_g7_f6_i7_h7_e9_c7.txt",
        "./divided/h8_g8_g7_f6_i7_h7_e9/h8_g8_g7_f6_i7_h7_e9_c7.txt.board2action.txt",
        "./divided/h8_g8_g7_f6_i7_h7_e9/h8_g8_g7_f6_i7_h7_e9_c8.txt",
        "./divided/h8_g8_g7_f6_i7_h7_e9/h8_g8_g7_f6_i7_h7_e9_c8.txt.board2action.txt"
    ]
    assert (is_solved(board_str, divided_folder))

    next_steps_suggestion_board_str = "h8_g8_g7_f6_i7_h7"
    next_steps_suggestion_divided_folder = [
        "./divided/h8_g8_g7_f6_i7_h7_e9/h8_g8_g7_f6_i7_h7_e9_c7.txt",
        "./divided/h8_g8_g7_f6_i7_h7_e9/h8_g8_g7_f6_i7_h7_e9_c7.txt.board2action.txt",
        "./divided/h8_g8_g7_f6_i7_h7_e9/h8_g8_g7_f6_i7_h7_e9_c8.txt",
        "./divided/h8_g8_g7_f6_i7_h7_e8/h8_g8_g7_f6_i7_h7_e8_c8.txt",
    ]
    expected_suggestion = {"OR":
        [
            "./divided/h8_g8_g7_f6_i7_h7_e8/",
            "./divided/h8_g8_g7_f6_i7_h7_e9/h8_g8_g7_f6_i7_h7_e9_c8.txt"
        ]}

    return_suggestion = next_steps_suggestion(next_steps_suggestion_board_str, next_steps_suggestion_divided_folder)
    and_or_tree_is_same(return_suggestion, expected_suggestion)

