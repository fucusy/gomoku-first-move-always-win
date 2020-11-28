#include <iostream>
#include <vector>
#include <iostream>
#include <string>
#include <algorithm>
#include "bit_board.h"
#include "game.h"
#include "helper.h"
#include "threat_eval.h"
#include "console_ui.h"

using namespace std;


string parse_parameter(char *url, string parameter_name) {
    // url is like "stepsString=_h8_g8_g7_f6_i7_h7_j6_g9&color=BLACK&level=HIGH&randomBegin=true"
    string url_s = string(url);
    auto parameter_position = url_s.find(parameter_name);
    if (parameter_position == string::npos) {
        return "";
    } else {
        auto start_position = parameter_name.size() + 1; // +1, because the = after the parameter name
        auto sub_url = url_s.substr(parameter_position);
        auto and_position = sub_url.find("&");
        if (and_position == string::npos) {
            return sub_url.substr(start_position);
        } else {
            auto value_length = and_position - start_position;
            return sub_url.substr(start_position, value_length);
        }
    }
}

coords next_step(bool player_starts, vector<string> history) {
    game g(player_starts); // create a game
    g.init(history);
    auto next_move = g.engine_->get_response();
    return next_move;
}

bit_board step_str2bit_board(string step_str) {
    bit_board board;
    board.set_steps_str(step_str);
    return board;
}

string step_str2bit_board_str(string step_str) {
    bit_board board = step_str2bit_board(step_str);
    return board.to_string();
}

inline bool filename_exists(const std::string &name) {
    ifstream f(name.c_str());
    return f.good();
}

#if defined(UNITTEST)

void test_winning_sequences(string win_steps_str){
    vector<string> win_steps;
    tokenize(win_steps_str, '_', win_steps);
    bool player_starts = false;
    game g(player_starts); // create a game
    g.init(win_steps);
    if(g.engine_->search_winning_test()){
        cout << "TEST PASS for winning sequence search, board is " << win_steps_str << endl;
    }else{
        cout << "TEST FAIL for winning sequence search, board is " << win_steps_str << endl;
    }
}

void test_move_to_win(std::string steps_str, vector<std::string> win_steps){
    vector<string> steps;
    tokenize(steps_str, '_', steps);

    bool success = false;

    bit_board b = step_str2bit_board(steps_str);
    coords move = b.get_move_to_win(figure::BLACK);

    for(auto s : win_steps){
        if(s == move.to_string()){
            success = true;
        }
    }

    if(success){
        cout << "TEST PASS move_to_win for input " << steps_str << endl;
    }else{
        cout << "TEST FAIL move_to_win for input " << steps_str << endl;
    }

}

void test_counter_opponent(std::string steps_str, std::string encounter_str) {
    vector<string> counter_steps;
    vector<string> res_steps;
    vector<coords> res_coords;
    tokenize(steps_str, '_', counter_steps);
    tokenize(encounter_str, '_', res_steps);
    bool success = true;

    bit_board counter_b = step_str2bit_board(steps_str);
    coords counter_last_move = coords::INCORRECT_POSITION;
    coords::try_parse(counter_steps[counter_steps.size()-1], counter_last_move);
    auto moves = counter_b.counter_opponent(counter_last_move);

    for(auto m : moves){
        if(encounter_str.find(m) == std::string::npos){
            cout << "TEST failed for counter_opponent " << m << " is not from " << encounter_str << endl;
            success = false;
        }
    }

    if(moves.size() != res_steps.size()) {
        cout << "TEST failed for counter_opponent found solution number is " << moves.size() <<
        " it should be " << res_steps.size() << endl;
        cout << "TEST failed the correct should be " <<  encounter_str << endl;
        cout << "But the function returns ";
        for(auto m : moves){
            cout << m << " _ ";
        }
        cout << endl;
        success = false;
    }
    if(success){
        cout << "TEST PASS for input " << steps_str << " output " << encounter_str << endl;
    }
}


