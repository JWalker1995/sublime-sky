#!/bin/sh

sort --unique sublime-sky.files | xargs ls -1d 2>/dev/null > sublime-sky.files.tmp
mv sublime-sky.files.tmp sublime-sky.files

printf "src/\nthird_party/\nthird_party/websocketpp/\nbuild-debug/src/\n/Users/"`whoami`"/source/tonk/\n" > sublime-sky.includes

