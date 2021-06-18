if not exist build-emcc mkdir build-emcc
cd build-emcc
call ..\external\emsdk\emsdk_env.bat
emcmake cmake -DBUILD_WITH_EMCMAKE=ON ..
mingw32-make -j8
emrun --port 8080 --browser chrome ../build-emcc/bin/pbrGL.html

pause
