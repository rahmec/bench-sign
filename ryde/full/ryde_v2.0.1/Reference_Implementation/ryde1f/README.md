# RYDE

This `README.md` file explains how to build, compile, and run the provided implementation of the **RYDE** scheme.

## Build and Compile

```bash
rm -rf build

# Reference Implementation (ref)
cmake -DCMAKE_BUILD_TYPE=Release -DOPT_IMPL=ref -B build
cmake --build build
```

By default, the implementation instantiates the commit calculation employing Rijndael. For commit calculations using SHA3, you must include `-DCMT_IMPL=SHA3`:

```bash
rm -rf build

# Reference Implementation (ref)
cmake -DCMAKE_BUILD_TYPE=Release -DOPT_IMPL=ref -DCMT_IMPL=SHA3 -B build
cmake --build build
```

### Debug mode

The **debug mode** enables the AddressSanitizer (ASan) to detect memory errors.
For enable the **debug mode** you must use `-DCMAKE_BUILD_TYPE=Debug`. For example,

```bash
rm -rf build

# Reference Implementation (ref)
cmake -DCMAKE_BUILD_TYPE=Debug -DOPT_IMPL=ref -B build
cmake --build build
```

## Intermediate Values

```bash
./build/tests/example-ryde-1f
```

## Benchmark

```bash
./build/bench/bench-ryde-1f
```

## NIST KATs

```bash
./build/nist/kat-ryde-1f
```

## Remarks

* This current parameter set does **not** require to increase the stack.
* Some parts of the field arithmetic are based on the generated code from https://rbc-lib.org
* The XKCP library is taken from https://github.com/XKCP/XKCP

## License

For the third-party code see their licenses:

* https://gitlab.owndata.org/rbc/rbc-lib/-/blob/master/license.txt?ref_type=heads
* https://github.com/XKCP/XKCP/blob/master/LICENSE
