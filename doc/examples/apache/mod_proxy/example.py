#!/usr/bin/python
# -*- coding: utf-8; -*-

import bottle

@bottle.get("/")
def index():
    u = bottle.request.headers["Remote-User"]
    return("Hey, %s! [from bottle]" % u)

if (__name__ == "__main__"):
    bottle.run(port=4080)
