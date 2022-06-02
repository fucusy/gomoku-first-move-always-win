#include "pn_search.h"
#include "parameters.h"
#include "stdlib.h"
#include "overlap_eval.h"
#include "threat_eval.h"
#include "console_ui.h"
#include "db_search.h"
#include "coords.h"
#include <iostream>
#include <fstream>

using namespace std;


// informs the engine that a new move was placed on board

void pn_search::set_next_move(coords pos) {
    if (last_move_ != coords::INCORRECT_POSITION) {
        board_.place_move(last_move_, current_player_);
        base_selector_.full_update(board_, last_move_);
    }
    board_position_history.push_back(pos);
    last_move_ = pos;
    change_player(current_player_);

    ++move_number_;
}


// engine returns his best response to actual board, doesn't update current state

coords pn_search::get_response() {
#if defined(_DEBUG) || defined(TEST)
    auto start = chrono::high_resolution_clock::now();
#endif

    if (move_number_ == 0) return coords(7, 7);

    // create and initialize new root
    uint8_t root_threat = threat_eval::evaluate(board_.get_lines<5>(last_move_, current_player_)) & 0xC;
    root_ = pn_node(last_move_, pn_type::_OR, 1, 1, root_threat);

    search();

#if defined(_DEBUG) || defined(TEST)
    auto finish = chrono::high_resolution_clock::now();
    debug(finish - start);
#endif

    coords next_move = select_next_move();
    return next_move;
}


// test purpose
bool pn_search::search_winning_test() {
    uint8_t root_threat = threat_eval::evaluate(board_.get_lines<5>(last_move_, current_player_)) & 0xC;
    root_ = pn_node(last_move_, pn_type::_OR, 1, 1, root_threat);

    restart_state();
    select_most_proving();
    //  start dependency-based search and search for a winning move
    if (!(root_.threat() & threat_eval::FOUR_ATTACK)) {
        db_search search(&board_);
        auto max_threat = (root_.threat() & threat_eval::THREE_ATTACK) ? 1 : 2;
        auto winning_move = search.get_winning_move(player_, max_threat);
        if (winning_move) {
            return true;
        }
    }
    return false;
}


void pn_search::search() {
    transposition_table_ = transposition_table();

    restart_state();
    select_most_proving();
    develop_node(root_);
    root_.update_ancestors();
    if (root_.children_size() == 1) {
        restart_state();
        return;
    }

    while (root_.proof() != 0 && root_.disproof() != 0 && root_.subtree_size() < parameters::PN_SEARCH_SIZE_LIMIT_) {
        restart_state();

#if defined(TEST)
        if(root_.subtree_size() % 10000 == 0){
            cout << root_.subtree_size() << " / " << parameters::PN_SEARCH_SIZE_LIMIT_ << endl;
            cout << root_.subtree_size() * 1.0 / parameters::PN_SEARCH_SIZE_LIMIT_ << endl;
        }
#endif
        pn_node &mostProvingNode = select_most_proving();
        develop_node(mostProvingNode);
        mostProvingNode.update_ancestors();
    }
    restart_state();
}


// selects the best answer to the last moves according to the searched tree

coords pn_search::select_next_move() const {
    if (root_.proof() == 0) { // if value == TRUE, select the proving children
        size_t i = 0;
        while (root_.child(i)->proof() != 0) ++i;
        return root_.child(i)->position();
    } else if (root_.disproof() == 0) { // if value == FALSE, select the most developed node
        const pn_node *best_node = root_.child(0);
        for (size_t i = 1; i < root_.children_size(); ++i) {
            if (best_node->subtree_size() < root_.child(i)->subtree_size())
                best_node = root_.child(i);
        }
        return best_node->position();
    } else { // else select the most proving node
        const pn_node *best_node = root_.child(0);
        float best_ratio = float(root_.child(0)->proof()) / float(root_.child(0)->disproof());

        for (size_t i = 1; i < root_.children_size(); ++i) {
            float new_ratio = float(root_.child(i)->proof()) / float(root_.child(i)->disproof());
            if (best_ratio > new_ratio) {
                best_node = root_.child(i);
                best_ratio = new_ratio;
            }
        }
        return best_node->position();
    }
}


// selects the node that will most likely end the search

