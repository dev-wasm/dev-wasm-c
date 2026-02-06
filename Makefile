sdk := /usr/local/lib/wasi-sdk-30.0-x86_64-linux/
cc := ${sdk}/bin/clang
cpp := ${sdk}/bin/clang++

.phony: all clean validate-environment

default: all

validate-environment:
	@echo "Validating devcontainer environment setup..."
	@echo "Checking wasi-sdk-30.0 installation..."
	@test -d ${sdk} || (echo "ERROR: wasi-sdk-30.0 not found at ${sdk}" && exit 1)
	@${cc} --version | head -1
	@echo "Checking wasmtime version..."
	@wasmtime --version | grep -q "41.0.2" || (echo "WARNING: Expected wasmtime 41.0.2" && wasmtime --version)
	@echo "Checking wit-bindgen availability..."
	@which wit-bindgen > /dev/null || (echo "ERROR: wit-bindgen not found" && exit 1)
	@echo "Checking wasm-tools availability..."
	@which wasm-tools > /dev/null || (echo "ERROR: wasm-tools not found" && exit 1)
	@echo "Environment validation complete!"

verify: validate-environment all check
	@echo "All verification steps passed successfully!"

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
