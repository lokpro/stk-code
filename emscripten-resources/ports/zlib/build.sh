mkdir cmake_build
cd cmake_build
emconfigure cmake ..

emmake make -j$(nproc)
