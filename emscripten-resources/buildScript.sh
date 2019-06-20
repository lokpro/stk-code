apt-get update
apt-get install -y pkg-config nasm automake libtool

export OWN_DIR="$(dirname $0)"

cd "${OWN_DIR}/ports"

source env.sh
ln -s "${EMSCRIPTEN}/system/include/GLES3/gl2ext.h" "${EMSCRIPTEN}/system/include/GLES3/gl3ext.h"

echo "Building all deps!"
bash buildAll.sh
echo "Built! Preparing STK!"
source env.sh

cd ../../
mkdir cmake_build_docker
cd cmake_build_docker

BUILD_DIR="$(pwd)"

mkdir -p overlay/opt/stk/share/supertuxkart/data
echo $EMSCRIPTEN > overlay/opt/stk/emscripten-dir.txt

mkdir bin

LINKER_ARGS="-D_REENTRANT \
-s USE_PTHREADS=0 \
-s WASM=1 \
-s SIMD=0 \
-s DISABLE_EXCEPTION_CATCHING=1 \
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
--pre-js ${OWN_DIR}/setupFs.js \
--preload-file $BUILD_DIR/overlay/opt@/opt \
--use-preload-cache \
--no-heap-copy \
\
-std=gnu++0x \
-Wall \
-Wno-unused-function \
-DNDEBUG @CMakeFiles/supertuxkart.dir/objects1.rsp \
@CMakeFiles/supertuxkart.dir/linklibs.rsp \
--emscripten-cxx \
-O3 \
\
\
-I${EMSCRIPTEN_SYSTEM}/include \
-I${EMSCRIPTEN_SYSTEM}/include/GL/gl.h \
-I${EMSCRIPTEN_SYSTEM}/usr/local/include/libfreetype2 \
-l${EMSCRIPTEN_SYSTEM}/usr/local/lib/libfreetype \
-l${EMSCRIPTEN_SYSTEM}/opt/libjpeg-turbo/lib32/libjpeg \
-l${EMSCRIPTEN_SYSTEM}/usr/local/lib/libpng \
-l${EMSCRIPTEN_SYSTEM}/usr/local/lib/libz \
-l${EMSCRIPTEN_SYSTEM}/usr/local/lib/libvorbisfile \
-l${EMSCRIPTEN_SYSTEM}/usr/local/lib/libvorbis \
-l${EMSCRIPTEN_SYSTEM}/usr/local/lib/libvorbisenc \
-l${EMSCRIPTEN_SYSTEM}/usr/local/lib/libogg \
-l${EMSCRIPTEN_SYSTEM}/usr/local/lib/libharfbuzz \
\
\
-s ASSERTIONS=0 \
-s ERROR_ON_UNDEFINED_SYMBOLS=0 \
-s ERROR_ON_MISSING_LIBRARIES=0 \
-s BINARYEN_TRAP_MODE='clamp'"

emconfigure cmake .. \
	    -DUSE_GLES2=1 \
	    -DBUILD_RECORDER=off \
	    -DNO_IRR_COMPILE_WITH_X11_=yes \
	    -DUSE_WIIUSE=0 \
	    -DENABLE_WAYLAND_DEVICE=0 \
	    -DCMAKE_INSTALL_PREFIX="/opt/stk" \
	    -DEMSCRIPTEN=1 \
	    -DCMAKE_EXE_LINKER_FLAGS="${LINKER_ARGS}"

emmake make -j$(nproc)
emmake make install DESTDIR="./overlay"

emcc $LINKER_ARGS -o "${BUILD_DIR}/bin/supertuxkart.html"
