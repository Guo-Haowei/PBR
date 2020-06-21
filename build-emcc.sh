#!/bin/bash
source ./external/emsdk/emsdk_env.sh || exit 1
if [ ! -d build-emcc ]; then
    mkdir build-emcc || exit 1
fi
cd build-emcc || exit 1
emcmake cmake -DBUILD_WITH_EMCMAKE=ON .. || exit 1
make -j8 || exit 1
emrun --port 8080 --browser chrome ../build-emcc/bin/pbrGL.html