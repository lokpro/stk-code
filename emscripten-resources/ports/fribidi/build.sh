emconfigure automake --add-missing
emconfigure ./autogen.sh
emmake make -j$(nproc)
