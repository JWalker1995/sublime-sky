#!/bin/bash

# Change working directory to script location
pushd `dirname $0` > /dev/null

rm -rf flatbuffers imgui readerwriterqueue spdlog staticsort

mkdir imgui readerwriterqueue spdlog staticsort

# curl -Lo flatbuffers/LICENSE.txt https://raw.githubusercontent.com/google/flatbuffers/v1.10.0/LICENSE.txt
# curl -Lo flatbuffers/base.h https://raw.githubusercontent.com/google/flatbuffers/v1.10.0/include/flatbuffers/base.h
# curl -Lo flatbuffers/code_generators.h https://raw.githubusercontent.com/google/flatbuffers/v1.10.0/include/flatbuffers/code_generators.h
# curl -Lo flatbuffers/flatbuffers.h https://raw.githubusercontent.com/google/flatbuffers/v1.10.0/include/flatbuffers/flatbuffers.h
# curl -Lo flatbuffers/flatc.h https://raw.githubusercontent.com/google/flatbuffers/v1.10.0/include/flatbuffers/flatc.h
# curl -Lo flatbuffers/flexbuffers.h https://raw.githubusercontent.com/google/flatbuffers/v1.10.0/include/flatbuffers/flexbuffers.h
# curl -Lo flatbuffers/grpc.h https://raw.githubusercontent.com/google/flatbuffers/v1.10.0/include/flatbuffers/grpc.h
# curl -Lo flatbuffers/hash.h https://raw.githubusercontent.com/google/flatbuffers/v1.10.0/include/flatbuffers/hash.h
# curl -Lo flatbuffers/idl.h https://raw.githubusercontent.com/google/flatbuffers/v1.10.0/include/flatbuffers/idl.h
# curl -Lo flatbuffers/reflection.h https://raw.githubusercontent.com/google/flatbuffers/v1.10.0/include/flatbuffers/reflection.h
# curl -Lo flatbuffers/reflection_generated.h https://raw.githubusercontent.com/google/flatbuffers/v1.10.0/include/flatbuffers/reflection_generated.h
# curl -Lo flatbuffers/registry.h https://raw.githubusercontent.com/google/flatbuffers/v1.10.0/include/flatbuffers/registry.h
# curl -Lo flatbuffers/stl_emulation.h https://raw.githubusercontent.com/google/flatbuffers/v1.10.0/include/flatbuffers/stl_emulation.h
# curl -Lo flatbuffers/util.h https://raw.githubusercontent.com/google/flatbuffers/v1.10.0/include/flatbuffers/util.h
# curl -Lo flatbuffers/idl_gen_text.cpp https://raw.githubusercontent.com/google/flatbuffers/v1.10.0/src/idl_gen_text.cpp
# curl -Lo flatbuffers/idl_parser.cpp https://raw.githubusercontent.com/google/flatbuffers/v1.10.0/src/idl_parser.cpp
# curl -Lo flatbuffers/util.cpp https://raw.githubusercontent.com/google/flatbuffers/v1.10.0/src/util.cpp

curl -Lo imgui/LICENSE.txt https://raw.githubusercontent.com/ocornut/imgui/v1.65/LICENSE.txt
curl -Lo imgui/imgui.cpp https://raw.githubusercontent.com/ocornut/imgui/v1.65/imgui.cpp
curl -Lo imgui/imgui.h https://raw.githubusercontent.com/ocornut/imgui/v1.65/imgui.h
curl -Lo imgui/imgui_demo.cpp https://raw.githubusercontent.com/ocornut/imgui/v1.65/imgui_demo.cpp
curl -Lo imgui/imgui_draw.cpp https://raw.githubusercontent.com/ocornut/imgui/v1.65/imgui_draw.cpp
curl -Lo imgui/imgui_widgets.cpp https://raw.githubusercontent.com/ocornut/imgui/v1.65/imgui_widgets.cpp
curl -Lo imgui/imgui_internal.h https://raw.githubusercontent.com/ocornut/imgui/v1.65/imgui_internal.h
curl -Lo imgui/imstb_rectpack.h https://raw.githubusercontent.com/ocornut/imgui/v1.65/imstb_rectpack.h
curl -Lo imgui/imstb_textedit.h https://raw.githubusercontent.com/ocornut/imgui/v1.65/imstb_textedit.h
curl -Lo imgui/imstb_truetype.h https://raw.githubusercontent.com/ocornut/imgui/v1.65/imstb_truetype.h
echo > imgui/imconfig.h

curl -Lo imgui/imgui_impl_glfw.h https://raw.githubusercontent.com/ocornut/imgui/v1.65/examples/imgui_impl_glfw.h
curl -Lo imgui/imgui_impl_glfw.cpp https://raw.githubusercontent.com/ocornut/imgui/v1.65/examples/imgui_impl_glfw.cpp
curl -Lo imgui/imgui_impl_opengl3.h https://raw.githubusercontent.com/ocornut/imgui/v1.65/examples/imgui_impl_opengl3.h
curl -Lo imgui/imgui_impl_opengl3.cpp https://raw.githubusercontent.com/ocornut/imgui/v1.65/examples/imgui_impl_opengl3.cpp

curl -Lo readerwriterqueue/LICENSE.md https://raw.githubusercontent.com/cameron314/readerwriterqueue/v1.0.0/LICENSE.md
curl -Lo readerwriterqueue/atomicops.h https://raw.githubusercontent.com/cameron314/readerwriterqueue/v1.0.0/atomicops.h
curl -Lo readerwriterqueue/readerwriterqueue.h https://raw.githubusercontent.com/cameron314/readerwriterqueue/v1.0.0/readerwriterqueue.h

curl -Lo spdlog_131.zip https://github.com/gabime/spdlog/archive/v1.3.1.zip
unzip -o spdlog_131.zip
mv spdlog-1.3.1/include/spdlog .
rm -r spdlog_131.zip spdlog-1.3.1

curl -Lo staticsort/staticsort.h https://raw.githubusercontent.com/Vectorized/Static-Sort/56da540e3ba322effd2985083b0801cdfb9b1fe7/static_sort.h

git clone git@github.com:tuxalin/THST.git
pushd THST
git apply ../THST_bbox.patch
popd

popd > /dev/null
