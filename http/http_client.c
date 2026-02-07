#include "http_client.h"
#include "wasi_http.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

// Internal structure for header storage
typedef struct {
    header_list_t raw_headers;
} internal_header_t;

// Parse URL into scheme, host, and path components
int parse_url(const char* full_url, url_parts_t* parts) {
    if (!full_url || !parts) {
        return 1;
    }
    
    // Initialize parts to NULL
    parts->scheme = NULL;
    parts->host = NULL;
    parts->path = NULL;
    
    const char* current = full_url;
    
    // Find scheme (http:// or https://)
    const char* scheme_end = strstr(current, "://");
    if (!scheme_end) {
        return 2;
    }
    
    size_t scheme_len = scheme_end - current;
    parts->scheme = malloc(scheme_len + 1);
    if (!parts->scheme) {
        return 3;
    }
    strncpy(parts->scheme, current, scheme_len);
    parts->scheme[scheme_len] = '\0';
    
    // Move past ://
    current = scheme_end + 3;
    
    // Find host (everything until / or end)
    const char* path_start = strchr(current, '/');
    if (!path_start) {
        // No path, just host
        parts->host = strdup(current);
        parts->path = strdup("/");
    } else {
        size_t host_len = path_start - current;
        parts->host = malloc(host_len + 1);
        if (!parts->host) {
            free(parts->scheme);
            return 4;
        }
        strncpy(parts->host, current, host_len);
        parts->host[host_len] = '\0';
        
        // Rest is path
        parts->path = strdup(path_start);
    }
    
    if (!parts->host || !parts->path) {
        free_url_parts(parts);
        return 5;
    }
    
    return 0;
}

void free_url_parts(url_parts_t* parts) {
    if (parts) {
        if (parts->scheme) {
            free(parts->scheme);
            parts->scheme = NULL;
        }
        if (parts->host) {
            free(parts->host);
            parts->host = NULL;
        }
        if (parts->path) {
            free(parts->path);
            parts->path = NULL;
        }
    }
}

// Internal helper to perform HTTP request
static http_response_t* perform_request(uint8_t method, const char* url, const char* data) {
    url_parts_t url_info;
    if (parse_url(url, &url_info) != 0) {
        return NULL;
    }
    
    // Determine scheme tag
    uint8_t scheme_tag = HTTP;
    if (strcmp(url_info.scheme, "https") == 0) {
        scheme_tag = HTTPS;
    }
    
    // Allocate response buffer
    char* buffer = malloc(256 * 1024);  // 256KB buffer
    if (!buffer) {
        free_url_parts(&url_info);
        return NULL;
    }
    
    wasi_http_response_t raw_response = {
        .body = buffer,
        .body_max_len = 256 * 1024
    };
    
    int result = wasi_http_request(method, scheme_tag, url_info.host, 
                                    url_info.path, data, &raw_response);
    
    free_url_parts(&url_info);
    
    if (result != 0) {
        printf("wasi_http_request failed with code: %d\n", result);
        free(buffer);
        return NULL;
    }
    
    // Create simplified response
    http_response_t* response = malloc(sizeof(http_response_t));
    if (!response) {
        free(buffer);
        free_response(&raw_response);
        return NULL;
    }
    
    response->status = raw_response.status_code;
    response->body_data = buffer;
    response->body_size = strlen(buffer);
    
    // Store headers in internal structure
    internal_header_t* headers = malloc(sizeof(internal_header_t));
    if (headers) {
        headers->raw_headers = raw_response.headers;
        response->internal_headers = headers;
    } else {
        response->internal_headers = NULL;
        free_response(&raw_response);
    }
    
    return response;
}

http_response_t* http_get(const char* url) {
    return perform_request(GET, url, NULL);
}

http_response_t* http_post(const char* url, const char* data) {
    return perform_request(POST, url, data);
}

http_response_t* http_put(const char* url, const char* data) {
    return perform_request(PUT, url, data);
}

http_response_t* http_delete(const char* url) {
    return perform_request(DELETE, url, NULL);
}

http_response_t* http_post_with_type(const char* url, const char* data, const char* content_type) {
    // For now, use the same implementation
    // Future enhancement: support custom content-type
    return perform_request(POST, url, data);
}

const char* get_response_header(http_response_t* response, const char* header_name) {
    if (!response || !response->internal_headers || !header_name) {
        return NULL;
    }
    
    internal_header_t* headers = (internal_header_t*)response->internal_headers;
    
    for (int i = 0; i < headers->raw_headers.len; i++) {
        // Case-insensitive comparison
        if (strcasecmp(headers->raw_headers.headers[i].name, header_name) == 0) {
            return headers->raw_headers.headers[i].value;
        }
    }
    
    return NULL;
}

void free_http_response(http_response_t* response) {
    if (!response) {
        return;
    }
    
    if (response->body_data) {
        free(response->body_data);
    }
    
    if (response->internal_headers) {
        internal_header_t* headers = (internal_header_t*)response->internal_headers;
        
        // Free the raw headers properly
        wasi_http_response_t temp = {
            .headers = headers->raw_headers
        };
        free_response(&temp);
        
        free(headers);
    }
    
    free(response);
}
