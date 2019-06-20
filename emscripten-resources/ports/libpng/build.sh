git checkout libpng-1.6.31-signed
mkdir cmake_build
cd cmake_build
emconfigure cmake .. -DM_LIBRARY=""
emmake make -j$(nproc)
