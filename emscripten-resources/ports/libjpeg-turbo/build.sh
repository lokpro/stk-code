mkdir cmake_build
cd cmake_build
emconfigure cmake -DWITH_SIMD=0 ..
emmake make -j$(nproc)
