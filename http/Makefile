wasi_sdk := /usr/local/lib/wasi-sdk-30.0-x86_64-linux/
cc := ${wasi_sdk}/bin/clang
ar := ${wasi_sdk}/bin/ar
lib_objs := client.o wasi_http.o

.phony: gen clean run validate-http-tools

default: main.wasm

validate-http-tools:
	@echo "Validating HTTP toolchain..."
	@test -d ${wasi_sdk} || (echo "ERROR: wasi-sdk not found" && exit 1)
	@wit-bindgen --version > /dev/null
	@wasm-tools --version > /dev/null
	@wasmtime --version > /dev/null
	@echo "HTTP toolchain validation complete!"


# Necessary b/c client.c doesn't exist during a make from clean.
client.o : client.c
	${cc} -c client.c -o client.o

%.o : %.c
	@echo "Compiling c file into o file"
	${cc} -c $< -o $@

libwasihttp.a: ${lib_objs}
	${ar} cr libwasihttp.a ${lib_objs}

wasi-http: ; git clone https://github.com/WebAssembly/wasi-http; cd wasi-http; git checkout v0.2.8; cp ../client.wit wit/client.wit; cd ../

gen: wasi-http
	wit-bindgen c ./wasi-http/wit -w client
	wit-bindgen c ./wasi-http/wit -w proxy

main.wasm: gen main.o libwasihttp.a; ${cc} main.o client_component_type.o -L. -lwasihttp -o main.wasm

main_0_2_8.embed.wasm: main.wasm ; wasm-tools component embed ./wasi-http/wit main.wasm -o main_0_2_8.embed.wasm -w client

wasi_snapshot_preview1.reactor.wasm: ; wget https://github.com/bytecodealliance/wasmtime/releases/download/v41.0.2/wasi_snapshot_preview1.reactor.wasm

main_0_2_8.component.wasm: main_0_2_8.embed.wasm wasi_snapshot_preview1.reactor.wasm; wasm-tools component new main_0_2_8.embed.wasm -o main_0_2_8.component.wasm --adapt wasi_snapshot_preview1.reactor.wasm

clean: ; rm -f client.c *.o client.h proxy.h proxy.c *.wasm *.a; rm -rf wasi-http

run: main_0_2_8.component.wasm ; wasmtime -S http --wasm component-model main_0_2_8.component.wasm

server.wasm: gen server.o libwasihttp.a; ${cc} server.o proxy_component_type.o client_component_type.o -L. -lwasihttp -o server.wasm

server_0_2_8.embed.wasm: server.wasm ; wasm-tools component embed ./wasi-http/wit server.wasm -o server_0_2_8.embed.wasm -w proxy

server_0_2_8.component.wasm: server_0_2_8.embed.wasm wasi_snapshot_preview1.reactor.wasm; wasm-tools component new server_0_2_8.embed.wasm -o server_0_2_8.component.wasm --adapt wasi_snapshot_preview1.reactor.wasm

run-server: server_0_2_8.component.wasm ; wasmtime serve -Scommon server_0_2_8.component.wasm

# REST API example demonstrating practical HTTP client patterns
rest_api_example.wasm: gen rest_api_example.o libwasihttp.a; ${cc} rest_api_example.o client_component_type.o -L. -lwasihttp -o rest_api_example.wasm

rest_api_example_0_2_8.embed.wasm: rest_api_example.wasm ; wasm-tools component embed ./wasi-http/wit rest_api_example.wasm -o rest_api_example_0_2_8.embed.wasm -w client

rest_api_example_0_2_8.component.wasm: rest_api_example_0_2_8.embed.wasm wasi_snapshot_preview1.reactor.wasm; wasm-tools component new rest_api_example_0_2_8.embed.wasm -o rest_api_example_0_2_8.component.wasm --adapt wasi_snapshot_preview1.reactor.wasm

run-rest-example: rest_api_example_0_2_8.component.wasm ; wasmtime -S http --wasm component-model rest_api_example_0_2_8.component.wasm
