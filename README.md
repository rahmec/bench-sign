# bench-sign

Benchmarking tool for digital signature schemes.
Currently benchmarkable schemes:

- CROSS
- FAEST
- LESS
- MIRATH
- MQOM
- PERK
- RYDE
- SDITH
- SPECK
- SQISIGN

Still a work in progress.

## USAGE

Run `./compile.sh` to compile all schemes' source code.

Run `python3 bench_signing.py` to benchmark signing times

Run `python3 bench_verification.py` to benchmark signing times

Run `./clean.sh` to remove all compiled binaries.
