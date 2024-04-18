#include "curl.h"
#include <stdarg.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "../../wasi_http.h"

CURL* curl_easy_init() {
    CURL* res = malloc(sizeof(CURL));
    return res;
}

CURLcode curl_easy_setopt(CURL *handle, CURLoption option, ...) {
    va_list ap;
    va_start(ap, option);
    switch (option) {
        case CURLOPT_URL:
            handle->url = strdup(va_arg(ap, char*));
            break;
        default:
            return CURLE_UNKNOWN_OPTION;
    }
    va_end(ap);
    return CURLE_OK;
}

CURLcode curl_easy_perform(CURL *easy_handle) {
    wasi_http_response_t resp;
    const char* authority = "example.com";
    const char* path_query = "";
    wasi_http_request(WASI_HTTP_TYPES_METHOD_GET, WASI_HTTP_TYPES_SCHEME_HTTPS, authority, path_query, NULL, &resp);

    printf("Accessing URL: %s\n", easy_handle->url);
    return CURLE_OK;
}

void curl_easy_cleanup(CURL *handle) {
    free(handle->url);
    free(handle);
}

const char* curl_easy_strerror(CURLcode res) {
    return "An unknown error occurred!";
}