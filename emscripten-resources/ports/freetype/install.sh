cd cmake_build
emmake make install DESTDIR="${EMSCRIPTEN_SYSTEM}"

for file in "${EMSCRIPTEN_SYSTEM}/usr/local/include/freetype2"/*; do
ln -s "${file}" "${EMSCRIPTEN_SYSTEM}/usr/local/include/$(basename "${file}")"
done
