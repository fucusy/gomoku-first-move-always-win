from divided_solution_manager import is_solved, next_steps_suggestion, and_or_tree_is_same, steps2board_str
from divided_solution_manager import get_all_step_str2action


if __name__ == '__main__':
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

