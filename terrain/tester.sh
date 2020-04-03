#!/bin/sh

../build-release/sublime-sky ../config_server.json ../config_game.json &
python3.7 server.py
wait
