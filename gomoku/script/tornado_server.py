import os
import platform
import sys

import leveldb
import tornado.ioloop
from tornado import options
from tornado import web
from tornado.log import app_log

from divided_solution_manager import find_next_steps_from_db
from gomoku_llm.train_transformer import predict_next_token
import json

options.options["log_file_prefix"] = "tornado_log"
options.parse_command_line()

db = leveldb.LevelDB('/Users/qiang/Documents/github/gomoku-first-move-always-win/gomoku/script/leveldb_web.db')
BOARD_SIZE = 15


def token_to_xy(token: str):
    return ord(token[0]) - ord("a"), int(token[1:]) - 1


def line_length(pos, direction, stones):
    """Count contiguous stones including pos along a direction."""
    x, y = pos
    dx, dy = direction
    length = 1
    for step in range(1, 5):
        nx, ny = x + dx * step, y + dy * step
        if 0 <= nx < BOARD_SIZE and 0 <= ny < BOARD_SIZE and (nx, ny) in stones:
            length += 1
        else:
            break
    for step in range(1, 5):
        nx, ny = x - dx * step, y - dy * step
        if 0 <= nx < BOARD_SIZE and 0 <= ny < BOARD_SIZE and (nx, ny) in stones:
            length += 1
        else:
            break
    return length


def score_line(length: int) -> int:
    if length >= 5:
        return 1000
    if length == 4:
        return 50
    if length == 3:
        return 3
    if length == 2:
        return 1
    return 0


def score_position(pos, black_positions, white_positions) -> float:
    directions = [(1, 0), (0, 1), (1, 1), (1, -1)]
    score = 0

    for direction in directions:
        score += score_line(line_length(pos, direction, black_positions))

    for direction in directions:
        white_len = line_length(pos, direction, white_positions)
        if white_len >= 5:
            score += 500
        elif white_len == 4:
            score += 40

    center = (BOARD_SIZE - 1) / 2
    dist_from_center = abs(pos[0] - center) + abs(pos[1] - center)
    score += max(0, 5 - dist_from_center * 0.5)
    return score


def best_heuristic_black_move(token_list):
    occupied = {token_to_xy(t) for t in token_list}
    black_positions = {token_to_xy(t) for idx, t in enumerate(token_list) if idx % 2 == 0}
    white_positions = {token_to_xy(t) for idx, t in enumerate(token_list) if idx % 2 == 1}
    empties = [(x, y) for x in range(BOARD_SIZE) for y in range(BOARD_SIZE) if (x, y) not in occupied]
    if not empties:
        return 0, 0

    best_move = None
    best_score = float("-inf")
    for pos in empties:
        s = score_position(pos, black_positions, white_positions)
        if s > best_score:
            best_score = s
            best_move = pos
    return best_move

class WhiteNextStepHandler(tornado.web.RequestHandler):

    def set_default_headers(self):
        # Allow all origins to access this resource
        self.set_header("Access-Control-Allow-Origin", "*")
        self.set_header("Access-Control-Allow-Headers", "x-requested-with")
        self.set_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS")

    def options(self):
        # no body
        self.set_status(204)
        self.finish()

    def get(self):
        self.set_header('Content-type', 'application/json')
        self.set_header('Access-Control-Allow-Origin',  '*')
        steps_url = self.get_argument("stepsString").strip("_")
        next_token = predict_next_token(steps_url, "gomoku_llm/models/transformer_1000.pt")
        next_move = next_token
        x = ord(next_move[0]) - ord('a')
        y = int(next_move[1:]) - 1
        response = '{"input": "%s", "x": %s, "y": %s}' % (steps_url, x, y)
        self.write(str.encode(response))



class BlackNextStepHandler(tornado.web.RequestHandler):

    def set_default_headers(self):
        # Allow all origins to access this resource
        self.set_header("Access-Control-Allow-Origin", "*")
        self.set_header("Access-Control-Allow-Headers", "x-requested-with")
        self.set_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS")

    def options(self):
        # no body
        self.set_status(204)
        self.finish()

    def get(self):
        self.set_header('Content-type', 'application/json')
        self.set_header('Access-Control-Allow-Origin',  '*')
        steps_url = self.get_argument("stepsString").strip("_")
        token_list = [t for t in steps_url.split("_") if t]
        possible_moves = find_next_steps_from_db(steps_url, db)

        response_dict = None
        if len(possible_moves) > 0:
            next_move = possible_moves[0]
            x = ord(next_move[0]) - ord('a')
            y = int(next_move[1:]) - 1
            response_dict = {"input": steps_url, "x": x, "y": y}
        else:
            timeout_program = "gtimeout"
            if platform.system() == 'Linux':
                timeout_program = 'timeout'
            cmd = 'export LD_LIBRARY_PATH=/usr/local/clang_9.0.0/lib:$LD_LIBRARY_PATH&&%s 10s %s/web_search %s' % (timeout_program, os.getcwd(), steps_url)
            app_log.info("Find from running the program: %s" % cmd)
            res = os.popen(cmd).read()
            try:
                xy_dict = json.loads(res)
                x, y = xy_dict['x'], xy_dict['y']
                response_dict = {"input": steps_url, "x": x, "y": y}
            except Exception:
                app_log.warning("Fallback to heuristic black move due to parse failure.")

        if response_dict is None:
            x, y = best_heuristic_black_move(token_list)
            response_dict = {"input": steps_url, "x": x, "y": y}

        app_log.info("response final %s" % response_dict)
        self.write(json.dumps(response_dict).encode())


def make_app():
    handlers = [(r"/white_next_step", WhiteNextStepHandler), (r"/next_step", BlackNextStepHandler) ]
    handlers.append((r'/(.*)', web.StaticFileHandler, {'path': "./web/"}))
    return tornado.web.Application(handlers)


if __name__ == "__main__":
    if len(sys.argv) < 2:
        port = 8080
    else:
        port = int(sys.argv[1])
    app = make_app()
    app.listen(port)
    tornado.ioloop.IOLoop.current().start()
