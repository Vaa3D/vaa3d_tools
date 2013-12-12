cd /d "%~dp0"

rd /q/s __tmp_cmake_tmp__
md __tmp_cmake_tmp__
cd __tmp_cmake_tmp__

rd /q/s ../c/CMakeCache.txt

cmake ../c -DCMAKE_BUILD_TYPE=Debug -G "MinGW Makefiles"
make clean
make
make clean
cmake ../c -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles"
make
make install

cd ..
rd /q/s __tmp_cmake_tmp__