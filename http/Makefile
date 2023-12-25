cc := /usr/local/lib/wasi-sdk-20.0/bin/clang

.phony: gen clean run

default: main.wasm

wasi-http: ; git clone https://github.com/WebAssembly/wasi-http; cd wasi-http; git checkout v0.2.0-rc-2023-11-10

gen: wasi-http ; wit-bindgen c ${PWD}/wasi-http/wit -w proxy

main.wasm: gen ; ${cc} proxy.c main.c -o main.wasm

main_2023_11_10.embed.wasm: main.wasm ; wasm-tools component embed ./wasi-http/wit main.wasm > main_2023_11_10.embed.wasm

wasi_snapshot_preview1.reactor.wasm: ; wget https://github.com/bytecodealliance/wasmtime/releases/download/v15.0.1/wasi_snapshot_preview1.reactor.wasm

main_2023_11_10.component.wasm: main_2023_11_10.embed.wasm wasi_snapshot_preview1.reactor.wasm; wasm-tools component new main_2023_11_10.embed.wasm -o main_2023_11_10.component.wasm --adapt wasi_snapshot_preview1.reactor.wasm

clean: ; rm -f proxy.c proxy_component_type.o proxy.h *.wasm; rm -r wasi-http

run: main_2023_11_10.component.wasm ; wasmtime -S http --wasm component-model main_2023_11_10.component.wasm
