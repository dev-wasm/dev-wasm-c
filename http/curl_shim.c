#include "curl_shim.h"
#include "wasi_http.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/* Constants */
#define MAX_RESPONSE_SIZE (256 * 1024)
#define MAX_HEADER_LINE_SIZE 2048

/* Internal CURL handle structure */
typedef struct {
    char *url;
    char *custom_request;
    char *useragent;
    struct curl_slist *headers;
    void *write_data;
    curl_write_callback write_function;
    void *header_data;
    curl_header_callback header_function;
    const char *post_fields;
    long post_field_size;
    int http_method; /* 0=GET, 1=POST, 2=PUT, 3=HEAD, 4=custom */
    int follow_location;
    int nobody;
    
    /* Response data */
    long response_code;
    char *content_type;
    char *effective_url;
} curl_handle_t;

/* Default write callback - writes to a buffer */
static size_t default_write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    /* By default, discard data if no callback is set */
    return size * nmemb;
}

/* Parse URL into components */
static int parse_url(const char *url, char **scheme, char **authority, char **path) {
    if (!url) return -1;
    
    const char *scheme_end = strstr(url, "://");
    if (!scheme_end) return -1;
    
    size_t scheme_len = scheme_end - url;
    *scheme = malloc(scheme_len + 1);
    if (!*scheme) return -1;
    strncpy(*scheme, url, scheme_len);
    (*scheme)[scheme_len] = '\0';
    
    const char *authority_start = scheme_end + 3;
    const char *path_start = strchr(authority_start, '/');
    
    if (path_start) {
        size_t authority_len = path_start - authority_start;
        *authority = malloc(authority_len + 1);
        if (!*authority) {
            free(*scheme);
            return -1;
        }
        strncpy(*authority, authority_start, authority_len);
        (*authority)[authority_len] = '\0';
        
        *path = strdup(path_start);
    } else {
        *authority = strdup(authority_start);
        *path = strdup("/");
    }
    
    return 0;
}

/* Global init/cleanup */
CURLcode curl_global_init(long flags) {
    /* No global initialization needed for wasi-http */
    return CURLE_OK;
}

void curl_global_cleanup(void) {
    /* No global cleanup needed */
}

/* Easy interface functions */
CURL *curl_easy_init(void) {
    curl_handle_t *handle = calloc(1, sizeof(curl_handle_t));
    if (!handle) return NULL;
    
    handle->write_function = default_write_callback;
    handle->http_method = 0; /* GET by default */
    handle->response_code = 0;
    
    return (CURL *)handle;
}

void curl_easy_cleanup(CURL *curl) {
    if (!curl) return;
    
    curl_handle_t *handle = (curl_handle_t *)curl;
    
    if (handle->url) free(handle->url);
    if (handle->custom_request) free(handle->custom_request);
    if (handle->useragent) free(handle->useragent);
    if (handle->content_type) free(handle->content_type);
    if (handle->effective_url) free(handle->effective_url);
    if (handle->headers) curl_slist_free_all(handle->headers);
    
    free(handle);
}

CURLcode curl_easy_setopt(CURL *curl, CURLoption option, ...) {
    if (!curl) return CURLE_FAILED_INIT;
    
    curl_handle_t *handle = (curl_handle_t *)curl;
    va_list args;
    va_start(args, option);
    
    switch (option) {
        case CURLOPT_URL: {
            const char *url = va_arg(args, const char *);
            if (handle->url) free(handle->url);
            handle->url = url ? strdup(url) : NULL;
            break;
        }
        case CURLOPT_WRITEDATA:
            handle->write_data = va_arg(args, void *);
            break;
        case CURLOPT_WRITEFUNCTION:
            handle->write_function = va_arg(args, curl_write_callback);
            break;
        case CURLOPT_HEADERDATA:
            handle->header_data = va_arg(args, void *);
            break;
        case CURLOPT_HEADERFUNCTION:
            handle->header_function = va_arg(args, curl_header_callback);
            break;
        case CURLOPT_HTTPHEADER:
            handle->headers = va_arg(args, struct curl_slist *);
            break;
        case CURLOPT_POSTFIELDS:
            handle->post_fields = va_arg(args, const char *);
            break;
        case CURLOPT_POSTFIELDSIZE:
            handle->post_field_size = va_arg(args, long);
            break;
        case CURLOPT_POST: {
            long post = va_arg(args, long);
            if (post) handle->http_method = 1; /* POST */
            break;
        }
        case CURLOPT_HTTPGET:
            va_arg(args, long); /* consume argument */
            handle->http_method = 0; /* GET */
            break;
        case CURLOPT_PUT: {
            long put = va_arg(args, long);
            if (put) handle->http_method = 2; /* PUT */
            break;
        }
        case CURLOPT_NOBODY: {
            long nobody = va_arg(args, long);
            handle->nobody = nobody;
            if (nobody) handle->http_method = 3; /* HEAD */
            break;
        }
        case CURLOPT_CUSTOMREQUEST: {
            const char *method = va_arg(args, const char *);
            if (handle->custom_request) free(handle->custom_request);
            handle->custom_request = method ? strdup(method) : NULL;
            handle->http_method = 4; /* custom */
            break;
        }
        case CURLOPT_FOLLOWLOCATION:
            handle->follow_location = va_arg(args, long);
            break;
        case CURLOPT_USERAGENT: {
            const char *ua = va_arg(args, const char *);
            if (handle->useragent) free(handle->useragent);
            handle->useragent = ua ? strdup(ua) : NULL;
            break;
        }
        case CURLOPT_VERBOSE:
            /* Ignore verbose flag */
            va_arg(args, long);
            break;
        default:
            /* Unsupported option - ignore */
            va_arg(args, void *);
            break;
    }
    
    va_end(args);
    return CURLE_OK;
}

