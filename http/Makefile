wasi_sdk := /usr/local/lib/wasi-sdk-22.0
cc := ${wasi_sdk}/bin/clang
ar := ${wasi_sdk}/bin/ar
lib_objs := client.o wasi_http.o

.phony: gen clean run

default: main.wasm


# Necessary b/c client.c doesn't exist during a make from clean.
client.o : client.c
	${cc} -c client.c -o client.o

%.o : %.c
	@echo "Compiling c file into o file"
	${cc} -c $< -o $@

libwasihttp.a: ${lib_objs}
	${ar} cr libwasihttp.a ${lib_objs}

wasi-http: ; git clone https://github.com/WebAssembly/wasi-http; cd wasi-http; git checkout v0.2.0; cp ../client.wit wit/client.wit; cd ../

gen: wasi-http ; wit-bindgen c ./wasi-http/wit -w client

main.wasm: gen main.o libwasihttp.a; ${cc} main.o -L. -lwasihttp -o main.wasm

main_0_2_0.embed.wasm: main.wasm ; wasm-tools component embed ./wasi-http/wit main.wasm -o main_0_2_0.embed.wasm -w client

wasi_snapshot_preview1.reactor.wasm: ; wget https://github.com/bytecodealliance/wasmtime/releases/download/v18.0.2/wasi_snapshot_preview1.reactor.wasm

main_0_2_0.component.wasm: main_0_2_0.embed.wasm wasi_snapshot_preview1.reactor.wasm; wasm-tools component new main_0_2_0.embed.wasm -o main_0_2_0.component.wasm --adapt wasi_snapshot_preview1.reactor.wasm

clean: ; rm -f client.c *.o client.h proxy.h proxy.c *.wasm *.a; rm -rf wasi-http

run: main_0_2_0.component.wasm ; wasmtime -S http --wasm component-model main_0_2_0.component.wasm

server.wasm: gen server.o libwasihttp.a; ${cc} server.o -L. -lwasihttp -o server.wasm

server_0_2_0.embed.wasm: server.wasm ; wasm-tools component embed ./wasi-http/wit server.wasm -o server_0_2_0.embed.wasm -w proxy

server_0_2_0.component.wasm: server_0_2_0.embed.wasm wasi_snapshot_preview1.reactor.wasm; wasm-tools component new server_0_2_0.embed.wasm -o server_0_2_0.component.wasm --adapt wasi_snapshot_preview1.reactor.wasm

run-server: server_0_2_0.component.wasm ; wasmtime serve -Scommon server_0_2_0.component.wasm
