/usr/bin/python /home/heatingdevice/projects/emsdk/emscripten/1.38.13/emcc.py -D_REENTRANT -s DISABLE_EXCEPTION_CATCHING=0 -I/home/heatingdevice/projects/emsdk/emscripten/1.38.13/system/include  -s ERROR_ON_UNDEFINED_SYMBOLS=0 -s USE_PTHREADS=0 -s WASM=0 -s ALLOW_MEMORY_GROWTH=1 -O0 -std=gnu++0x -Wall -Wno-unused-function -DNDEBUG @CMakeFiles/supertuxkart.dir/objects1.rsp -o bin/supertuxkart.html --preload-file opt@/home/heatingdevice/projects/stk-code/cmake_build/opt @CMakeFiles/supertuxkart.dir/linklibs.rsp --emscripten-cxx -s DEMANGLE_SUPPORT=1 -s USE_WEBGL2=0 -I/home/heatingdevice/projects/freetype/include -l/home/heatingdevice/projects/freetype/cmake_build/libfreetype -l/home/heatingdevice/projects/emscripten-libjpeg-turbo/libjpeg-turbo/lib64/libjpeg -l/home/heatingdevice/projects/libpng-1.2.49/cmake_build/libpng -l/home/heatingdevice/projects/emscripten-zlib/cmake_build/libz -l/home/heatingdevice/projects/ogg/cmake_build/libogg -l/home/heatingdevice/projects/vorbis/cmake_build/lib/libvorbisfile -l/home/heatingdevice/projects/vorbis/cmake_build/lib/libvorbis -l/home/heatingdevice/projects/vorbis/cmake_build/lib/libvorbisenc -l/home/heatingdevice/projects/ogg/cmake_build/libogg -s FULL_ES2=1 -O3 -s DISABLE_EXCEPTION_CATCHING=1
