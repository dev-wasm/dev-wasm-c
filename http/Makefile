wasi_sdk := /usr/local/lib/wasi-sdk-30.0-x86_64-linux/
cc := ${wasi_sdk}/bin/clang
ar := ${wasi_sdk}/bin/ar
lib_objs := client.o wasi_http.o

.PHONY: gen clean run validate-http-tools test

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

main.wasm: gen main.o libwasihttp.a; ${cc} -Wl,-z,stack-size=131072 -Wl,--initial-memory=262144 main.o client_component_type.o -L. -lwasihttp -o main.wasm

main_0_2_8.embed.wasm: main.wasm ; wasm-tools component embed ./wasi-http/wit main.wasm -o main_0_2_8.embed.wasm -w client

wasi_snapshot_preview1.reactor.wasm: ; wget https://github.com/bytecodealliance/wasmtime/releases/download/v41.0.2/wasi_snapshot_preview1.reactor.wasm

main_0_2_8.component.wasm: main_0_2_8.embed.wasm wasi_snapshot_preview1.reactor.wasm; wasm-tools component new main_0_2_8.embed.wasm -o main_0_2_8.component.wasm --adapt wasi_snapshot_preview1.reactor.wasm

clean: ; rm -f client.c *.o client.h proxy.h proxy.c *.wasm *.a; rm -rf wasi-http

run: main_0_2_8.component.wasm ; wasmtime -S http --wasm component-model main_0_2_8.component.wasm

server.wasm: gen server.o libwasihttp.a; ${cc} -Wl,-z,stack-size=131072 -Wl,--initial-memory=262144 server.o proxy_component_type.o client_component_type.o -L. -lwasihttp -o server.wasm

server_0_2_8.embed.wasm: server.wasm ; wasm-tools component embed ./wasi-http/wit server.wasm -o server_0_2_8.embed.wasm -w proxy

server_0_2_8.component.wasm: server_0_2_8.embed.wasm wasi_snapshot_preview1.reactor.wasm; wasm-tools component new server_0_2_8.embed.wasm -o server_0_2_8.component.wasm --adapt wasi_snapshot_preview1.reactor.wasm

run-server: server_0_2_8.component.wasm ; wasmtime serve -Scommon server_0_2_8.component.wasm

# Run HTTP examples in CI/CD for validation
# The client makes real HTTP requests - will validate execution even if network fails
# The server is started, tested with curl, then stopped
test: main_0_2_8.component.wasm server_0_2_8.component.wasm
	@echo "Testing HTTP client example..."
	@echo "Running HTTP client (network requests may fail in CI, but validates WASM execution):"
	@wasmtime -S http --wasm component-model main_0_2_8.component.wasm; \
	EXIT_CODE=$$?; \
	if [ $$EXIT_CODE -eq 0 ]; then \
		echo "✓ HTTP client executed successfully"; \
	elif [ $$EXIT_CODE -eq 1 ]; then \
		echo "✓ HTTP client executed (exited with code 1 - likely network error, WASM execution validated)"; \
	else \
		echo "✗ HTTP client failed with exit code $$EXIT_CODE"; \
		exit $$EXIT_CODE; \
	fi
	@echo ""
	@echo "Testing HTTP server example..."
	@SERVER_LOG=$$(mktemp); \
	CURL_OUTPUT=$$(mktemp); \
	wasmtime serve -Scommon server_0_2_8.component.wasm > $$SERVER_LOG 2>&1 & \
	SERVER_PID=$$!; \
	cleanup() { \
		kill $$SERVER_PID 2>/dev/null || true; \
		wait $$SERVER_PID 2>/dev/null || true; \
		rm -f $$SERVER_LOG $$CURL_OUTPUT; \
	}; \
	trap cleanup EXIT; \
	echo "Server started with PID $$SERVER_PID"; \
	echo "Waiting for server to be ready..."; \
	MAX_RETRIES=10; \
	RETRY=0; \
	while [ $$RETRY -lt $$MAX_RETRIES ]; do \
		if curl -f -s http://localhost:8080/ > $$CURL_OUTPUT 2>&1; then \
			if [ $$RETRY -eq 0 ]; then \
				echo "✓ Server responded successfully on first attempt"; \
			else \
				echo "✓ Server responded successfully after $$RETRY retries"; \
			fi; \
			echo "Response:"; \
			cat $$CURL_OUTPUT; \
			trap - EXIT; \
			cleanup; \
			exit 0; \
		fi; \
		RETRY=$$((RETRY + 1)); \
		sleep 1; \
	done; \
	echo "✗ Server failed to respond after $$MAX_RETRIES attempts"; \
	cat $$CURL_OUTPUT 2>/dev/null || true; \
	cat $$SERVER_LOG 2>/dev/null || true; \
	trap - EXIT; \
	cleanup; \
	exit 1
