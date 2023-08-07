cmake \
    -DCMAKE_BUILD_TYPE="Debug" \
    -S./ \
    -B./cmake-build \
    -DCMAKE_C_FLAGS="-fsanitize=address"