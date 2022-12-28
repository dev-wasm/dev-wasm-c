#include "req.h"
#include <string.h>
#include <stdio.h>

// TODO: turn this into a real client library (maybe)
typedef struct response_t {
    HttpError err;
    uint16_t statusCode;
} HttpResponse;

HttpResponse get(const char* url, const char* headers, char* body_buffer, size_t body_buffer_length) {
    uint16_t code;
    ResponseHandle handle;
    HttpError err = req(url, strlen(url), "GET", 3, headers, strlen(headers), "", 0, &code, &handle);
    if (err != SUCCESS) {
        HttpResponse response = {err, 0};
        return response;
    }
    size_t written;
    err = bodyRead(handle, body_buffer, body_buffer_length, &written);
    if (err != SUCCESS) {
        HttpResponse response = {err, 0};
        return response;
    }
    if (written < body_buffer_length) {
        body_buffer[written] = 0;
    }
    close(handle);
    HttpResponse response = {SUCCESS, code};
    return response;
}

int main() {
    const char* url =  "https://postman-echo.com/get";
    const char* headers = "Content-type: text/html\nUser-agent: wasm32-wasi-http";
    size_t length = 1024 * 1024;
    char* buffer = (char*) malloc(length);
    HttpResponse resp = get(url, headers, buffer, length);

    if (resp.err != SUCCESS) {
        printf("Request Failed: (%d)\n", resp.err);
    } else {
        printf("Request succeeded: %d\n", resp.statusCode);
        printf("%s\n", buffer);
    }
    free(buffer);
}