#include "wasi_http.h"
#include <stdio.h>
#include <string.h>

/*
 * Example: Building a REST API Client
 * 
 * This example demonstrates how to structure code when porting
 * an application that needs to interact with REST APIs.
 */

// Application-specific API endpoint builder
typedef struct {
    const char* base_authority;
    char path_buffer[512];
} api_client_t;

void init_api_client(api_client_t* client, const char* authority) {
    client->base_authority = authority;
}

// Build path for specific endpoint
const char* build_endpoint(api_client_t* client, const char* resource, const char* id) {
    if (id != NULL) {
        snprintf(client->path_buffer, sizeof(client->path_buffer), "/%s/%s", resource, id);
    } else {
        snprintf(client->path_buffer, sizeof(client->path_buffer), "/%s", resource);
    }
    return client->path_buffer;
}

// Perform GET request with error handling
int api_get(api_client_t* client, const char* endpoint, wasi_http_response_t* response) {
    int result = wasi_http_request(GET, HTTPS, client->base_authority, endpoint, NULL, response);
    
    if (result != 0) {
        fprintf(stderr, "Request failed with code %d\n", result);
        return -1;
    }
    
    if (response->status_code < 200 || response->status_code >= 300) {
        fprintf(stderr, "HTTP error: %d\n", response->status_code);
        return -1;
    }
    
    return 0;
}

// Perform POST request with JSON payload
int api_post(api_client_t* client, const char* endpoint, const char* json_data, wasi_http_response_t* response) {
    int result = wasi_http_request(POST, HTTPS, client->base_authority, endpoint, json_data, response);
    
    if (result != 0) {
        fprintf(stderr, "POST request failed with code %d\n", result);
        return -1;
    }
    
    return 0;
}

int main() {
    printf("=== REST API Client Example ===\n\n");
    
    // Initialize API client for a specific service
    api_client_t api;
    init_api_client(&api, "postman-echo.com");
    
    // Example 1: GET request to retrieve data
    printf("1. Fetching data from API...\n");
    char get_buffer[16384];
    wasi_http_response_t get_resp = {
        .body = get_buffer,
        .body_max_len = sizeof(get_buffer)
    };
    
    const char* get_endpoint = build_endpoint(&api, "get", NULL);
    if (api_get(&api, get_endpoint, &get_resp) == 0) {
        printf("   Status: %d\n", get_resp.status_code);
        printf("   Response: %.200s...\n", get_resp.body);
        free_response(&get_resp);
    }
    printf("\n");
    
    // Example 2: POST request to create/update data
    printf("2. Sending data to API...\n");
    char post_buffer[16384];
    wasi_http_response_t post_resp = {
        .body = post_buffer,
        .body_max_len = sizeof(post_buffer)
    };
    
    const char* post_endpoint = build_endpoint(&api, "post", NULL);
    const char* json_payload = "{\"action\":\"create\",\"data\":{\"name\":\"test\",\"value\":42}}";
    
    if (api_post(&api, post_endpoint, json_payload, &post_resp) == 0) {
        printf("   Status: %d\n", post_resp.status_code);
        printf("   Response: %.200s...\n", post_resp.body);
        free_response(&post_resp);
    }
    printf("\n");
    
    // Example 3: Multiple requests showing resource management
    printf("3. Making multiple requests...\n");
    char multi_buffer[8192];  // Shared buffer for efficiency
    
    const char* endpoints[] = {"/get?page=1", "/get?page=2", "/get?page=3"};
    int num_endpoints = 3;
    
    for (int i = 0; i < num_endpoints; i++) {
        wasi_http_response_t resp = {
            .body = multi_buffer,
            .body_max_len = sizeof(multi_buffer)
        };
        
        int result = wasi_http_request(GET, HTTPS, api.base_authority, endpoints[i], NULL, &resp);
        if (result == 0) {
            printf("   Request %d: Status %d\n", i + 1, resp.status_code);
            free_response(&resp);
        }
    }
    
    printf("\n=== Example Complete ===\n");
    return 0;
}

// WASM component entry point
bool exports_wasi_cli_run_run() {
    return !main();
}
