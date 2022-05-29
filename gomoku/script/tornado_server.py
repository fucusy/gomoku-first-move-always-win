import os
import platform
import sys

import tornado.ioloop
from divided_solution_manager import apply_all_transformation_for_steps_str, apply_transformation, find_next_steps_from_board_str_hash2action
from divided_solution_manager import get_all_step_str2action, normalize_steps_str, steps2board_str, board_str_hash, move2position
from tornado import options
from tornado import web
from tornado.log import app_log

options.options["log_file_prefix"] = "tornado_log"
options.parse_command_line()

def find_simple_steps(steps_url):
    """
    The second white move is 3 step far way from the center
    :param steps_url: e.g. h8_g11
    :return:
    """
    steps = steps_url.strip("_").split("_")
    whilte_first_move = steps[1]
    black_first_move = "h8"
    white_first_x, white_first_y = move2position(whilte_first_move)
    black_first_x, black_first_y = move2position(black_first_move)

    if not steps[0] == black_first_move:
        return []
    if abs(white_first_x - black_first_x) <= 2 and abs(white_first_y - black_first_y) <= 2:
        return []

    # black step two
    if len(steps) == 2:
        possible_solution = ["i7", "i9", "g7", "g9"]
        max_dis = 0
        max_res = "i7"
        for s in possible_solution:
            x3, y3 = move2position(s)
            dis = abs(white_first_x-x3) + abs(white_first_y-y3)
            if dis > max_dis:
                max_dis = dis
                max_res = s
        return [max_res]

    step_3 = {"i7": ["j8", "i9", "g7", "h6"], \
              "i9": ["j8", "i7", "g9", "h10"], \
              "g7": ["g9", "i7", "f8", "h6"], \
              "g9": ["g7", "i9", "f8", "h10"], }
    # black step three
    white_step_2 = steps[3]
    white_step_2_x, white_step_2_y = move2position(white_step_2)
    if len(steps) == 4:
        black_step_2 = steps[2]
        possible_solution = step_3[black_step_2]
        max_dis = 0
        max_res = possible_solution[0]
        for s in possible_solution:
            if s not in steps:
                x3, y3 = move2position(s)
                dis = abs(white_step_2_x-x3) + abs(white_step_2_y-y3) + abs(white_first_x-x3) + abs(white_first_y-y3)
                if dis > max_dis:
                    max_dis = dis
                    max_res = s
        return [max_res]
    return []

class MainHandler(tornado.web.RequestHandler):

    def get(self):
        self.set_header('Content-type', 'application/json')
        self.set_header('Access-Control-Allow-Origin',  '*')
        steps_url = self.get_argument("stepsString").strip("_")
        possible_moves = find_next_steps_from_board_str_hash2action(steps_url, board_str_hash2action)

        if len(possible_moves) > 0:
            next_move = possible_moves[0]
            x = ord(next_move[0]) - ord('a')
            y = int(next_move[1:]) - 1
            response = '{"input": "%s", "cache_count": %s, "x": %s, "y": %s}' % \
                       (steps_url, len(board_str_hash2action), x, y)
        else:
            timeout_program = "gtimeout"
            if platform.system() == 'Linux':
                timeout_program = 'timeout'
            cmd = 'export LD_LIBRARY_PATH=/usr/local/clang_9.0.0/lib:$LD_LIBRARY_PATH&&%s 10s %s/web_search %s' % (timeout_program, os.getcwd(), steps_url)
            app_log.info("Find from running the program: %s" % cmd)
            res = os.popen(cmd).read()
            response = res
        app_log.info("response final %s" % response)
        self.write(str.encode(response))


def make_app(debug):
    handlers = [(r"/", MainHandler) ]
    if debug:
        handlers.append((r'/(.*)', web.StaticFileHandler, {'path': "./"}))
    return tornado.web.Application(handlers)


if __name__ == "__main__":
    debug = False
    port = int(sys.argv[1])
    if len(sys.argv) > 2:
        print("DEBUG MODE, Server Web UI Here")
        debug = True
    global board_str_hash2action
    board_str_hash2action = get_all_step_str2action()
    print("cache step_str2action size ", len(board_str_hash2action))

    app = make_app(debug)
    app.listen(port)
    tornado.ioloop.IOLoop.current().start()

