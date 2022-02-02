import os
import platform
import sys

import tornado.ioloop
from divided_solution_manager import apply_all_transformation_for_steps_str, apply_transformation
from divided_solution_manager import get_all_step_str2action, normalize_steps_str, steps2board_str
from tornado import options
from tornado import web
from tornado.log import app_log

options.options["log_file_prefix"] = "tornado_log"
options.parse_command_line()


class MainHandler(tornado.web.RequestHandler):

    def get(self):
        self.set_header('Content-type', 'application/json')
        self.set_header('Access-Control-Allow-Origin',  '*')
        steps_url = self.get_argument("stepsString").strip("_")
        before_trans = ''
        tran_action = ''
        new_format_steps, trans = apply_all_transformation_for_steps_str(steps_url)
        next_move = ""
        for steps, tran in zip(new_format_steps, trans):
            norm_steps = steps2board_str(steps)
            if norm_steps in board_str2action:
                app_log.info("Find from saved step_str2action")
                before_trans = board_str2action[norm_steps]
                tran_action = tran
                next_move = apply_transformation(before_trans, tran, reverse=True)
                break

        if next_move != "":
            x = ord(next_move[0]) - ord('a')
            y = int(next_move[1:]) - 1
            response = '{"input": "%s", "cache_count": %s, "x": %s, "y": %s, "before_trans": "%s", "trans_action": "%s"}' % \
                       (steps_url, len(board_str2action), x, y, before_trans, tran_action)

        else:
            timeout_program = "gtimeout"
            if platform.system() == 'Linux':
                timeout_program = 'timeout'
            cmd = 'export LD_LIBRARY_PATH=/usr/local/clang_9.0.0/lib:$LD_LIBRARY_PATH&&%s 5s %s/web_search %s' % (timeout_program, os.getcwd(), steps_url)
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
    global board_str2action
    board_str2action = get_all_step_str2action()
    print("cache step_str2action size ", len(board_str2action))

    app = make_app(debug)
    app.listen(port)
    tornado.ioloop.IOLoop.current().start()

