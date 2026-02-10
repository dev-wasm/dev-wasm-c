# Porting HTTP Applications to WASM using wasi-http

This guide helps developers port C applications that use HTTP libraries (like libcurl, libhttp, etc.) to WebAssembly using the wasi-http API.

## Understanding wasi-http

The wasi-http API is part of the WASI (WebAssembly System Interface) specification. Unlike traditional HTTP libraries, it's designed specifically for WebAssembly's component model.

### Key Differences from Traditional HTTP Libraries

1. **Component-based**: Uses WIT (WebAssembly Interface Types) for type safety
2. **Async by nature**: Uses pollables and futures for non-blocking I/O
3. **Resource management**: Explicit ownership of handles and streams
4. **Pre-parsed URLs**: Requires authority and path to be separated

## Common Porting Patterns

### Pattern 1: Simple GET Request

**Traditional libcurl approach:**
```c
CURL *curl = curl_easy_init();
curl_easy_setopt(curl, CURLOPT_URL, "https://api.example.com/data");
curl_easy_perform(curl);
curl_easy_cleanup(curl);
```

**wasi-http equivalent (see http/main.c):**
```c
char response_buffer[65536];
wasi_http_response_t response = {
    .body = response_buffer,
    .body_max_len = sizeof(response_buffer)
};

wasi_http_request(GET, HTTPS, "api.example.com", "/data", NULL, &response);
// Use response.status_code, response.body, response.headers
free_response(&response);
```

### Pattern 2: POST with Data

**Traditional approach:**
```c
curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
curl_easy_perform(curl);
```

**wasi-http equivalent:**
```c
const char* payload = "{\"key\": \"value\"}";
wasi_http_request(POST, HTTPS, "api.example.com", "/endpoint", payload, &response);
```

### Pattern 3: Handling Response Headers

**Accessing headers:**
```c
for (int i = 0; i < response.headers.len; i++) {
    char* name = response.headers.headers[i].name;
    char* value = response.headers.headers[i].value;
    
    if (strcmp(name, "content-type") == 0) {
        printf("Content type: %s\n", value);
    }
}
```

## Helper Macros for Cleaner Code

Add these to your code for easier usage:

```c
// Initialize a response buffer
#define HTTP_RESPONSE_INIT(name, size) \\
    char name##_buffer[size]; \\
    wasi_http_response_t name = { \\
        .body = name##_buffer, \\
        .body_max_len = size \\
    }

// Usage:
HTTP_RESPONSE_INIT(resp, 32768);
wasi_http_request(GET, HTTPS, "example.com", "/api", NULL, &resp);
```

## URL Handling Strategies

Since wasi-http requires split URLs, here are strategies:

### Strategy 1: Manual Splitting
```c
// Split "https://api.github.com/repos/user/project"
const char* authority = "api.github.com";
const char* path = "/repos/user/project";
wasi_http_request(GET, HTTPS, authority, path, NULL, &response);
```

### Strategy 2: URL Builder Helper
```c
// Create a simple structure to hold split URL components
typedef struct {
    const char* authority;
    const char* path;
} url_components_t;

// For your specific application's URLs
url_components_t build_api_url(const char* endpoint) {
    url_components_t url = {
        .authority = "your-api-server.com",
        .path = endpoint
    };
    return url;
}

// Usage:
url_components_t url = build_api_url("/users/123");
wasi_http_request(GET, HTTPS, url.authority, url.path, NULL, &response);
```

## Error Handling Best Practices

The wasi_http_request function returns error codes. Handle them appropriately:

```c
int result = wasi_http_request(GET, HTTPS, host, path, NULL, &response);
switch (result) {
    case 0:
        // Success
        if (response.status_code >= 200 && response.status_code < 300) {
            // Process successful response
        } else {
            // Handle HTTP error status
            fprintf(stderr, "HTTP error: %d\\n", response.status_code);
        }
        break;
    case 1:
        fprintf(stderr, "Failed to get response\\n");
        break;
    case 2:
        fprintf(stderr, "Response indicated error\\n");
        break;
    // ... handle other error codes
    default:
        fprintf(stderr, "Unknown error: %d\\n", result);
}
```

## Memory Management

**Important**: Always call `free_response()` after using a response:

```c
wasi_http_response_t response = {/* ... */};
int result = wasi_http_request(/*...*/);

if (result == 0) {
    // Use response data
    process_data(response.body);
    
    // MUST free headers
    free_response(&response);
}
// The body buffer is owned by caller, so manage separately
```

## Multi-Request Applications

For applications making multiple requests:

```c
// Reuse the same buffer for efficiency
char shared_buffer[65536];

for (int i = 0; i < num_requests; i++) {
    wasi_http_response_t response = {
        .body = shared_buffer,
        .body_max_len = sizeof(shared_buffer)
    };
    
    wasi_http_request(GET, HTTPS, hosts[i], paths[i], NULL, &response);
    
    // Process response
    handle_response(&response);
    
    // Free headers (but buffer is reused)
    free_response(&response);
}
```

## Testing Your Port

1. **Build**: `make -C http main.wasm`
2. **Test locally**: `make -C http run`
3. **Verify**: Check that HTTP requests complete without errors

## Common Pitfalls

1. **Forgetting to free headers**: Always call `free_response()`
2. **Buffer too small**: Ensure body_max_len is sufficient for expected responses
3. **Wrong scheme**: Use HTTPS constant for https://, HTTP for http://
4. **Missing path**: Path must start with `/`, include query parameters

## Examples in This Repository

- **http/main.c**: GET, POST, and PUT requests to echo service
- **http/server.c**: HTTP server implementation
- **http/wasi_http.c**: Core implementation showing low-level wasi-http usage

## Further Reading

- WASI HTTP specification: https://github.com/WebAssembly/wasi-http
- Component Model: https://component-model.bytecodealliance.org/
- WIT format: https://component-model.bytecodealliance.org/design/wit.html
