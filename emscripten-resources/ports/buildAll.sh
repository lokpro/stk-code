base=$(pwd)

for port in "libjpeg-turbo/" "zlib/" "libpng/" "ogg/" "vorbis/" "freetype/" "harfbuzz/" "fribidi/"; do
  source "${base}/env.sh"

  echo "Building port: ${port}!"
  cd "${base}/${port}"
  bash "${base}/${port}/clone.sh"
  echo "Downloaded ${port}, building"
  cd "${base}/${port}/library"
  bash "${base}/${port}/build.sh"
  echo "Built ${port}, installing"
  cd "${base}/${port}/library"
  bash "${base}/${port}/install.sh"
done
