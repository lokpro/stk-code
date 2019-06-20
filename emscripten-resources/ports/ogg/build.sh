# mkdir cmake_build
# cd cmake_build
# emconfigure cmake ..
emconfigure ./autogen.sh
emconfigure ./autogen.sh
emconfigure ./configure
emmake make -j$(nproc)
