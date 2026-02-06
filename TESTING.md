# Testing the Devcontainer

This document describes how to test and validate the devcontainer setup.

## Overview

The devcontainer is configured with:
- **wasmtime** v41.0.2 - WebAssembly runtime
- **wasi-sdk** 30.0 - WASI-enabled C/C++ toolchain
- **wit-bindgen** 0.52.0 - WIT interface code generator
- **wasm-tools** 1.244.0 - WebAssembly component tools

## Quick Validation

To quickly validate your devcontainer environment:

```bash
make validate-environment
```

This will check:
- wasi-sdk-30.0 installation at `/usr/local/lib/wasi-sdk-30.0-x86_64-linux/`
- wasmtime version 41.0.2
- wit-bindgen availability
- wasm-tools availability

## Full Verification

Run the complete verification suite:

```bash
make verify
```

This performs:
1. Environment validation
2. Build C and C++ examples
3. Execute WASM modules with wasmtime
4. Verify file I/O operations

## Testing Individual Components

### Test Basic C Compilation

```bash
make main.wasm
wasmtime --dir . main.wasm
```

Expected: Creates test.txt and test2.txt files.

### Test Basic C++ Compilation

```bash
make main-cc.wasm
wasmtime --dir . main-cc.wasm
```

Expected: Creates test-cc.txt and test-cc-2.txt files.

### Test HTTP Toolchain

```bash
cd http
make validate-http-tools
```

This validates the HTTP-specific toolchain components.

### Test HTTP Client Build

```bash
cd http
make clean
make main_0_2_8.component.wasm
```

Expected: Successfully builds HTTP client component.

### Test HTTP Server Build

```bash
cd http
make clean
make server_0_2_8.component.wasm
```

Expected: Successfully builds HTTP server component.

## Continuous Integration

The GitHub Actions workflow (`.github/workflows/c-cpp.yml`) automatically:
1. Installs all required tools at specified versions
2. Validates the environment
3. Builds all examples
4. Runs verification tests

The workflow runs on:
- Pull requests to main
- Pushes to main
- Manual workflow dispatch

## Troubleshooting

### wasi-sdk not found

Ensure the devcontainer feature installed wasi-sdk-30.0:
```bash
ls /usr/local/lib/wasi-sdk-30.0-x86_64-linux/
```

### wasmtime version mismatch

Check your wasmtime version:
```bash
wasmtime --version
```

Should contain: `41.0.2`

### wit-bindgen or wasm-tools missing

Verify installation:
```bash
which wit-bindgen
which wasm-tools
```

Both should be in your PATH.

## Version Updates

When updating versions:
1. Update `.devcontainer/devcontainer.json` with new version numbers
2. Update `Makefile` to reference new wasi-sdk path
3. Update `http/Makefile` similarly
4. Update `.github/workflows/c-cpp.yml` to install new versions
5. Run full verification: `make verify`
6. Test HTTP examples: `cd http && make clean && make main.wasm server.wasm`
