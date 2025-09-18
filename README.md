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

## Requirements

Currently tested only on Arch linux with kernel 6.16 or higher.

Required software:

- python3
- cmake

## Usage

Run `./compile.sh` to compile all schemes' source code.

Run `python3 bench.py {keygen,signing,verification}` to benchmark the preferred signature phase. 

Run `./clean.sh` to remove all compiled binaries.
