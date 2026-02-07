#ifndef __HTTP_CLIENT_H__
#define __HTTP_CLIENT_H__

#include <stdint.h>
#include <stdbool.h>

// Simplified HTTP response with automatic memory management
typedef struct {
    uint16_t status;
    char* body_data;
    uint32_t body_size;
    void* internal_headers;  // Opaque pointer to header storage
} http_response_t;

// URL information structure
typedef struct {
    char* scheme;      // "http" or "https"
    char* host;        // hostname/authority
    char* path;        // path and query string
} url_parts_t;

// Parse a full URL into components
// Returns 0 on success, non-zero on error
int parse_url(const char* full_url, url_parts_t* parts);

// Free URL parts
void free_url_parts(url_parts_t* parts);

// Simplified HTTP methods - accept full URLs
http_response_t* http_get(const char* url);
http_response_t* http_post(const char* url, const char* data);
http_response_t* http_put(const char* url, const char* data);
http_response_t* http_delete(const char* url);

// Custom request with specified content type
http_response_t* http_post_with_type(const char* url, const char* data, const char* content_type);

// Get a specific header value from response
const char* get_response_header(http_response_t* response, const char* header_name);

// Free all memory associated with a response
void free_http_response(http_response_t* response);

#endif // __HTTP_CLIENT_H__
