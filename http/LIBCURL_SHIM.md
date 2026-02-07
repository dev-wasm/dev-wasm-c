# libcurl API Shim for wasi-http

This directory contains a libcurl-compatible API shim that allows C applications using libcurl to be ported to WebAssembly with minimal changes.

## Overview

The `curl_shim.h` and `curl_shim.c` files implement the most commonly used libcurl "easy" interface functions in terms of the underlying wasi-http library. This allows existing code that uses libcurl to work in WASM environments without rewriting HTTP calls.

## Supported libcurl Functions

### Initialization and Cleanup
- `curl_global_init()` - Initialize curl (no-op in shim)
- `curl_global_cleanup()` - Cleanup curl (no-op in shim)
- `curl_easy_init()` - Initialize an easy handle
- `curl_easy_cleanup()` - Cleanup an easy handle
- `curl_easy_reset()` - Reset an easy handle to default state

### Configuration
- `curl_easy_setopt()` - Set options on a handle

### Supported Options
- `CURLOPT_URL` - Set the URL
- `CURLOPT_WRITEDATA` - Set data pointer for write callback
- `CURLOPT_WRITEFUNCTION` - Set write callback function
- `CURLOPT_HEADERDATA` - Set data pointer for header callback
- `CURLOPT_HEADERFUNCTION` - Set header callback function
- `CURLOPT_HTTPHEADER` - Set custom headers (stored but not yet used)
- `CURLOPT_POST` - Enable POST request
- `CURLOPT_POSTFIELDS` - Set POST data
- `CURLOPT_POSTFIELDSIZE` - Set POST data size
- `CURLOPT_HTTPGET` - Force GET request
- `CURLOPT_PUT` - Enable PUT request
- `CURLOPT_NOBODY` - Do HEAD request
- `CURLOPT_CUSTOMREQUEST` - Set custom HTTP method
- `CURLOPT_USERAGENT` - Set user agent (stored but not yet used)
- `CURLOPT_FOLLOWLOCATION` - Follow redirects (stored but not yet implemented)
- `CURLOPT_VERBOSE` - Enable verbose output (ignored)

### Execution
- `curl_easy_perform()` - Perform the request

### Information Retrieval
- `curl_easy_getinfo()` - Get information about the transfer

### Supported Info Types
- `CURLINFO_RESPONSE_CODE` - Get HTTP response code
- `CURLINFO_CONTENT_TYPE` - Get content type header
- `CURLINFO_EFFECTIVE_URL` - Get the effective URL

### Utilities
- `curl_easy_strerror()` - Get error string for a CURLcode
- `curl_slist_append()` - Add string to a list
- `curl_slist_free_all()` - Free a string list

## Usage Example

```c
#include "curl_shim.h"
#include <stdio.h>
#include <string.h>

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    // Handle response data
    printf("Received %zu bytes\n", size * nmemb);
    return size * nmemb;
}

int main() {
    CURL *curl;
    CURLcode res;
    
    curl_global_init(0);
    curl = curl_easy_init();
    
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://example.com/api");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        
        res = curl_easy_perform(curl);
        
        if (res == CURLE_OK) {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            printf("Status: %ld\n", response_code);
        } else {
            fprintf(stderr, "Error: %s\n", curl_easy_strerror(res));
        }
        
        curl_easy_cleanup(curl);
    }
    
    curl_global_cleanup();
    return 0;
}
```

## Building Applications

The curl shim is automatically included in `libwasihttp.a`. To build an application:

```bash
make curl_example.wasm
```

To run:

```bash
make run-curl-example
```

Or manually:

```bash
wasmtime -S http --wasm component-model curl_example_0_2_8.component.wasm
```

## Limitations

### Current Limitations
- Custom headers are parsed but not yet sent with requests
- User-agent setting is stored but not used
- Follow redirects is not implemented
- SSL certificate verification options are not supported
- Proxy settings are not supported
- Upload/download progress callbacks are not supported
- Multi-interface is not implemented

### Design Limitations
- Response bodies are limited to 256KB
- Only synchronous operations (no async/multi interface)
- Some advanced libcurl features cannot be mapped to wasi-http

## Porting Your Code

To port existing libcurl code:

1. Replace `#include <curl/curl.h>` with `#include "curl_shim.h"`
2. Build with the provided Makefile targets
3. Most common curl easy interface code should work unchanged

Example:

```c
// Before (libcurl)
#include <curl/curl.h>

// After (curl shim)
#include "curl_shim.h"

// Rest of code remains the same
```

## Implementation Notes

- The shim internally uses `wasi_http_request()` for all HTTP operations
- URLs are parsed into scheme, authority, and path components
- Response buffers are allocated dynamically (256KB)
- Error codes are mapped to appropriate CURLcode values
- Memory management follows libcurl conventions

## Examples

See `curl_example.c` for a complete working example demonstrating:
- Simple GET requests
- POST requests with JSON data
- Custom HTTP methods (PUT, DELETE)
- Response code and header retrieval
- Write callbacks for response handling

## Testing

The example can be tested with:

```bash
make clean
make run-curl-example
```

This will demonstrate GET, POST, and PUT requests using the libcurl-compatible API.