CURLcode curl_easy_perform(CURL *curl) {
    if (!curl) return CURLE_FAILED_INIT;
    
    curl_handle_t *handle = (curl_handle_t *)curl;
    
    if (!handle->url) return CURLE_URL_MALFORMAT;
    
    /* Parse URL */
    char *scheme = NULL, *authority = NULL, *path = NULL;
    if (parse_url(handle->url, &scheme, &authority, &path) != 0) {
        return CURLE_URL_MALFORMAT;
    }
    
    /* Store effective URL */
    if (handle->effective_url) free(handle->effective_url);
    handle->effective_url = strdup(handle->url);
    
    /* Determine scheme */
    uint8_t scheme_tag = HTTP;
    if (strcmp(scheme, "https") == 0) {
        scheme_tag = HTTPS;
    }
    
    /* Determine HTTP method */
    uint8_t method_tag = GET;
    const char *body = NULL;
    
    switch (handle->http_method) {
        case 0: /* GET */
            method_tag = GET;
            break;
        case 1: /* POST */
            method_tag = POST;
            body = handle->post_fields;
            break;
        case 2: /* PUT */
            method_tag = PUT;
            body = handle->post_fields;
            break;
        case 3: /* HEAD */
            method_tag = HEAD;
            break;
        case 4: /* Custom */
            if (handle->custom_request) {
                if (strcmp(handle->custom_request, "DELETE") == 0) {
                    method_tag = DELETE;
                } else if (strcmp(handle->custom_request, "PATCH") == 0) {
                    method_tag = WASI_HTTP_TYPES_METHOD_PATCH;
                } else if (strcmp(handle->custom_request, "OPTIONS") == 0) {
                    method_tag = OPTIONS;
                } else {
                    /* Default to GET for unknown methods */
                    method_tag = GET;
                }
            }
            break;
    }
    
    /* Allocate response buffer */
    char *response_buffer = malloc(MAX_RESPONSE_SIZE);
    if (!response_buffer) {
        free(scheme);
        free(authority);
        free(path);
        return CURLE_OUT_OF_MEMORY;
    }
    
    wasi_http_response_t response = {
        .body = response_buffer,
        .body_max_len = MAX_RESPONSE_SIZE
    };
    
    /* Make the request */
    int result = wasi_http_request(method_tag, scheme_tag, authority, path, body, &response);
    
    free(scheme);
    free(authority);
    free(path);
    
    if (result != 0) {
        free(response_buffer);
        return CURLE_COULDNT_CONNECT;
    }
    
    /* Store response code */
    handle->response_code = response.status_code;
    
    /* Call header callback if provided */
    if (handle->header_function && handle->header_data) {
        for (int i = 0; i < response.headers.len; i++) {
            char header_line[MAX_HEADER_LINE_SIZE];
            snprintf(header_line, sizeof(header_line), "%s: %s\r\n",
                     response.headers.headers[i].name,
                     response.headers.headers[i].value);
            handle->header_function(header_line, 1, strlen(header_line), handle->header_data);
            
            /* Store content-type if present */
            if (strcasecmp(response.headers.headers[i].name, "content-type") == 0) {
                if (handle->content_type) free(handle->content_type);
                handle->content_type = strdup(response.headers.headers[i].value);
            }
        }
    }
    
    /* Call write callback with body
     * Note: wasi_http returns null-terminated string responses. This means
     * binary data or responses with embedded null bytes are not fully supported
     * by the underlying wasi_http API. This is a known limitation. */
    if (handle->write_function && response.body[0] != '\0') {
        size_t body_len = strlen(response.body);
        size_t written = handle->write_function(response.body, 1, body_len, handle->write_data);
        if (written != body_len) {
            free(response_buffer);
            free_response(&response);
            return CURLE_WRITE_ERROR;
        }
    }
    
    free(response_buffer);
    free_response(&response);
    
    return CURLE_OK;
}

