CC?=gcc
CXX?=g++
CFLAGS+= -g -O3 -std=c23 -march=native -mtune=native -flto
CXXFLAGS+= -g -O3 -std=c++23 -march=native -mtune=native -flto
CPPFLAGS+= -I. -Isha3 -DHAVE_OPENSSL
LDFLAGS+= -flto=auto -lm

SOURCES=$(filter-out randomness_os.c randomness_randombytes.c,$(wildcard *.c)) $(wildcard *.cpp) $(wildcard avx2/*.c) $(wildcard avx2/*.cpp) $(wildcard sha3/*.c) $(wildcard sha3/*.s)
EXECUTABLE_BENCH=bench/bench_faest
LIBFAEST=libfaest.a
BENCH_SOURCES=$(wildcard bench/*.c)

all: $(LIBFAEST) $(EXECUTABLE_BENCH) 
$(LIBFAEST): $(SOURCES:.c=.o) $(SOURCES:.cpp=.o) $(SOURCES:.s=.o)
	ar rcs $@ $^
$(EXECUTABLE_BENCH): $(BENCH_SOURCES:.c=.o) $(LIBFAEST) randomness_randombytes.o
	$(CC) $(CPPFLAGS) $(LDFLAGS) $^ -lcrypto -o $@

clean:
	rm -f \
		$(wildcard *.o) \
		$(wildcard sha3/*.o) \
		$(wildcard NIST-KATs/*.o) \
		$(wildcard tests/*.o) \
		$(LIBFAEST) $(EXECUTBALE_BENCH)
