#include "http_client.h"
#include <stdio.h>
#include <string.h>

int main() {
    printf("=== Simplified HTTP Client Demo ===\n\n");
    
    // Example 1: Simple GET request
    printf("1. GET Request:\n");
    http_response_t* resp1 = http_get("https://postman-echo.com/get?foo=bar");
    if (resp1) {
        printf("   Status: %d\n", resp1->status);
        printf("   Body size: %u bytes\n", resp1->body_size);
        
        const char* content_type = get_response_header(resp1, "content-type");
        if (content_type) {
            printf("   Content-Type: %s\n", content_type);
        }
        
        printf("   Body preview: %.200s...\n\n", resp1->body_data);
        free_http_response(resp1);
    } else {
        printf("   Request failed\n\n");
    }
    
    // Example 2: POST request with JSON data
    printf("2. POST Request with JSON:\n");
    const char* json_data = "{\"message\": \"Hello from WASM\", \"value\": 42}";
    http_response_t* resp2 = http_post("https://postman-echo.com/post", json_data);
    if (resp2) {
        printf("   Status: %d\n", resp2->status);
        printf("   Body preview: %.200s...\n\n", resp2->body_data);
        free_http_response(resp2);
    } else {
        printf("   Request failed\n\n");
    }
    
    // Example 3: PUT request
    printf("3. PUT Request:\n");
    http_response_t* resp3 = http_put("http://postman-echo.com/put", "{\"updated\": true}");
    if (resp3) {
        printf("   Status: %d\n", resp3->status);
        
        // Show how to iterate headers by looking for specific ones
        const char* server = get_response_header(resp3, "server");
        if (server) {
            printf("   Server: %s\n", server);
        }
        
        printf("\n");
        free_http_response(resp3);
    } else {
        printf("   Request failed\n\n");
    }
    
    // Example 4: URL parsing demonstration
    printf("4. URL Parsing Example:\n");
    url_parts_t parts;
    const char* test_url = "https://example.com:8080/api/v1/users?active=true";
    if (parse_url(test_url, &parts) == 0) {
        printf("   URL: %s\n", test_url);
        printf("   Scheme: %s\n", parts.scheme);
        printf("   Host: %s\n", parts.host);
        printf("   Path: %s\n", parts.path);
        free_url_parts(&parts);
    }
    
    printf("\n=== Demo Complete ===\n");
    return 0;
}

// WASM component hook
bool exports_wasi_cli_run_run() {
    return !main();
}
