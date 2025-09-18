#!/bin/bash
set -e # Exit if something goes wrong
bench_dir=$(dirname $(realpath $0))

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

cd $bench_dir/cross/Additional_Implementations/Benchmarking
rm -rf build
mkdir build
cd build
cmake ..
make -j8
cd $bench_dir

echo "CROSS compiled successfully"

# LESS

echo "Starting LESS compilation"

cd $bench_dir/less/Optimized_Implementation/avx2
rm -rf build
mkdir build
cd build
cmake ..
make -j8
cd $bench_dir

echo "LESS compiled successfully"

# SPECK

echo "Starting SPECK compilation"

cd $bench_dir/speck
rm -rf build
mkdir build
cd build
cmake ..
make -j8
cd $bench_dir

echo "SPECK compiled successfully"

# FAEST

echo "Starting FAEST compilation"

for target in faest_128f faest_128s faest_em_128f faest_em_128s; do
    (cd "$bench_dir/faest/$target" && make -j8)
done
cd $bench_dir

echo "FAEST compiled successfully"

# MQOM

echo "Starting MQOM compilation"

cd $bench_dir/mqom
python3 manage.py compile cat1
cd $bench_dir

echo "MQOM compiled successfully"

# MIRATH

echo "Starting MIRATH compilation"

for target in mirath_tcith_1a_fast mirath_tcith_1a_short mirath_tcith_1b_fast mirath_tcith_1b_short; do
    (cd "$bench_dir/mirath/$target" && rm -rf build && mkdir build && cd build && cmake .. && make -j8)
done
cd $bench_dir

echo "MIRATH compiled successfully"

# RYDE

echo "Starting RYDE compilation"

for target in ryde1f ryde1s; do
    (cd "ryde/$target" && rm -rf build && cmake -DCMAKE_BUILD_TYPE=Release -DOPT_IMPL=avx -B build && cmake --build build)
done
cd $bench_dir

echo "RYDE compiled successfully"

# SQISIGN

echo "Starting SQISIGN compilation"

cd $bench_dir/sqisign
rm -rf build
mkdir build
cd build
cmake -DSQISIGN_BUILD_TYPE=broadwell -DCMAKE_BUILD_TYPE=Release ..
make -j8
cd $bench_dir

echo "SQISIGN compiled successfully"

# SDITH

echo "Starting SDITH compilation"

for target in sdith_hypercube_cat1_gf256 sdith_hypercube_cat1_p251 sdith_threshold_cat1_gf256 sdith_threshold_cat1_p251; do
    (cd "$bench_dir/sdith/$target" && make -j8)
done
cd $bench_dir

echo "SDITH compiled successfully"

# PERK

echo "Starting PERK compilation"

for target in perk-128-fast-3 perk-128-fast-5 perk-128-short-3 perk-128-short-5; do
    (cd "$bench_dir/perk/$target" && make -j8)
done
cd $bench_dir

echo "PERK compiled successfully"

echo "ALL SCHEMES COMPILED SUCCESFULLY!"
