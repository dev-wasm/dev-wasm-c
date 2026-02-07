#include "http_client.h"
#include <stdio.h>
#include <string.h>

// Test URL parsing functionality without network access
int main() {
    printf("=== HTTP Client URL Parsing Tests ===\n\n");
    
    const char* test_urls[] = {
        "https://api.github.com/repos/owner/repo",
        "http://localhost:8080/api/v1/users",
        "https://example.com/path",
        "http://192.168.1.1:3000/status?debug=true",
        "https://subdomain.example.com:443/api/endpoint?key=value&foo=bar"
    };
    
    int num_tests = sizeof(test_urls) / sizeof(test_urls[0]);
    int passed = 0;
    
    for (int i = 0; i < num_tests; i++) {
        printf("Test %d: %s\n", i + 1, test_urls[i]);
        
        url_parts_t parts;
        int result = parse_url(test_urls[i], &parts);
        
        if (result == 0) {
            printf("  ✓ Parsed successfully\n");
            printf("    Scheme: %s\n", parts.scheme);
            printf("    Host: %s\n", parts.host);
            printf("    Path: %s\n", parts.path);
            
            // Verify scheme is extracted correctly
            if ((strstr(test_urls[i], "https://") && strcmp(parts.scheme, "https") == 0) ||
                (strstr(test_urls[i], "http://") && strcmp(parts.scheme, "http") == 0)) {
                passed++;
            } else {
                printf("  ✗ Scheme mismatch!\n");
            }
            
            free_url_parts(&parts);
        } else {
            printf("  ✗ Parse failed with error code %d\n", result);
        }
        printf("\n");
    }
    
    printf("=== Results: %d/%d tests passed ===\n", passed, num_tests);
    return (passed == num_tests) ? 0 : 1;
}

// WASM component hook
bool exports_wasi_cli_run_run() {
    return !main();
}