CURLcode curl_easy_getinfo(CURL *curl, CURLINFO info, ...) {
    if (!curl) return CURLE_FAILED_INIT;
    
    curl_handle_t *handle = (curl_handle_t *)curl;
    va_list args;
    va_start(args, info);
    
    switch (info) {
        case CURLINFO_RESPONSE_CODE: {
            long *codep = va_arg(args, long *);
            if (codep) *codep = handle->response_code;
            break;
        }
        case CURLINFO_CONTENT_TYPE: {
            char **ct = va_arg(args, char **);
            if (ct) *ct = handle->content_type;
            break;
        }
        case CURLINFO_EFFECTIVE_URL: {
            char **url = va_arg(args, char **);
            if (url) *url = handle->effective_url;
            break;
        }
        default:
            va_arg(args, void *);
            break;
    }
    
    va_end(args);
    return CURLE_OK;
}

void curl_easy_reset(CURL *curl) {
    if (!curl) return;
    
    curl_handle_t *handle = (curl_handle_t *)curl;
    
    if (handle->url) {
        free(handle->url);
        handle->url = NULL;
    }
    if (handle->custom_request) {
        free(handle->custom_request);
        handle->custom_request = NULL;
    }
    if (handle->useragent) {
        free(handle->useragent);
        handle->useragent = NULL;
    }
    if (handle->content_type) {
        free(handle->content_type);
        handle->content_type = NULL;
    }
    if (handle->effective_url) {
        free(handle->effective_url);
        handle->effective_url = NULL;
    }
    
    if (handle->headers) {
        curl_slist_free_all(handle->headers);
    }
    
    handle->headers = NULL;
    handle->write_data = NULL;
    handle->write_function = default_write_callback;
    handle->header_data = NULL;
    handle->header_function = NULL;
    handle->post_fields = NULL;
    handle->post_field_size = 0;
    handle->http_method = 0;
    handle->follow_location = 0;
    handle->nobody = 0;
    handle->response_code = 0;
}

const char *curl_easy_strerror(CURLcode code) {
    switch (code) {
        case CURLE_OK: return "No error";
        case CURLE_UNSUPPORTED_PROTOCOL: return "Unsupported protocol";
        case CURLE_FAILED_INIT: return "Failed initialization";
        case CURLE_URL_MALFORMAT: return "URL malformed";
        case CURLE_COULDNT_RESOLVE_HOST: return "Could not resolve host";
        case CURLE_COULDNT_CONNECT: return "Could not connect";
        case CURLE_HTTP_RETURNED_ERROR: return "HTTP returned error";
        case CURLE_WRITE_ERROR: return "Write error";
        case CURLE_OUT_OF_MEMORY: return "Out of memory";
        case CURLE_OPERATION_TIMEDOUT: return "Operation timed out";
        case CURLE_TOO_MANY_REDIRECTS: return "Too many redirects";
        case CURLE_GOT_NOTHING: return "Got nothing";
        case CURLE_RECV_ERROR: return "Receive error";
        case CURLE_SEND_ERROR: return "Send error";
        default: return "Unknown error";
    }
}

/* Header list management */
struct curl_slist *curl_slist_append(struct curl_slist *list, const char *string) {
    struct curl_slist *new_item = malloc(sizeof(struct curl_slist));
    if (!new_item) return list;
    
    new_item->data = strdup(string);
    if (!new_item->data) {
        free(new_item);
        return list;
    }
    new_item->next = NULL;
    
    if (!list) {
        return new_item;
    }
    
    struct curl_slist *current = list;
    while (current->next) {
        current = current->next;
    }
    current->next = new_item;
    
    return list;
}

void curl_slist_free_all(struct curl_slist *list) {
    while (list) {
        struct curl_slist *next = list->next;
        if (list->data) free(list->data);
        free(list);
        list = next;
    }
}
