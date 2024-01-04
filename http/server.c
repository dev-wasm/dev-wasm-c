#include "proxy.h"

#include <stdio.h>

void exports_wasi_http_0_2_0_rc_2023_11_10_incoming_handler_handle(exports_wasi_http_0_2_0_rc_2023_11_10_incoming_handler_own_incoming_request_t request, exports_wasi_http_0_2_0_rc_2023_11_10_incoming_handler_own_response_outparam_t response_out) {
    wasi_http_0_2_0_rc_2023_11_10_types_tuple2_field_key_field_value_t headers [] = {
        {
            .f0 = {
                .ptr = (uint8_t*) "Content-type",
                .len = strlen("Content-type"),
            },
            .f1 = {
                .ptr = (uint8_t*) "text/plain",
                .len = strlen("text/plain"),
            },
        },
    };

    wasi_http_0_2_0_rc_2023_11_10_types_list_tuple2_field_key_field_value_t header_list = {
        .ptr = headers,
        .len = 1,
    };

    wasi_http_0_2_0_rc_2023_11_10_types_own_headers_t f;
    wasi_http_0_2_0_rc_2023_11_10_types_header_error_t err;

    wasi_http_0_2_0_rc_2023_11_10_types_static_fields_from_list(&header_list, &f, &err);

    wasi_http_0_2_0_rc_2023_11_10_types_own_outgoing_response_t res = wasi_http_0_2_0_rc_2023_11_10_types_constructor_outgoing_response(f);
    wasi_http_0_2_0_rc_2023_11_10_types_method_outgoing_response_set_status_code(wasi_http_0_2_0_rc_2023_11_10_types_borrow_outgoing_response(res), 200);

    wasi_http_0_2_0_rc_2023_11_10_types_own_outgoing_body_t body;
    wasi_http_0_2_0_rc_2023_11_10_types_method_outgoing_response_body(wasi_http_0_2_0_rc_2023_11_10_types_borrow_outgoing_response(res), &body);

    wasi_http_0_2_0_rc_2023_11_10_types_own_response_outparam_t outparam;
    wasi_http_0_2_0_rc_2023_11_10_types_result_own_outgoing_response_error_code_t res_err;
    wasi_http_0_2_0_rc_2023_11_10_types_static_response_outparam_set(outparam, &res_err);

    wasi_http_0_2_0_rc_2023_11_10_types_own_output_stream_t stream;
    wasi_http_0_2_0_rc_2023_11_10_types_method_outgoing_body_write(wasi_http_0_2_0_rc_2023_11_10_types_borrow_outgoing_body(body), &stream);

    wasi_io_0_2_0_rc_2023_11_10_streams_list_u8_t contents;
    contents.ptr = (uint8_t*) "Hello world!";
    contents.len = strlen((const char*) contents.ptr);
    wasi_io_0_2_0_rc_2023_11_10_streams_stream_error_t stream_err;
    wasi_io_0_2_0_rc_2023_11_10_streams_method_output_stream_blocking_write_and_flush(wasi_io_0_2_0_rc_2023_11_10_streams_borrow_output_stream(stream), &contents, &stream_err);

    wasi_io_0_2_0_rc_2023_11_10_streams_output_stream_drop_own(stream);

    wasi_http_0_2_0_rc_2023_11_10_types_error_code_t error_code;
    wasi_http_0_2_0_rc_2023_11_10_types_static_outgoing_body_finish(body, NULL, &error_code);	
}

int main() {}