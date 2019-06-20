# icky
git checkout 3a9394635ffd663d8acd0715236dd01d9f22f3b8
mkdir cmake_build
cd cmake_build
emconfigure cmake .. -DHB_HAVE_FREETYPE=ON -DHB_BUILD_TESTS=OFF "-DINCLUDE_DIRECTORIES=${EMSCRIPTEN_SYSTEM}/usr/local/include/freetype2"
emmake make -j$(nproc)