pn_node &pn_search::select_most_proving() {
    pn_node *selected_node = &root_;
    while (selected_node->children_size() > 0) { // go deeper the DAG until a leaf node is reached
        size_t i = 0;
        switch (selected_node->type()) {
            case pn_type::_OR:
                while (selected_node->child(i)->proof() != selected_node->proof()) ++i;
                break;
            case pn_type::_AND:
                while (selected_node->child(i)->disproof() != selected_node->disproof()) ++i;
                break;
        }
        update_state(selected_node->position());
        selected_node = selected_node->child(i);
    }

    update_state(selected_node->position());
    return *selected_node;
}

coords pn_search::find_from_solved_solution(bit_board b){
    coords position = coords::INCORRECT_POSITION;
    vector<bit_board> all_board;
    vector<vector<bool>> all_trans; // the element is 3 size bool vector,
    b.apply_all_transformation(all_board, all_trans);

    for(int i = 0; i < all_board.size(); i++){
        string board_str = all_board[i].to_string();
        string position_str = "XX";
        rocksdb::Status status = db->Get(rocksdb::ReadOptions(), board_str, &position_str);
        if(!status.IsNotFound()){
            //cout << "board_str:" << board_str << ", move" << position_str << endl;
            coords tmp_position;
            coords::try_parse(position_str, tmp_position);
            position = coords::apply_trans(tmp_position, all_trans[i][0], all_trans[i][1], all_trans[i][2], true);
            return position;
        }
    }
    return position;
}


// adds children to the most-proving node