int main(int argc, char** argv) { // the optional console arguments is a path to a logged game file for debug purposes
    vector <string> history;
    auto first_step = next_step(false, history);
    if (first_step.to_string() == "h8") {
        cout << "TEST PASS about game AI first step" << endl;
    } else {
        cout << "First step should be h8, but it is " << first_step.to_string() << endl;
    }

    string url = "stepsString=_h8_g8_g7_f6_i7_h7_j6_g9&color=BLACK&level=HIGH&randomBegin=true";
    char char_url[url.size() + 1];
    strcpy(char_url, url.c_str());
    string value = parse_parameter(char_url, "color");
    if(value == "BLACK"){
        cout << "TEST PASS about parse_parameter function" << endl;
    }else{
        cout << "value should be BLACK, but it is " << value << endl;
    }

    std::string steps_str = "h8_g8_g7_f6_i7_h7_j6";
    vector<string> steps;
    tokenize(steps_str, '_', steps);
    bit_board b = step_str2bit_board(steps_str);
    coords last_move = coords::INCORRECT_POSITION;
    coords::try_parse(steps[steps.size()-1], last_move);
    uint16_t score = b.black_evaluation(last_move);
    if(score == 5){
        cout << "TEST PASS about black evaluation 5, two open two" << endl;
    }else{
        cout << "TEST FAIL about black evaluation 5, two open two, It should be 5, but it's " << score << endl;
    }

    // encouter component test
    test_counter_opponent("h8_g8_h7_g9_h1_g10", "g6_g7_g11_g12");
    test_counter_opponent("h8_g8_h7_f8_h1_e8", "d8_c8");
    test_counter_opponent("h8_g8_h7_f7_h1_e6", "h9_i10_d5_c4");
    test_counter_opponent("h8_g8_h7_f9_h1_e10", "d11_c12");
    test_counter_opponent("h8_h7_i7_j6_g9_i8_g8_g6_j9_f5_e4_h6_f6_l5_h9_k6_i6_m4_j7_n3", "o2");


    test_move_to_win("h8_h7_g7_i8_i9_g6_j10_j9", {"k11", "f6"});


    // board, wining sequence search test
    test_winning_sequences("h8_h7_g8_i8_g6_g7_f7_e8_e6_d5_g9_f10_f6_d6_f5_f8_g4_d7");

    // test coords transformation

    coords c;
    string step_str = "i7";
    coords::try_parse(step_str, c);
    if(c.symmetry_horizontal().to_string() == "i9"){
        cout << "TEST PASS symmetry_horizontal" << endl;
    }else{
        cout << "TEST FAIL symmetry_horizontal" << endl;
    }

    if(c.symmetry_vertical().to_string() == "g7"){
        cout << "TEST PASS symmetry_vertical" << endl;
    }else{
        cout << "TEST FAIL symmetry_vertical" << endl;
    }

    if(c.symmetry_anti_diagonal().to_string() == "g9"){
        cout << "TEST PASS symmetry_anti_diagonal" << endl;
    }else{
        cout << "TEST FAIL symmetry_anti_diagonal" << endl;
    }

    coords res = coords::apply_trans(c, false, false, true);
    if(res.to_string() == "g9"){
        cout << "TEST PASS for apply trans" << endl;
    }else{
        cout << "TEST FAIL for apply trans" << endl;
    }


    // test board transformation
    bit_board source_board = step_str2bit_board("h8_h7_i7_g8_g9_f10");
    vector<bit_board> all_board;
    vector<vector<bool>> all_trans; // the element is 3 size bool vector,
    source_board.apply_all_transformation(all_board, all_trans);

    bit_board target_board = step_str2bit_board("h8_h7_i7_g8_g9_j6");
    bool found = false;
    vector<bool> found_tran;
    for(int i = 0; i < all_board.size(); i++){
        if(target_board.to_string() == all_board[i].to_string() && target_board.to_string() != source_board.to_string()){
            found = true;
            found_tran = all_trans[i];
        }
    }
    if(found){
        cout << "TEST PASS about board transformation, "
                << " is_symmetry_horizontal=" << found_tran[0]
                << " is_symmetry_vertical=" << found_tran[1]
                << " is_symmetry_anti_diagonal=" << found_tran[2] << endl;
    }else{
        cout << "TEST FAIL about board transformation" << endl;
    }


    // step str to board str examples
    cout << "step_str: " << "h8_h7_g8_i8_g6_g7_f7_e8_e6_d5_g9_f10_f6_d6_f5_f8_g4_d7" << " board_str: " << step_str2bit_board_str("h8_h7_g8_i8_g6_g7_f7_e8_e6_d5_g9_f10_f6_d6_f5_f8_g4_d7") << endl;
    cout << "step_str: " << "h8_h7" << " board_str: " << step_str2bit_board_str("h8_h7") << endl;


    coords init(3, 9);
    coords trans_res = coords::apply_trans(init, true, false, true);
    coords trans_res_back = coords::apply_trans(trans_res, true, false, true, true);
    if(trans_res_back == init){
        cout << "TEST PASS for transporattion reverse" << endl;
    }else{
        cout << "TEST FAIL for transporattion reverse" << endl;
    }

    exit(0);

    // find broken solutions
    		// init the solved_boardstr2action
    namespace fs = std::filesystem;
    string path = "./divided/";
    for (const auto & entry : fs::directory_iterator(path)){
        if(!fs::is_directory(entry)){
            continue;
        }
        for (const auto & filename : fs::directory_iterator(entry.path())){
            if(string(filename.path()).find(".board2action.txt") != string::npos){
                string line;
                ifstream myfile(string(filename.path()));
                bool is_broken = false;
                if (myfile.is_open()) {
                    while (getline(myfile, line)) {
                        if(is_broken) break;
                        vector<string> key_value;
                        tokenize(line, ':', key_value);

                        std::string board_str = key_value[0];
                        std::string black_move_str = key_value[1];
                        coords black_move;
                        coords::try_parse(black_move_str, black_move);

                        bit_board b = bit_board();
                        b.set_board_str(board_str);
                        b.place_move(black_move, figure::BLACK);

                        bool found_white_four_attack = false;
                        bool found_black_win = false;
                        for(int x = 0; x < 15; x++){
                            for(int y = 0; y < 15; y++) {
                                coords move = coords(x, y);
                                if(b.get_move(move) == figure::WHITE){
                                    auto res = threat_eval::evaluate(b.get_lines<5>(coords(x, y), figure::WHITE));
                                    if(res & threat_eval::FOUR_ATTACK){
                                        found_white_four_attack = true;
                                    }
                                }
                                if(b.get_move(move) == figure::BLACK){
                                    auto res = threat_eval::evaluate(b.get_lines<5>(coords(x, y), figure::BLACK));
                                    if(res == UINT8_MAX){
                                        found_black_win = true;
                                    }
                                }

                            }
                        }
                        if(found_white_four_attack && !found_black_win){
                            auto ui =  new console_ui();
                            ui->render(b.debug_board_);
                            is_broken = true;
                        }

                    }
                    myfile.close();
                }
                if(is_broken){
                    std::cout << "Found broken solutions " << string(filename.path()) << endl;
                }else{
                    std::cout << "GOOD         solutions " << string(filename.path()) << endl;
                }
            }
        }
    }
}
#endif


#if defined(WEB)
int main(int argc, char** argv) { // the optional console arguments is a path to a logged game file for debug purposes
    string steps_str = argv[1]; // h8_h7
    vector<string> steps;
    if(steps_str.size() > 0){
        tokenize(steps_str, '_', steps);
    }
    bool player_starts = false; // ai starts the first game

    // hit the cache
    coords next_move;
    game g(player_starts); // create a game
    g.init(steps);
    next_move = g.engine_->get_response();

    cout << "{\"x\":" << std::to_string(next_move.x) <<",\"y\":" << std::to_string(next_move.y) << "}" << endl;
}
#endif