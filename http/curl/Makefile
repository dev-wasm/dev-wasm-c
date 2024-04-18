wasi_sdk := /usr/local/lib/wasi-sdk-20.0
cc := ${wasi_sdk}/bin/clang
ar := ${wasi_sdk}/bin/ar
lib_objs := client.o wasi_http.o

.phony: gen clean run

default: simple.wasm

%.o : %.c
	@echo "Compiling c file into o file"
	${cc} -c $< -o $@ -I.

simple.wasm: simple.o
	${cc} -o simple.wasm simple.o curl/curl.o -lwasihttp -L../

simple_0_2_0.embed.wasm: simple.wasm ; wasm-tools component embed ../wasi-http/wit simple.wasm -o simple_0_2_0.embed.wasm -w client

wasi_snapshot_preview1.reactor.wasm: ; wget https://github.com/bytecodealliance/wasmtime/releases/download/v18.0.2/wasi_snapshot_preview1.reactor.wasm

simple_0_2_0.component.wasm: simple_0_2_0.embed.wasm wasi_snapshot_preview1.reactor.wasm; wasm-tools component new simple_0_2_0.embed.wasm -o simple_0_2_0.component.wasm --adapt wasi_snapshot_preview1.reactor.wasm

clean: ; rm -f client.c *.o client.h proxy.h proxy.c *.wasm *.a; rm -rf wasi-http

run: simple_0_2_0.component.wasm ; wasmtime -S http --wasm component-model simple_0_2_0.component.wasm