void pn_search::develop_node(pn_node &node) {
    // if the actual node is just a transposition of found_node, add its children and return
    auto found_node = transposition_table_.update(hash_.actual(), &node);
    if (found_node != nullptr) {
        for (auto &&child : found_node->children())
            node.add_child(child);
        return;
    }


    // search at the beginning
    // for some given board like h8_h7_g7_i8_i9_g6_j10_j9, it won't start the db search for black move,
    // because there is no parent_threat by given board, then it doesn't know black has FOUR_ATTACK

	coords win_move = board_.get_move_to_win(player_);
	if(!win_move.is_out_of_board()){
        switch (node.type()) {
            case pn_type::_OR:
                node.add_child(win_move, pn_type::_AND, 0, UINT32_MAX, UINT8_MAX);
                return;
            case pn_type::_AND:
                node.add_child(win_move, pn_type::_OR, UINT32_MAX, 0, UINT8_MAX);
                return;
        }
	}

    //  start dependency-based search and search for a winning move
    if (!(node.threat() & threat_eval::FOUR_ATTACK) || node.parent_threat() & threat_eval::FOUR_ATTACK) {
        db_search search(&board_);
        auto max_threat = (node.threat() & threat_eval::THREE_ATTACK) ? 1 : 2;
        auto winning_move = search.get_winning_move(player_, max_threat);
        if (winning_move) {
            switch (node.type()) {
                case pn_type::_OR:
                    node.add_child(*winning_move, pn_type::_AND, 0, UINT32_MAX, UINT8_MAX);
                    return;
                case pn_type::_AND:
                    node.add_child(*winning_move, pn_type::_OR, UINT32_MAX, 0, UINT8_MAX);
                    return;
            }
        }
    }

    // else select a few best moves according to selector's static evaluation
    selector_.assign_scores(board_, player_);
    uint32_t init_disproof_proof = 1 + position_history_.size() / 2;
    if (player_ == BLACK) {
        // Search existing solution database
        string board_str = board_.to_string();
        coords position = find_from_solved_solution(board_);
        if(!position.is_out_of_board()){
            string position_str = position.to_string();
            hitted_board2action.insert(board_str + ":" + position_str);
            cout << " the existing solution " << board_str << " " << position_str  << endl;
            node.add_child(position, pn_type::_AND, 0, UINT32_MAX, UINT8_MAX);
                            return;
        }

        /**
         *
         * For BLACK
         * 1. add the children who counter the white component
         * 2. add the 10 most promising children using heurist method
         */
        assert(node.type() == pn_type::_OR);
        set<string> counter_positions = board_.counter_opponent(node.position());


        // below is board_str for steps h8_h7_i7_j6_g9_g10_g8_j9, if it is this steps, add g6 in this children
        vector<string> added_board_strs = {
        "h8_h7_i7_j6_g9_g10_g8_j9",
        "h8_i7_i9_g7_j9_g8_j7_g9_g6_j8_k9_l9_j10_k10_i11_h12_k11_l12_j11_h11",
        "h8_i7_i9_g7_j9_g8_j7_g9_g6_j8_k9_l9_j10_k10_i11_h12_k11_l12_j11_h11_l11_m11",
        "h8_i7_h9_h10_i8_g10_g8_j8_i9_j9_f7_i10_j10_g7",
        "h8_i7_h9_h10_i8_g10_g8_j8_i9_j9_f7_i10_j10_g7_h6_e8_e6_d5_f6_e10_f10_g6_f5_f4", // no need if enable search chong chong, four
        "h8_i7_h9_h10_i8_g10_g8_j8_i9_j9_f7_i10_j10_g7_h6_e10_f10_e8", // no need if enable search chong chong, four
        "h8_i7_h9_h10_i8_g9_g10_j7_f8_g8", // no need if enable search chong chong, four
        "h8_i7_h9_h10_i8_g9_g10_j7_f8_g8_f11_e12_g7_e9_f6_e5_f9_f7_h11_i12_j9_k8", // no need if enable search chong chong, four
        "h8_i7_h9_h10_i8_g9_g10_j7_f8_g8_f11_e12_g7_e9_f6_e5_f9_f7_h11_i12_j9_f12_h7_g6", // tricky one
        "h8_i7_h9_h10_i8_g9_g10_j7_f8_g8_f11_e12_g7_e9_f6_e5_f9_f7_h11_i12_j9_f12_h7_g6_l8_g12_h12_j10", // tricky one
        "h8_i7_h9_h10_i8_g9_g10_j7_f8_g8_f11_e12_g7_e9_f6_e5_f9_f7_h11_i12_j9_f12_h7_g6_l8_g12_h12_d12_c12_j10", // tricky one
        "h8_i7_h9_h10_i8_g9_g10_j7_f8_g8_f11_e12_g7_e9_f6_e5_f9_f7_h11_i12_j9_f12_h7_g6_l8_g12_h12_k9_j8_k8", // tricky one, we need it even if enable search chong chong, four
        "h8_i7_h9_h10_i8_g9_g10_j7_f8_g8_f11_e12_g7_e9_f6_e5_f9_f7_h11_i12_j9_f12_h7_g6_l8_g12_h12_d12_c12_k9", // tricky one, we need it even if enable search chong chong, four
        "h8_i7_h9_h10_i8_g9_g10_j7_f8_g8_f11_e12_g7_e9_f6_e5_f9_f7_h11_i12_j9_i10_h7_g6", // tricky one, we need it even if enable search chong chong, four
        "h8_i7_h9_h10_i8_g9_g10_j7_f8_g8_f11_e12_g7_e9_f6_e5_f9_f7_h11_i12_j9_g12_h7_g6", // tricky one, we need it even if enable search chong chong, four
        "h8_i7_h9_h10_i8_g9_g10_j7_f8_g8_f11_e12_g7_e9_f6_e5_f9_f7_h11_i12_j9_g12_h7_g6_f12_f10", // tricky one, we need it even if enable search chong chong, four
        "h8_i7_h9_h10_i8_g9_g10_j7_f8_g8_f11_e12_g7_e9_f6_e5_f9_f7_h11_i12_j9_l11_g11_i11", // tricky one, we need it even if enable search chong chong, four
        "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_g9_h11_e8_f12_f10_i11_g11_i12_j13_h12_i9_g12_j12", // tricky, the black should encoutner this
        "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_e8_i9_e9", // tricky one, we need it even if enable search chong chong, four
        "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_k7_h7_i9_j8_h11_e9_l7_m7_f8", // tricky one, we need it even if enable search chong chong, four
        "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_k7_h7_i9_j8_h11_e9_f7", // special case
        "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_g7_f7_e8_f10_f8_g9_e9_e11_d12", // we should have d10 to counter white
        "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_g7_f7_e8_f10_f8_g9_c8_d8_e9_e11_d12", // // we should have d10 to counter white
        "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_g7_f7_g9_e8_g6_g5_d7_k8_j8",
        "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_g7_f7_g9_e8_g6_g5_d7_k8_l8_f8_f10", // tricky one
        "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_g7_f7_g9_e8_d7_k8_j8_g6_d9",
        "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_g7_f7_g9_e8_d7_k8_l8_f8_f10",
        "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_g7_f7_g9_e8_d7_k8_l8_f8_f10_i12_h11",
        "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_g7_f7_g9_e8_d7_k8_l8_f8_f10_i12_h11_h13_g14_g12_e10",
        "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_g7_f7_g9_e8_d7_k8_l8_f8_f10_i12_h11_h13_k10_g12_e10_i14_j15",
        "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_g7_f7_g9_e8_d7_k8_l8_f8_f10_i12_h11_h13_k10_g12_g6_g5_e10_i14_j15",
        "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_g7_f7_g9_e8_d7_k8_l8_f8_f10_i12_h11_h13_k10_g12_e10_i14_j15_i13_g6_g5_i11_j13_g13", //tricy
        "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_g7_f7_g9_e8_d7_k8_l8_f8_f10_i12_h11_h13_k10_g12_e10_i14_j15_i13_i11_j13_g13",
        "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_e8_g7_e9_i9_j10_h11_e11_e10_j9_j11_i10_k10_h13",
        "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_e8_g7_e9_i9_e11_e10_j10", //tricky, we should have it
        "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_e8_g7_e9_i9_e11_e10_j10_j9_g9",
        "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_g9_f8_f7_g7_e9",
        "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_g9_f8_g7_f7_f10_e8_d7_g6_d9",
        "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_g9_f8_f10",
         "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_g9_f8_f10_g7_h6_i9_j10_e8_d7",
         "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_g9_f8_f10_g7_h6_i9_j10_e8_d7_d9_c10",
         "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_g9_f8_f10_g7_e9_f6_e5_f7_f5_e8_d7",
         "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_g9_f8_f10_g7_e9_f6_e5_f7_f5_e8_d7_i11_i12",
         "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_g9_f8_f10_g7_e9_f6_e5_f7_f5_e8_d7_i11_h11",
         "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_g9_f8_f10_g7_f6_e8_d7", // not sure if necessary
          "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_h11_e9_g9_f8_g7_f7_f10_d9_c10",
          "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_h11_e9_g9_f8_g7_f7_f10_d9_g6_g5_c8",
          "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_h11_e9_g9_f8_g7_f7_f10_d9_g6_g5_c8_d10_d11",
          "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_h11_e9_g9_f8_f10_g7_h6_f6_e5",
          "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_h11_e9_g9_f8_f10_g7_h6_f6_i9",
          "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_h11_e9_g7_f7_g9_e8_g6_g5_e7_f8_f10",

          "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_h11_e9_g7_f7_g9_e8_e7_f8_f10",
          "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_g9_f8_g7_f7_f10_e8_g6_g5_d7",
          "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_g9_f8_g7_f7_f10_e8_g6_g5_d7_k8_j8_k9_k10",
          "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_g9_f8_g7_f7_f10_e8_g6_g5_d7_k8_j8_k9_k10_j9_i10",
          "h8_i7_h9_h10_i8_g8_g10_j7_f11_e12_f9_h7_k7_g9_f8_g7_f7_f10_e8_g6_g5_d7_k8_l8",
           "h8_i7_h9_h10_i8_j8",
              };
        vector<string> added_search_actions = {
        "g6", "l11", "j13", "h6", "g5", "e6", "f11", "j11", "l8",
        "l8", "k10", "j10", "j11", "l8", "f12", "l11", "g13", "j8", "i11",
        "e7", "d7", "d10", "d10", "i5", "h13","j9", "i12", "h13", "i13", "i13", "i13", "k12", "k12", "l8", "j9", "h11"
        ,"k9","l8","g7","d9","e7","i11", "l8", "l8", "k9","d7", "d10", "e7", "d7", "d7", "c8", "c8", "k8","j9","j6","h13",
        "h6",
        };

        for(int i = 0; i < added_board_strs.size(); i++){
            bit_board added_board;
            added_board.set_steps_str(added_board_strs[i]);
            string board_str_patch = added_board.to_string();
            string patch_position_str = added_search_actions[i];
            if(board_.to_string() == board_str_patch){
                counter_positions.insert(patch_position_str);
            }
        }


        for (auto p: counter_positions) {
            coords counter_p;
            coords::try_parse(p, counter_p);
            auto lines = board_.get_lines<5>(counter_p, figure::BLACK);
            uint8_t threats = threat_eval::evaluate(lines);
            node.add_child(counter_p, pn_type::_AND, init_disproof_proof, init_disproof_proof, threats);
        }


        auto selected_score = 5;
        auto avg_move_steps = (board_position_history.size() + position_history_.size()) / 2;
        auto min_score = avg_move_steps >= 11 ? 3 : 0;
        int selected_item = 0;

        // set it to 15, allow g9 is in the possible black move, which is from the
        // Search for Solution book page 152,
        int black_limit_promising_item = 15;
        while (selected_score >= min_score && selected_item < black_limit_promising_item) {
            auto move = selector_.get_first();
            while (move.get_offset() != board_list_item::NULL_OFFSET) { // loop over all possible moves
                uint16_t score = move.score[player_ - 1];
                if (score == UINT16_MAX) { // if the move is proving, add it and return
                    switch (node.type()) {
                        case pn_type::_OR:
                            node.add_child(move.position, pn_type::_AND, 0, UINT32_MAX, UINT8_MAX);
                            return;
                        case pn_type::_AND:
                            node.add_child(move.position, pn_type::_OR, UINT32_MAX, 0, UINT8_MAX);
                            return;
                    }
                }
                if (score == selected_score) {
                    node.add_child(move.position, node.type() == pn_type::_OR ? pn_type::_AND : pn_type::_OR,
                                   init_disproof_proof, init_disproof_proof,
                                   move.threats[player_ - 1] & 0xC);
                    selected_item++;
                }
                move = selector_.get_item(move.previous);
            }
            selected_score--;
        }
    } else {
        auto move = selector_.get_first();
        while (move.get_offset() != board_list_item::NULL_OFFSET) { // loop over all possible moves
            uint16_t score = move.score[player_ - 1];
            // select all white
            node.add_child(
                    move.position,
                    node.type() == pn_type::_OR ? pn_type::_AND : pn_type::_OR,
                    init_disproof_proof,
                    init_disproof_proof,
                    move.threats[player_ - 1] & 0xC);
            move = selector_.get_item(move.previous);
        }
    }
    string player_name = "black";
    if (player_ == WHITE) {
        player_name = "white";
    }

    #if defined(TEST)
    if (rand() % 10000 == 1) {
        auto avg_move_steps = (board_position_history.size() + position_history_.size()) / 2;
        cout << "avg move steps for black/white " << avg_move_steps << endl;
        cout << "child node size for " << player_name << " is " << node.children_size() << endl;
        cout << "selector size for " << player_name << " is " << selector_.size() << endl;
    }
    #endif

#if defined(DIVIDE)
    ofstream black_steps;
    black_steps.open ("black_steps.txt");
    for (int i = 0; i < node.children_size(); i++) {
        black_steps << node.children()[i]->position().to_string() << endl;
    }
    black_steps.close();
    exit(0);
#endif
}


