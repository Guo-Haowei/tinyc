cd unit/build || exit 1
cmake --build . || exit 1
./test.out
