#!/bin/bash
set -e # Exit if something goes wrong
bench_dir=$(dirname $(realpath $0))

while true; do
    read -p "Do you wish to clean all binaries? " yn
    case $yn in
        [Yy]* ) break;;
        [Nn]* ) exit;;
        * ) echo "Please answer y/n.";;
    esac
done

echo "Starting binaries removal!"

# CROSS

echo "Cleaning CROSS"

cd $bench_dir/cross/Additional_Implementations/Benchmarking
rm -rf build
cd $bench_dir

echo "CROSS binaries removed successfully"

# LESS

echo "Cleaning LESS"

cd $bench_dir/less/Optimized_Implementation/avx2
rm -rf build
cd $bench_dir

echo "LESS binaries removed successfully"

# SPECK

echo "Cleaning SPECK"

cd $bench_dir/speck
rm -rf build
cd $bench_dir

echo "SPECK binaries removed successfully"

# FAEST

echo "Cleaning FAEST"

cd $bench_dir/faest/faest_128f
make clean
cd ../faest_128s
make clean
cd ../faest_em_128f
make clean
cd ../faest_em_128s
make clean
cd $bench_dir

echo "FAEST binaries removed successfully"

# MQOM

echo "Cleaning MQOM"

cd $bench_dir/mqom
rm -rf build
cd $bench_dir

echo "MQOM binaries removed successfully"

# MIRATH

echo "Cleaning MIRATH"

for target in mirath_tcith_1a_fast mirath_tcith_1a_short mirath_tcith_1b_fast mirath_tcith_1b_short; do
    (cd $bench_dir/mirath/$target && rm -rf build)
done
cd $bench_dir

echo "MIRATH binaries removed successfully"

# RYDE

echo "Cleaning RYDE"

for target in ryde1f ryde1s; do
    (cd $bench_dir/ryde/$target && rm -rf build)
done
cd $bench_dir

echo "RYDE binaries removed successfully"

# SQISIGN

echo "Cleaning SQISIGN"

cd $bench_dir/sqisign
rm -rf build
cd $bench_dir

echo "SQISIGN binaries removed successfully"

# SDITH

echo "Cleaning SDITH"

for target in sdith_hypercube_cat1_gf256 sdith_hypercube_cat1_p251 sdith_threshold_cat1_gf256 sdith_threshold_cat1_p251; do
    (cd $bench_dir/sdith/$target && make clean)
done
cd $bench_dir

echo "SDITH binaries removed successfully"

# PERK

echo "Cleaning PERK"

for target in perk-128-fast-3 perk-128-fast-5 perk-128-short-3 perk-128-short-5; do
    (cd $bench_dir/perk/$target && make clean)
done
cd $bench_dir

echo "PERK binaries removed successfully"

echo "ALL BINARIES REMOVED SUCCESFULLY!"
