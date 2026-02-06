sdk := /usr/local/lib/wasi-sdk-30.0-x86_64-linux/
cc := ${sdk}/bin/clang
cpp := ${sdk}/bin/clang++

.phony: all clean

default: all

main.wasm: main.c
	${cc} -o main.wasm main.c

run_c: main.wasm
	wasmtime --dir . main.wasm

main-cc.wasm: main.cc
	${cpp} -o main-cc.wasm main.cc

run_cc: main-cc.wasm
	wasmtime --dir . main-cc.wasm

all: run_c run_cc

clean:
	rm -f main.wasm main-cc.wasm test.txt test2.txt test-cc.txt test-cc-2.txt

check: all
	ls test.txt > /dev/null
	ls test2.txt > /dev/null
	ls test-cc.txt > /dev/null
	ls test-cc-2.txt > /dev/null
