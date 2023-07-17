cc := /usr/local/lib/wasi-sdk-20.0/bin/clang

.phony: gen clean run

default: main.wasm

wasi-http: ; git clone https://github.com/WebAssembly/wasi-http; cd wasi-http; git checkout 1c95bc2

gen: wasi-http ; wit-bindgen c ${PWD}/wasi-http/wit -w proxy

main.wasm: gen ; ${cc} proxy.c proxy_component_type.o main.c -o main.wasm

clean: ; rm -f proxy.c proxy_component_type.o proxy.h *.wasm; rm -r wasi-http

run: main.wasm ; wasmtime --wasi-modules=experimental-wasi-http main.wasm
