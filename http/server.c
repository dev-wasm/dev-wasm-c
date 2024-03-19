#include "wasi_http.h"

#include <stdio.h>

void handler_fn(wasi_http_request_t *req, wasi_http_response_t *res) {
    res->status_code = 200;
    sprintf(res->body, "Hello world!\nPath is %s\nAuthority is %s\n", req->path_query, req->authority);

    header_t headers [] = {
        {
            .name = "Content-type",
            .value = "text/plain"
        },
        {
            .name = "Server",
            .value = "C/WASM"
        }
    };
    res->headers.headers = (header_t*) &headers;
    res->headers.len = 2;
}

int main() {
    handler = handler_fn;
}

// This is hack and doesn't actually do anything for this server 
bool exports_wasi_cli_run_run() {
    return !main();
}
