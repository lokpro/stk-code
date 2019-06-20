mkdir /tmp/.emscripten_cache
cp -rv /tmp/.emscripten_cache/* /home/heatingdevice/.emscripten_cache 
cp -v /tmp/.emscripten{,_sanity} /home/heatingdevice/
DEPS_DIR="/home/heatingdevice/projects" #Directory of built deps (vorbis,png,etc.)
BUILD_DIR="$DEPS_DIR/stk-code/cmake_build" #Directory you're building in

# -s FULL_ES2=1 \
# -s FULL_ES3=1 \

emcc \
-D_REENTRANT \
\
-s ERROR_ON_UNDEFINED_SYMBOLS=0 \
-s USE_PTHREADS=0 \
-s WASM=1 \
-s SIMD=0 \
-s DISABLE_EXCEPTION_CATCHING=0 \
\
-s BINARYEN_TRAP_MODE=clamp \
\
-s DEMANGLE_SUPPORT=1 \
\
-s FULL_ES2=1 \
-s USE_WEBGL2=0 \
\
-s ALLOW_MEMORY_GROWTH=1 \
-s TOTAL_MEMORY=33554432 \
\
--pre-js setupFs.js \
--preload-file $BUILD_DIR/opt@$BUILD_DIR/opt \
--use-preload-cache \
--no-heap-copy \
\
-std=gnu++0x \
-Wall \
-Wno-unused-function \
-DNDEBUG @CMakeFiles/supertuxkart.dir/objects1.rsp \
@CMakeFiles/supertuxkart.dir/linklibs.rsp \
--emscripten-cxx \
\
\
"-I${EMSCRIPTEN}/system/include" \
"-I${EMSCRIPTEN}/system/include/GL/gl.h" \
"-I${DEPS_DIR}/freetype/include" \
"-l${DEPS_DIR}/freetype/cmake_build/libfreetype" \
"-l${DEPS_DIR}/emscripten-libjpeg-turbo/libjpeg-turbo/lib64/libjpeg" \
"-l${DEPS_DIR}/libpng-1.2.49/cmake_build/libpng" \
"-l${DEPS_DIR}/emscripten-zlib/cmake_build/libz" \
"-l${DEPS_DIR}/vorbis/cmake_build/lib/libvorbisfile" \
"-l${DEPS_DIR}/vorbis/cmake_build/lib/libvorbis" \
"-l${DEPS_DIR}/vorbis/cmake_build/lib/libvorbisenc" \
"-l${DEPS_DIR}/ogg/cmake_build/libogg" \
-o "${BUILD_DIR}/bin/supertuxkart.html" \
-O3 $*
cp -rv ~/.emscripten_cache/* /tmp/.emscripten_cache
cp -v ~/.emscripten{,_sanity} /tmp/

