# Experimental HTTP Client and Server Example
*NB*: this example uses an experimental `wasi-http` that incorporates an
experimental HTTP client library being developed as part of the WASI specification.
Use at your own risk, things may change in the future.

## For developers porting from other HTTP libraries

The `wasi_http_request()` function in `wasi_http.c` provides the main HTTP functionality. See `main.c` for usage examples showing GET, POST, and PUT requests. Key differences from traditional libraries:
- URLs must be split into authority (host) and path components
- Response buffer must be pre-allocated by caller
- Call `free_response()` to release header memory after use

## Building the client example
```sh
make main.wasm
```

### Running
```sh
make clean; make run
```

## Building the server example
```sh
make server.wasm
```

### Running
```sh
make clean; make run-server
```
