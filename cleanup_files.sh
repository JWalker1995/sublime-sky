#!/bin/sh

sort --unique sublime_sky.files | xargs ls -1d 2>/dev/null > sublime_sky.files.tmp
mv sublime_sky.files.tmp sublime_sky.files

printf "src/\nthird_party/\nbuild-debug/src/\n/Users/"`whoami`"/source/tonk/\n" > sublime_sky.includes

