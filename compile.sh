#!/bin/bash
set -e # Exit if something goes wrong
bench_dir=$(pwd)

while true; do
    read -p "Do you wish to recompile all schemes? " yn
    case $yn in
        [Yy]* ) break;;
        [Nn]* ) exit;;
        * ) echo "Please answer y/n.";;
    esac
done

echo "Starting compilation of all schemes!"

# CROSS

echo "Starting CROSS compilation"

cd cross/Additional_Implementations/Benchmarking
rm -rf build
mkdir build
cd build
cmake ..
make -j8
cd $bench_dir

echo "CROSS compiled successfully"

# LESS

echo "Starting LESS compilation"

cd less/Optimized_Implementation/avx2
rm -rf build
mkdir build
cd build
cmake ..
make -j8
cd $bench_dir

echo "LESS compiled successfully"

# SPECK

echo "Starting SPECK compilation"

cd speck
rm -rf build
mkdir build
cd build
cmake ..
make -j8
cd $bench_dir

echo "SPECK compiled successfully"

# FAEST

echo "Starting FAEST compilation"

cd faest/faest_128f
make -j8
cd ../faest_128s
make -j8
cd ../faest_em_128f
make -j8
cd ../faest_em_128s
make -j8
cd $bench_dir

echo "FAEST compiled successfully"

# MQOM

echo "Starting MQOM compilation"

cd mqom
python3 manage.py compile cat1
cd $bench_dir

echo "MQOM compiled successfully"

# MIRATH

echo "Starting MIRATH compilation"

cd mirath/mirath_tcith_1a_fast
rm -rf build
mkdir build
cd build
cmake ..
make -j8
cd ../../mirath_tcith_1a_short
rm -rf build
mkdir build
cd build
cmake ..
make -j8
cd ../../mirath_tcith_1b_fast
rm -rf build
mkdir build
cd build
cmake ..
make -j8
cd ../../mirath_tcith_1b_short
rm -rf build
mkdir build
cd build
cmake ..
make -j8
cd $bench_dir

echo "MIRATH compiled successfully"

# RYDE

echo "Starting RYDE compilation"

cd ryde/ryde1f
rm -rf build
cmake -DCMAKE_BUILD_TYPE=Release -DOPT_IMPL=avx -B build
cmake --build build
cd ../ryde1s
rm -rf build
cmake -DCMAKE_BUILD_TYPE=Release -DOPT_IMPL=avx -B build
cmake --build build
cd $bench_dir

echo "RYDE compiled successfully"

# SQISIGN

echo "Starting SQISIGN compilation"

cd sqisign
rm -rf build
mkdir build
cd build
cmake -DSQISIGN_BUILD_TYPE=broadwell -DCMAKE_BUILD_TYPE=Release ..
make -j8
cd $bench_dir

echo "SQISIGN compiled successfully"

# SDITH

echo "Starting SDITH compilation"

cd sdith/sdith_hypercube_cat1_gf256
make -j8
cd ../sdith_hypercube_cat1_p251
make -j8
cd ../sdith_threshold_cat1_gf256
make -j8
cd ../sdith_threshold_cat1_p251
make -j8
cd $bench_dir

echo "SDITH compiled successfully"

# PERK

echo "Starting PERK compilation"

cd perk/perk-128-fast-3
make -j8
cd ../perk-128-fast-5
make -j8
cd ../perk-128-short-3
make -j8
cd ../perk-128-short-5
make -j8
cd $bench_dir

echo "PERK compiled successfully"

echo "ALL SCHEMES COMPILED SUCCESFULLY!"
