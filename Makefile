cc := /usr/local/lib/wasi-sdk-20.0/bin/clang

.phony: all clean

default: all

main.wasm: main.c
	clang -o main.wasm main.c

run_c: main.wasm
	wasmtime --dir . main.wasm

main-cc.wasm: main.cc
	clang++ -o main-cc.wasm main.cc

run_cc: main-cc.wasm
	wasmtime --dir . main-cc.wasm

all: run_c run_cc

clean:
	rm -f main.wasm main-cc.wasm test.txt test2.txt test-cc.txt test-cc-2.txt
