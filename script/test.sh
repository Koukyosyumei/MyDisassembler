# clean up
#rm -rf build/*

# build
./script/build.sh

# test for c++
cd build
ctest -V
cd ..

# cp build/script/myplan myplan
# cp ./myplan /usr/bin/myplan
