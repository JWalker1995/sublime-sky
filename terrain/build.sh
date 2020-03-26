#!/bin/sh

rm -rf generated

# Compile message schemas
flatc --python -o generated/ ../schemas/worldgen.fbs

touch generated/__init__.py