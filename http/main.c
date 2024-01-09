#include "wasi_http.h"
#include <stdio.h>

void print_response(wasi_http_response_t* response) {
    printf("Status code is: %d\n", response->status_code);
    printf("Headers:\n");
    for (int i = 0; i < response->headers.len; i++) {
        printf("\t%s: %s\n", response->headers.headers[i].name, response->headers.headers[i].value);
    }
    printf("Body is:\n%s\n", response->body);
}

int main() {
    char buff[64 * 1024];
    wasi_http_response_t response = {
        .body = buff,
        .body_max_len = 64 * 1024
    };
    wasi_http_request(GET, HTTPS, "postman-echo.com", "/get?some=arg&goes=here", NULL, &response);
    print_response(&response);
    free_response(&response);

    wasi_http_request(POST, HTTPS, "postman-echo.com", "/post", "{\"foo\": \"bar\"}", &response);
    print_response(&response);
    free_response(&response);
    
    wasi_http_request(PUT, HTTP, "postman-echo.com", "/put", NULL, &response);
    print_response(&response);
    free_response(&response);
    
    return 0;
}

// The wasm component hook for the 'main'
bool exports_wasi_cli_0_2_0_rc_2023_11_10_run_run() {
    return !main();
}
