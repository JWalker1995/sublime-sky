#!/bin/bash

set -e
set -x

# Change working directory to script location
pushd `dirname $0` > /dev/null

rm -rf flatbuffers imgui readerwriterqueue spdlog Static-Sort THST

git clone git@github.com:ocornut/imgui.git
pushd imgui
git checkout v1.75
popd

git clone git@github.com:cameron314/readerwriterqueue.git
pushd readerwriterqueue
git checkout v1.0.2
popd

# curl -Lo imgui/imgui_impl_glfw.h https://raw.githubusercontent.com/ocornut/imgui/v1.65/examples/imgui_impl_glfw.h
# curl -Lo imgui/imgui_impl_glfw.cpp https://raw.githubusercontent.com/ocornut/imgui/v1.65/examples/imgui_impl_glfw.cpp
# curl -Lo imgui/imgui_impl_opengl3.h https://raw.githubusercontent.com/ocornut/imgui/v1.65/examples/imgui_impl_opengl3.h
# curl -Lo imgui/imgui_impl_opengl3.cpp https://raw.githubusercontent.com/ocornut/imgui/v1.65/examples/imgui_impl_opengl3.cpp

git clone git@github.com:gabime/spdlog.git
pushd spdlog
git checkout v1.5.0
popd

git clone git@github.com:Vectorized/Static-Sort.git
pushd Static-Sort
git checkout b2991473a606a63a4d330af5b930df657e3eb7c7
popd

git clone git@github.com:tuxalin/THST.git
pushd THST
git apply ../THST_bbox.patch
popd

git clone git@github.com:zaphoyd/websocketpp.git
pushd websocketpp
git checkout 0.8.1
popd


popd > /dev/null