// updates actual state of the board

void pn_search::update_state(coords position) {
    board_.place_move(position, player_);
    position_history_.push(position);
    hash_.update(position, player_ - 1);
    selector_.shallow_update(board_, position);

    change_player(player_);
}


// restarts state of the board to the initial root value

void pn_search::restart_state() {
    while (!position_history_.empty()) {
        board_.delete_move(position_history_.top());
        position_history_.pop();
    }

    selector_ = base_selector_;
    player_ = current_player_;
    hash_.restart();
}


// debug-only function, it creates a command line subprogram that traverses the created search tree


/**
string board2string(simple_board board) {
    bit_board bb = bit_board();
    for (int y = 0; y < constants::BOARD_SIZE; ++y){
        for (int x = 0; x < constants::BOARD_SIZE; ++x) {
            auto position = coords(x, y);
            bb.place_move(position, board.get_move(position));
        }
    }
    return bb.to_string();
}
 **/

/**
gomoku::pb_pn_node to_pb_pn_node(const pn_node root) {
    gomoku::pb_pn_node n;
    n.set_col(root.position().x);
    //cout << "n position x: " << n.col() << endl;
    n.set_row(root.position().y);
    n.set_proof(root.proof());
    n.set_disproof(root.disproof());
    n.set_threat(root.threat());
    n.set_size(root.children_size());

    for (int i = 0; i < root.children_size(); i++) {
        n.add_children();
    }
    for (int i = 0; i < root.children_size(); i++) {
        gomoku::pb_pn_node child_node = to_pb_pn_node(*root.child(i));
        n.mutable_children(i)->CopyFrom(child_node);
    }
    return n;
}
**/

