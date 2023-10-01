import os
import platform
import sys

import tornado.ioloop
from tornado import options
from tornado import web
from tornado.log import app_log

from divided_solution_manager import find_next_steps_from_board_str_hash2action, get_all_step_str2action

options.options["log_file_prefix"] = "tornado_log"
options.parse_command_line()

all_step_str2action = get_all_step_str2action()


class MainHandler(tornado.web.RequestHandler):

    def get(self):
        self.set_header('Content-type', 'application/json')
        self.set_header('Access-Control-Allow-Origin',  '*')
        steps_url = self.get_argument("stepsString").strip("_")
        possible_moves = find_next_steps_from_board_str_hash2action(steps_url, all_step_str2action)

        if len(possible_moves) > 0:
            next_move = possible_moves[0]
            x = ord(next_move[0]) - ord('a')
            y = int(next_move[1:]) - 1
            response = '{"input": "%s", "x": %s, "y": %s}' % (steps_url, x, y)
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

    app = make_app(debug)
    app.listen(port)
    tornado.ioloop.IOLoop.current().start()

