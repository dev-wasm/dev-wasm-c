cc := /usr/local/lib/wasi-sdk-20.0/bin/clang
client_objs := client.o main.o wasi_http.o

.phony: gen clean run

default: main.wasm

%.o : %.c
	@echo "Compiling c file into o file"
	${cc} -c $< -o $@

wasi-http: ; git clone https://github.com/WebAssembly/wasi-http; cd wasi-http; git checkout v0.2.0-rc-2023-11-10 ; cp ../client.wit wit/client.wit

gen: wasi-http ; wit-bindgen c ${PWD}/wasi-http/wit -w client ; wit-bindgen c ${PWD}/wasi-http/wit -w proxy

main.wasm: gen ${client_objs}; ${cc} ${client_objs} -o main.wasm

main_2023_11_10.embed.wasm: main.wasm ; wasm-tools component embed ./wasi-http/wit main.wasm -o main_2023_11_10.embed.wasm -w client

wasi_snapshot_preview1.reactor.wasm: ; wget https://github.com/bytecodealliance/wasmtime/releases/download/v15.0.1/wasi_snapshot_preview1.reactor.wasm

main_2023_11_10.component.wasm: main_2023_11_10.embed.wasm wasi_snapshot_preview1.reactor.wasm; wasm-tools component new main_2023_11_10.embed.wasm -o main_2023_11_10.component.wasm --adapt wasi_snapshot_preview1.reactor.wasm

clean: ; rm -f client.c client_component_type.o client.h proxy.h proxy.c *.wasm; rm -rf wasi-http

run: main_2023_11_10.component.wasm ; wasmtime -S http --wasm component-model main_2023_11_10.component.wasm

server.wasm: gen ; ${cc} proxy.c server.c -o server.wasm

server_2023_11_10.embed.wasm: server.wasm ; wasm-tools component embed ./wasi-http/wit server.wasm -o server_2023_11_10.embed.wasm -w proxy

server_2023_11_10.component.wasm: server_2023_11_10.embed.wasm wasi_snapshot_preview1.reactor.wasm; wasm-tools component new server_2023_11_10.embed.wasm -o server_2023_11_10.component.wasm --adapt wasi_snapshot_preview1.reactor.wasm

run-server: server_2023_11_10.component.wasm ; wasmtime serve server_2023_11_10.component.wasm
