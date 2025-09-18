import subprocess
import time

print("Benchmarking Verification Times (AVG of 128 runs)")

benchmarks = {}

def run_benchs(name,pairs):
    print(f"--------------------------{name.upper()}--------------------------")
    for pair in pairs:
        cmd = f"{pair[1]} 2>/dev/null | grep 'Verification' | grep -o '[0-9]*\\.[0-9]*'"
        result = subprocess.run(cmd, shell=True, capture_output=True).stdout.decode()
        values = [float(val) for val in result.split() if val]
        # values = [KCycles AVG, KCycles STDDEV, milliseconds AVG]
        print(f"{pair[0]:<30} {values[0]:>10.2f} KCycles {values[2]:>10.2f} ms")
        benchmarks[pair[0]] = [values[0],values[2]]
        time.sleep(0.01)

def sort_and_print(benchmarks):
    print(f"--------------------------ORDERED LIST--------------------------")
    sorted_bench = sorted(benchmarks.items(), key=lambda x:x[1][0])
    for i in range(len(benchmarks)):
        print(f"{(i+1):02}) {sorted_bench[i][0]:<30} {sorted_bench[i][1][0]:>10.2f} KCycles {sorted_bench[i][1][1]:>10.2f} ms")
        
speck_pairs = [
    ['speck_128_133', './speck/build/SPECK_benchmark_cat_128_133'],
    ['speck_128_256', './speck/build/SPECK_benchmark_cat_128_256'],
    ['speck_128_512', './speck/build/SPECK_benchmark_cat_128_512'],
    ['speck_128_768', './speck/build/SPECK_benchmark_cat_128_768'],
    ['speck_128_4096', './speck/build/SPECK_benchmark_cat_128_4096']
]

sqisign_pairs = [
    ['sqisign_cat1', './sqisign/build/apps/benchmark_lvl1']
]

mirath_pairs = [
    ['mirath_1a_fast', './mirath/mirath_tcith_1a_fast/build/bench'],
    ['mirath_1a_short', './mirath/mirath_tcith_1a_short/build/bench'],
    ['mirath_1b_fast', './mirath/mirath_tcith_1b_fast/build/bench'],
    ['mirath_1b_short', './mirath/mirath_tcith_1b_short/build/bench']
]


ryde_pairs = [
    ['ryde_cat1_fast','./ryde/ryde1f/build/bench/bench-ryde-1f'],
    ['ryde_cat1_short','./ryde/ryde1s/build/bench/bench-ryde-1s']
]

mqom_pairs = [
    ['mqom_cat1_gf256_fast_r3','./mqom/build/cat1_gf256_fast_r3/cat1_gf256_fast_r3_bench'],
    ['mqom_cat1_gf256_fast_r5','./mqom/build/cat1_gf256_fast_r5/cat1_gf256_fast_r5_bench'],
    ['mqom_cat1_gf256_short_r3','./mqom/build/cat1_gf256_short_r3/cat1_gf256_short_r3_bench'],
    ['mqom_cat1_gf256_short_r5','./mqom/build/cat1_gf256_short_r5/cat1_gf256_short_r5_bench'],
    ['mqom_cat1_gf2_fast_r3','./mqom/build/cat1_gf2_fast_r3/cat1_gf2_fast_r3_bench'],
    ['mqom_cat1_gf2_fast_r5','./mqom/build/cat1_gf2_fast_r5/cat1_gf2_fast_r5_bench'],
    ['mqom_cat1_gf2_short_r3','./mqom/build/cat1_gf2_short_r3/cat1_gf2_short_r3_bench'],
    ['mqom_cat1_gf2_short_r5','./mqom/build/cat1_gf2_short_r5/cat1_gf2_short_r5_bench']
]

sdith_pairs = [
    ['sdith_hypercube_cat1_gf256','./sdith/sdith_hypercube_cat1_gf256/bench'],
    ['sdith_hypercube_cat1_p251','./sdith/sdith_hypercube_cat1_p251/bench'],
    ['sdith_threshold_cat1_gf256','./sdith/sdith_threshold_cat1_gf256/bench'],
    ['sdith_threshold_cat1_p251','./sdith/sdith_threshold_cat1_p251/bench']
]

perk_pairs = [
    ['perk_128_fast_3','./perk/perk-128-fast-3/build/bin/bench'],
    ['perk_128_fast_5','./perk/perk-128-fast-5/build/bin/bench'],
    ['perk_128_short_3','./perk/perk-128-short-3/build/bin/bench'],
    ['perk_128_short_5','./perk/perk-128-short-5/build/bin/bench']
]

faest_pairs = [
    ['faest_128f','./faest/faest_128f/bench/bench_faest'],
    ['faest_128s','./faest/faest_128s/bench/bench_faest'],
    ['faest_em_128f','./faest/faest_em_128f/bench/bench_faest'],
    ['faest_em_128s','./faest/faest_em_128s/bench/bench_faest']
]

cross_pairs = [
    ['cross_cat1_rsdp_balanced','./cross/Additional_Implementations/Benchmarking/build/bin/CROSS_benchmark_cat_1_RSDP_BALANCED'],
    ['cross_cat1_rsdp_speed','./cross/Additional_Implementations/Benchmarking/build/bin/CROSS_benchmark_cat_1_RSDP_SPEED'],
    ['cross_cat1_rsdpg_balanced','./cross/Additional_Implementations/Benchmarking/build/bin/CROSS_benchmark_cat_1_RSDPG_BALANCED'],
    ['cross_cat1_rsdpg_speed','./cross/Additional_Implementations/Benchmarking/build/bin/CROSS_benchmark_cat_1_RSDPG_SPEED']
]

less_pairs = [
    ['less_252_45','./less/Optimized_Implementation/avx2/build/LESS_benchmark_cat_252_45'],
    ['less_252_68','./less/Optimized_Implementation/avx2/build/LESS_benchmark_cat_252_68'],
    ['less_252_192','./less/Optimized_Implementation/avx2/build/LESS_benchmark_cat_252_192']
]


run_benchs('sdith',sdith_pairs)
run_benchs('speck',speck_pairs)
run_benchs('sqisign',sqisign_pairs)
run_benchs('mirath',mirath_pairs)
run_benchs('perk',perk_pairs)
run_benchs('faest',faest_pairs)
run_benchs('mqom',mqom_pairs)
run_benchs('ryde',ryde_pairs)
run_benchs('cross',cross_pairs)
run_benchs('less',less_pairs)
sort_and_print(benchmarks)