void pn_search::save_tree(std::string output_filename, bit_board board_track, const pn_node *actual_root) {
    vector<tuple<const pn_node *, int>> tree_loop_node_stack;
    vector<const pn_node *> node_path;
    // board_track.delete_move(actual->position());
    int last_pop_layer = -1;
    tree_loop_node_stack.push_back(make_tuple(actual_root, 0));
    ofstream myfile;
    myfile.open(output_filename);
    while (tree_loop_node_stack.size() > 0) {
        auto selected = tree_loop_node_stack.back();
        const pn_node *selected_node = get<0>(selected);
        int layer = get<1>(selected);
        tree_loop_node_stack.pop_back();

        // decide the need to clear the node_path
        int pop_count = (last_pop_layer - layer + 1) * 2;
        //cout << "pop count " << pop_count << endl;
        for (int i = 0; i < pop_count; i++) {
            const pn_node *a = node_path.back();
            node_path.pop_back();
            board_track.delete_move(a->position());
        }

        // output board, black position
        pn_node *best_black = new pn_node();
        for (auto &&child : selected_node->children()) {
            if (child->proof() == 0) { best_black = child.get(); }
        }
        board_track.place_move(selected_node->position(), figure::WHITE);
        myfile << board_track.to_string() << ":" << best_black->position().to_string() << endl;

        //auto ui =  new console_ui();
        //ui->render(board_track.debug_board_);
        board_track.place_move(best_black->position(), figure::BLACK);

        //int test;
        //cin >> test;
        // step with given node
        node_path.push_back(selected_node);
        node_path.push_back(best_black);
        last_pop_layer = layer;

        // expand
        for (auto &&child : best_black->children()) {
            tree_loop_node_stack.push_back(make_tuple(child.get(), layer + 1));
        }
    }
    myfile.close();
}

