import os
import platform
import sys

import leveldb
import tornado.ioloop
from tornado import options
from tornado import web
from tornado.log import app_log

from divided_solution_manager import find_next_steps_from_db

options.options["log_file_prefix"] = "tornado_log"
options.parse_command_line()

db = leveldb.LevelDB('leveldb.db')

class MainHandler(tornado.web.RequestHandler):

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
        possible_moves = find_next_steps_from_db(steps_url, db)

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


def make_app():
    handlers = [(r"/next_step", MainHandler), (r"/", MainHandler) ]
    handlers.append((r'/(.*)', web.StaticFileHandler, {'path': "./web/"}))
    return tornado.web.Application(handlers)


if __name__ == "__main__":
    port = int(sys.argv[1])
    app = make_app()
    app.listen(port)
    tornado.ioloop.IOLoop.current().start()

