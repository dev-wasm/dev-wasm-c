#include "curl_shim.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Example demonstrating libcurl API compatibility
 * This code uses standard libcurl APIs and works with the shim
 */

/* Write callback that stores response in a string */
typedef struct {
    char *data;
    size_t size;
} response_buffer_t;

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t total_size = size * nmemb;
    response_buffer_t *buf = (response_buffer_t *)userdata;
    
    char *new_data = realloc(buf->data, buf->size + total_size + 1);
    if (!new_data) {
        return 0; /* Out of memory */
    }
    
    buf->data = new_data;
    memcpy(buf->data + buf->size, ptr, total_size);
    buf->size += total_size;
    buf->data[buf->size] = '\0';
    
    return total_size;
}

int main() {
    CURLcode res;
    
    printf("=== libcurl-compatible API Demo ===\n\n");
    
    /* Initialize curl globally */
    curl_global_init(0);
    
    /* Example 1: Simple GET request */
    printf("1. Simple GET request:\n");
    CURL *curl1 = curl_easy_init();
    if (curl1) {
        response_buffer_t buffer1 = {0};
        
        curl_easy_setopt(curl1, CURLOPT_URL, "https://postman-echo.com/get?foo=bar");
        curl_easy_setopt(curl1, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl1, CURLOPT_WRITEDATA, &buffer1);
        
        res = curl_easy_perform(curl1);
        
        if (res == CURLE_OK) {
            long response_code;
            curl_easy_getinfo(curl1, CURLINFO_RESPONSE_CODE, &response_code);
            printf("   Status: %ld\n", response_code);
            printf("   Response: %.200s...\n", buffer1.data ? buffer1.data : "");
        } else {
            printf("   Error: %s\n", curl_easy_strerror(res));
        }
        
        if (buffer1.data) free(buffer1.data);
        curl_easy_cleanup(curl1);
    }
    printf("\n");
    
    /* Example 2: POST request with data */
    printf("2. POST request with JSON:\n");
    CURL *curl2 = curl_easy_init();
    if (curl2) {
        response_buffer_t buffer2 = {0};
        const char *json_data = "{\"message\": \"Hello from libcurl shim\"}";
        
        curl_easy_setopt(curl2, CURLOPT_URL, "https://postman-echo.com/post");
        curl_easy_setopt(curl2, CURLOPT_POST, 1L);
        curl_easy_setopt(curl2, CURLOPT_POSTFIELDS, json_data);
        curl_easy_setopt(curl2, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl2, CURLOPT_WRITEDATA, &buffer2);
        
        res = curl_easy_perform(curl2);
        
        if (res == CURLE_OK) {
            long response_code;
            curl_easy_getinfo(curl2, CURLINFO_RESPONSE_CODE, &response_code);
            printf("   Status: %ld\n", response_code);
            printf("   Response: %.200s...\n", buffer2.data ? buffer2.data : "");
        } else {
            printf("   Error: %s\n", curl_easy_strerror(res));
        }
        
        if (buffer2.data) free(buffer2.data);
        curl_easy_cleanup(curl2);
    }
    printf("\n");
    
    /* Example 3: Custom HTTP method */
    printf("3. PUT request:\n");
    CURL *curl3 = curl_easy_init();
    if (curl3) {
        response_buffer_t buffer3 = {0};
        const char *put_data = "{\"updated\": true}";
        
        curl_easy_setopt(curl3, CURLOPT_URL, "http://postman-echo.com/put");
        curl_easy_setopt(curl3, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl3, CURLOPT_POSTFIELDS, put_data);
        curl_easy_setopt(curl3, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl3, CURLOPT_WRITEDATA, &buffer3);
        
        res = curl_easy_perform(curl3);
        
        if (res == CURLE_OK) {
            long response_code;
            char *content_type = NULL;
            curl_easy_getinfo(curl3, CURLINFO_RESPONSE_CODE, &response_code);
            curl_easy_getinfo(curl3, CURLINFO_CONTENT_TYPE, &content_type);
            printf("   Status: %ld\n", response_code);
            if (content_type) {
                printf("   Content-Type: %s\n", content_type);
            }
        } else {
            printf("   Error: %s\n", curl_easy_strerror(res));
        }
        
        if (buffer3.data) free(buffer3.data);
        curl_easy_cleanup(curl3);
    }
    printf("\n");
    
    /* Cleanup */
    curl_global_cleanup();
    
    printf("=== Demo Complete ===\n");
    return 0;
}

/* WASM component entry point */
bool exports_wasi_cli_run_run() {
    return !main();
}
