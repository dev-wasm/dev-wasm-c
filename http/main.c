#include "proxy.h"
#include <stdio.h>

void exports_wasi_http_incoming_handler_handle(uint32_t arg, uint32_t arg0) {

}

int request(uint8_t method_tag, uint8_t scheme_tag, const char * authority_str, const char* path_query_str, const char* body) {
    proxy_tuple2_string_string_t content_type[] = {{
        .f0 = { .ptr = "User-agent", .len = 10 },
        .f1 = { .ptr = "WASI-HTTP/0.0.1", .len = 15},
    },
    {
        .f0 = { .ptr = "Content-type", .len = 12 },
        .f1 = { .ptr = "application/json", .len = 16},
    }};
    proxy_list_tuple2_string_string_t headers_list = {
        .ptr = &content_type[0],
        .len = 2,
    };
    wasi_http_types_fields_t headers = wasi_http_types_new_fields(&headers_list);
    wasi_http_types_method_t method = { .tag = method_tag };
    wasi_http_types_scheme_t scheme = { .tag = scheme_tag };
    proxy_string_t path_query, authority;
    proxy_string_set(&path_query, path_query_str);
    proxy_string_set(&authority, authority_str);

    wasi_http_outgoing_handler_outgoing_request_t req = wasi_http_types_new_outgoing_request(&method, &path_query, &scheme, &authority, headers);
    wasi_http_outgoing_handler_future_incoming_response_t res;

    if (req == 0) {
        printf("Error creating request\n");
        return 4;
    }
    if (body != NULL) {
        wasi_http_types_outgoing_stream_t ret;
        if (!wasi_http_types_outgoing_request_write(req, &ret)) {
            printf("Error getting output stream\n");
            return 7;
        }
        proxy_list_u8_t buf = {
            .ptr = (uint8_t *) body,
            .len = strlen(body),
        };
        uint64_t ret_val;
        wasi_io_streams_write(ret, &buf, &ret_val, NULL);
    }

    res = wasi_http_outgoing_handler_handle(req, NULL);
    if (res == 0) {
        printf("Error sending request\n");
        return 5;
    }
    
    proxy_result_incoming_response_error_t result;
    if (!wasi_http_types_future_incoming_response_get(res, &result)) {
        printf("failed to get value for incoming request\n");
        return 1;
    }

    if (result.is_err) {
        printf("response is error!\n");
        return 2;
    }
    // poll_drop_pollable(res);

    wasi_http_types_status_code_t code = wasi_http_types_incoming_response_status(result.val.ok);
    printf("STATUS: %d\n", code);

    wasi_http_types_headers_t header_handle = wasi_http_types_incoming_response_headers(result.val.ok);
    proxy_list_tuple2_string_list_u8_t header_list;
    wasi_http_types_fields_entries(header_handle, &header_list);

    for (int i = 0; i < header_list.len; i++) {
        char name[128];
        char value[128];
        strncpy(name, header_list.ptr[i].f0.ptr, header_list.ptr[i].f0.len);
        name[header_list.ptr[i].f0.len] = 0;
        strncpy(value, (const char*) header_list.ptr[i].f1.ptr, header_list.ptr[i].f1.len);
        value[header_list.ptr[i].f1.len] = 0;
        printf("%s: %s\n", name, value);
    }


    wasi_http_types_incoming_stream_t stream;
    if (!wasi_http_types_incoming_response_consume(result.val.ok, &stream)) {
        printf("stream is error!\n");
        return 3;
    }

    printf("Stream is %d\n", stream);

    int32_t len = 64 * 1024;
    proxy_tuple2_list_u8_bool_t body_res;
    wasi_io_streams_stream_error_t err;
    if (!wasi_io_streams_read(stream, len, &body_res, &err)) {
        printf("BODY read is error!\n");
        return 6;
    }
    printf("data from read: %s\n", body_res.f0.ptr);
    proxy_tuple2_list_u8_bool_free(&body_res);


    wasi_http_types_drop_outgoing_request(req);
    wasi_io_streams_drop_input_stream(stream);
    wasi_http_types_drop_incoming_response(result.val.ok);

    return 0;
}

int main() {
    request(WASI_HTTP_TYPES_METHOD_GET, WASI_HTTP_TYPES_SCHEME_HTTPS, "postman-echo.com", "/get?some=arg&goes=here", NULL);
    request(WASI_HTTP_TYPES_METHOD_POST, WASI_HTTP_TYPES_SCHEME_HTTPS, "postman-echo.com", "/post", "{\"foo\": \"bar\"}");
    request(WASI_HTTP_TYPES_METHOD_PUT, WASI_HTTP_TYPES_SCHEME_HTTP, "postman-echo.com", "/put", NULL);
    return 0;
}