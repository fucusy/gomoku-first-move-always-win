from divided_solution_manager import get_all_step_str2action_tuple, board_str2steps, normalize_steps_str

if __name__ == "__main__":
    limit = 50000000
    i = 0
    path = "/Users/qiang/Documents/github/gomoku-move-database-compressor/moves.full.txt"
    with open(path, 'w') as file:
        for key, value in get_all_step_str2action_tuple():
            norm_step_seq = normalize_steps_str(board_str2steps(key))            
            file.write(f"{norm_step_seq}:{value}\n")
            i += 1
            if i > limit:
                break
