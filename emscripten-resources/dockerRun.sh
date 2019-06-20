docker run --rm -v "$(pwd)/../../stk-assets:/stk-assets" -v "$(pwd)/..:/stk-code" trzeci/emscripten bash /stk-code/emscripten-resources/buildScript.sh
