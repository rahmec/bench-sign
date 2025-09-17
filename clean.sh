#!/bin/bash
set -e # Exit if something goes wrong
bench_dir=$(pwd)

while true; do
    read -p "Do you wish to clean all binaries? " yn
    case $yn in
        [Yy]* ) break;;
        [Nn]* ) exit;;
        * ) echo "Please answer y/n.";;
    esac
done

echo "Starting compilation of all schemes!"

# CROSS

cd cross/Additional_Implementations/Benchmarking
rm -rf build
cd $bench_dir

# LESS

cd less/Optimized_Implementation/avx2
rm -rf build
cd $bench_dir

# SPECK

cd speck
rm -rf build
cd $bench_dir

# FAEST

cd faest/faest_128f
make clean
cd ../faest_128s
make clean
cd ../faest_em_128f
make clean
cd ../faest_em_128s
make clean
cd $bench_dir

# MQOM

cd mqom
rm -rf build
cd $bench_dir

# MIRATH

cd mirath/mirath_tcith_1a_fast
rm -rf build
cd ../mirath_tcith_1a_short
rm -rf build
cd ../mirath_tcith_1b_fast
rm -rf build
cd ../mirath_tcith_1b_short
rm -rf build
cd $bench_dir

# RYDE

cd ryde/ryde1f
rm -rf build
cd ../ryde1s
rm -rf build
cd $bench_dir

# SQISIGN

cd sqisign
rm -rf build
cd $bench_dir

# SDITH

cd sdith/sdith_hypercube_cat1_gf256
make clean
cd ../sdith_hypercube_cat1_p251
make clean
cd ../sdith_threshold_cat1_gf256
make clean
cd ../sdith_threshold_cat1_p251
make clean
cd $bench_dir

# PERK

cd perk/perk-128-fast-3
make clean
cd ../perk-128-fast-5
make clean
cd ../perk-128-short-3
make clean
cd ../perk-128-short-5
make clean
cd $bench_dir

echo "ALL BINARIES REMOVED SUCCESFULLY!"