#if defined(_DEBUG) || defined(TEST)
void pn_search::debug(chrono::duration<double, milli> duration) const {
    using namespace std;

    vector<const pn_node*> node_stack;

    const pn_node* actual = &root_;
    node_stack.push_back(actual);

    //new way to serialize content
    string output_prefix = "";
    for(auto pos: board_position_history){
        output_prefix += pos.to_string();
    }
    string output_name = output_prefix + ".pbcache";
//    gomoku::pb_pn_node pd_root = to_pb_pn_node(root_);
//    fstream output(output_name, ios::out | ios::trunc | ios::binary);
//    if (!pd_root.SerializeToOstream(&output)) {
//        cerr << "Failed to write address book." << endl;
//    }

    cout << endl << endl << "elapsed time: " << duration.count() << " ms";

    while (true) {
        cout << endl << endl << "path to actual: ";
        for (auto&& node : node_stack) cout << node->position().to_string() << " / ";
        cout << endl << endl;

        cout << "actual: proof = " << actual->proof() << ", disproof = " << actual->disproof() << ", threat = " << int(actual->threat()) << ", size = " << actual->subtree_size() << endl;
        auto best_position = coords();
        for (auto&& child : actual->children()){
            cout << "\t" << child->position().to_string() << ": proof = " << child->proof() << ", disproof = " << child->disproof() << ", threat = " << int(child->threat()) << ", size = " << child->subtree_size() << endl;
            if(child->proof() == 0){ best_position = child->position(); }
        }

#if defined(NOINTERACTIVE)
        auto ui =  new console_ui();
        ui->render(board_.debug_board_);
        string output_name = init_filename + ".board2action.txt";
        if(actual->proof() == 0){
            save_tree(output_name, board_, actual);
            cout << "saved solution to " << output_name << " exit the program" << endl;

            // save the hitted board2action, so that we can find later on move from another solution files
            if(hitted_board2action.size() > 0){
                cout << "saviing the hitted record" << endl;
                string incomplete_filename = init_filename + ".hitted_record.txt";
                ofstream myfile;
                myfile.open(incomplete_filename);
                for(auto record : hitted_board2action){
                    myfile << record << endl;
                }
                myfile.close();
            }            

        }else{
            cout << "no save solution to " << output_name << " , because proof is not 0, black doesn't always win" << endl;
        }

        exit(0);
#endif

        while(true) {
            string command;
            cout << endl << "command: ";
            cin >> command;

            coords position;
            if(command == "debug_board"){
                auto ui =  new console_ui();
                ui->render(board_.debug_board_);
            }else if(command == "log"){
                cout << board_.to_string() << "," << best_position.to_string() << endl;
            }else if(command == "save_tree"){
                string output_name = init_filename + ".board2action.txt";
                save_tree(output_name, board_, actual);
            }
            else if(command == "x" || command == "exit") {
                return;
            }
            else if(command == "..") {
                node_stack.pop_back();
                actual = node_stack.back();
                break;
            }
            else if(coords::try_parse(command, position)) {
                bool found = false;
                for (auto&& child : actual->children()) {
                    if(child->position() == position) {
                        actual = child.get();
                        node_stack.push_back(actual);
                        found = true;
                        break;
                    }
                }
                if (found) break;
            }
        }
    }
}
#endif
