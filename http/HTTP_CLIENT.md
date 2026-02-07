# HTTP Client Wrapper

This directory contains a simplified HTTP client wrapper built on top of the wasi-http library, making it easier to port C applications that need HTTP functionality.

## Overview

The HTTP client wrapper provides a simplified API that:
- Accepts full URLs (no need to split into scheme/authority/path)
- Handles memory management automatically
- Provides convenience functions for common HTTP methods
- Offers header lookup by name
- Simplifies response handling

## Files

- **http_client.h** - Header file with the simplified API
- **http_client.c** - Implementation wrapping wasi_http
- **client_example.c** - Example demonstrating the simplified API

## API Functions

### URL Parsing
```c
int parse_url(const char* full_url, url_parts_t* parts);
void free_url_parts(url_parts_t* parts);
```

Parse a full URL like "https://example.com/path?query=value" into scheme, host, and path components.

### HTTP Methods
```c
http_response_t* http_get(const char* url);
http_response_t* http_post(const char* url, const char* data);
http_response_t* http_put(const char* url, const char* data);
http_response_t* http_delete(const char* url);
```

Perform HTTP requests with a single function call. Returns NULL on error.

### Response Handling
```c
const char* get_response_header(http_response_t* response, const char* header_name);
void free_http_response(http_response_t* response);
```

Access response data easily. The response structure contains:
- `status` - HTTP status code
- `body_data` - Response body as a null-terminated string
- `body_size` - Length of response body

## Usage Example

```c
#include "http_client.h"
#include <stdio.h>

int main() {
    // Make a GET request
    http_response_t* resp = http_get("https://api.example.com/data");
    if (resp) {
        printf("Status: %d\n", resp->status);
        printf("Body: %s\n", resp->body_data);
        
        // Get a specific header
        const char* content_type = get_response_header(resp, "content-type");
        if (content_type) {
            printf("Content-Type: %s\n", content_type);
        }
        
        // Clean up
        free_http_response(resp);
    }
    
    return 0;
}
```

## Building

The http_client library is included in `libwasihttp.a` automatically. To build an application using it:

```bash
make client_example_0_2_8.component.wasm
```

To run:

```bash
make run-client-example
```

Or manually:

```bash
wasmtime -S http --wasm component-model client_example_0_2_8.component.wasm
```

## Comparison with Original API

### Before (wasi_http):
```c
char buff[64 * 1024];
wasi_http_response_t response = {
    .body = buff,
    .body_max_len = 64 * 1024
};
wasi_http_request(GET, HTTPS, "example.com", "/api/data", NULL, &response);
// Use response
free_response(&response);
```

### After (http_client):
```c
http_response_t* resp = http_get("https://example.com/api/data");
// Use resp->body_data, resp->status
free_http_response(resp);
```

## Features

- **Automatic Buffer Management**: No need to pre-allocate buffers
- **Full URL Support**: Pass complete URLs instead of parsing them manually
- **Header Lookup**: Find headers by name with case-insensitive comparison
- **Unified Cleanup**: Single function to free all response resources
- **Error Handling**: NULL return on errors, debug output available

## Limitations

- Currently uses a fixed 256KB buffer for responses
- Content-Type is hardcoded in the underlying wasi_http implementation
- Synchronous only (no async support)

## Future Enhancements

- Custom content-type support via http_post_with_type()
- Streaming large responses
- Custom headers for requests
- Timeout configuration
- Retry logic
