echo "Benchmarking Verification Times"

grep_command=" 2>/dev/null | grep 'Verification kCycles' | grep -o '[0-9]*\.[0-9]*' | head -n 1"

echo "=========================SPECK========================="
echo "speck_252_133 $(eval 'echo -n ' ' & ./speck/build/SPECK_benchmark_cat_252_133'$grep_command)" | sed 's/ /\t\t/g'
sleep 0.1
echo "speck_252_256 $(eval './speck/build/SPECK_benchmark_cat_252_256'$grep_command)"
sleep 0.1
echo "speck_252_512 $(eval './speck/build/SPECK_benchmark_cat_252_512'$grep_command)"
sleep 0.1
echo "speck_252_768 $(eval './speck/build/SPECK_benchmark_cat_252_768'$grep_command)"
sleep 0.1
echo "speck_252_4096 $(eval './speck/build/SPECK_benchmark_cat_252_4096'$grep_command)"
sleep 0.1

echo "=========================SQISIGN========================="
echo "sqisign_cat1 $(eval './sqisign/build/apps/benchmark_lvl1'$grep_command)"
sleep 0.1

echo "=========================MIRATH========================="
echo "mirath_1a_fast $(eval './mirath/mirath_tcith_1a_fast/build/bench'$grep_command)"
sleep 0.1
echo "mirath_1a_short $(eval './mirath/mirath_tcith_1a_short/build/bench'$grep_command)"
sleep 0.1
echo "mirath_1b_fast $(eval './mirath/mirath_tcith_1b_fast/build/bench'$grep_command)"
sleep 0.1
echo "mirath_1b_short $(eval './mirath/mirath_tcith_1b_short/build/bench'$grep_command)"
sleep 0.1


echo "=========================RYDE========================="
echo "ryde_cat1_fast $(eval './ryde/ryde1f/build/bench/bench-ryde-1f'$grep_command)"
sleep 0.1
echo "ryde_cat1_short $(eval './ryde/ryde1s/build/bench/bench-ryde-1s'$grep_command)"
sleep 0.1

echo "=========================MQOM========================="
echo "mqom_cat1_gf256_fast_r3 $(eval './mqom/build/cat1_gf256_fast_r3/cat1_gf256_fast_r3_bench'$grep_command)"
sleep 0.1
echo "mqom_cat1_gf256_fast_r5 $(eval './mqom/build/cat1_gf256_fast_r5/cat1_gf256_fast_r5_bench'$grep_command)"
sleep 0.1
echo "mqom_cat1_gf256_short_r3 $(eval './mqom/build/cat1_gf256_short_r3/cat1_gf256_short_r3_bench'$grep_command)"
sleep 0.1
echo "mqom_cat1_gf256_short_r5 $(eval './mqom/build/cat1_gf256_short_r5/cat1_gf256_short_r5_bench'$grep_command)"
sleep 0.1
echo "mqom_cat1_gf2_fast_r3 $(eval './mqom/build/cat1_gf2_fast_r3/cat1_gf2_fast_r3_bench'$grep_command)"
sleep 0.1
echo "mqom_cat1_gf2_fast_r5 $(eval './mqom/build/cat1_gf2_fast_r5/cat1_gf2_fast_r5_bench'$grep_command)"
sleep 0.1
echo "mqom_cat1_gf2_short_r3 $(eval './mqom/build/cat1_gf2_short_r3/cat1_gf2_short_r3_bench'$grep_command)"
sleep 0.1
echo "mqom_cat1_gf2_short_r5 $(eval './mqom/build/cat1_gf2_short_r5/cat1_gf2_short_r5_bench'$grep_command)"
sleep 0.1

echo "=========================SDITH========================="
echo "sdith_hypercube_cat1_gf256 $(eval './sdith/sdith_hypercube_cat1_gf256/bench'$grep_command)"
sleep 0.1
echo "sdith_hypercube_cat1_p251 $(eval './sdith/sdith_hypercube_cat1_p251/bench'$grep_command)"
sleep 0.1
echo "sdith_threshold_cat1_gf256 $(eval './sdith/sdith_threshold_cat1_gf256/bench'$grep_command)"
sleep 0.1
echo "sdith_threshold_cat1_p251 $(eval './sdith/sdith_threshold_cat1_p251/bench'$grep_command)"
sleep 0.1

echo "=========================PERK========================="
echo "perk_128_fast_3 $(eval './perk/perk-128-fast-3/build/bin/bench'$grep_command)"
sleep 0.1
echo "perk_128_fast_5 $(eval './perk/perk-128-fast-5/build/bin/bench'$grep_command)"
sleep 0.1
echo "perk_128_short_3 $(eval './perk/perk-128-short-3/build/bin/bench'$grep_command)"
sleep 0.1
echo "perk_128_short_5 $(eval './perk/perk-128-short-5/build/bin/bench'$grep_command)"
sleep 0.1

echo "=========================FAEST========================="
echo "faest_128f $(eval './faest/faest_128f/bench/bench_faest'$grep_command)"
sleep 0.1
echo "faest_128s $(eval './faest/faest_128s/bench/bench_faest'$grep_command)"
sleep 0.1
echo "faest_em_128f $(eval './faest/faest_em_128f/bench/bench_faest'$grep_command)"
sleep 0.1
echo "faest_em_128s $(eval './faest/faest_em_128s/bench/bench_faest'$grep_command)"
sleep 0.1

echo "=========================CROSS========================="
echo "cross_cat1_rsdp_balanced $(eval './cross/Additional_Implementations/Benchmarking/build/bin/CROSS_benchmark_cat_1_RSDP_BALANCED'$grep_command)"
sleep 0.1
echo "cross_cat1_rsdp_speed $(eval './cross/Additional_Implementations/Benchmarking/build/bin/CROSS_benchmark_cat_1_RSDP_SPEED'$grep_command)"
sleep 0.1
echo "cross_cat1_rsdpg_balanced $(eval './cross/Additional_Implementations/Benchmarking/build/bin/CROSS_benchmark_cat_1_RSDPG_BALANCED'$grep_command)"
sleep 0.1
echo "cross_cat1_rsdpg_speed $(eval './cross/Additional_Implementations/Benchmarking/build/bin/CROSS_benchmark_cat_1_RSDPG_SPEED'$grep_command)"
sleep 0.1

echo "=========================LESS========================="
echo "less_252_45 $(eval './less/Optimized_Implementation/avx2/build/LESS_benchmark_cat_252_45'$grep_command)"
sleep 0.1
echo "less_252_68 $(eval './less/Optimized_Implementation/avx2/build/LESS_benchmark_cat_252_68'$grep_command)"
sleep 0.1
echo "less_252_192 $(eval './less/Optimized_Implementation/avx2/build/LESS_benchmark_cat_252_192'$grep_command)"
sleep 0.1
