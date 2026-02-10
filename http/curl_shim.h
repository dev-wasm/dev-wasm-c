#ifndef __CURL_SHIM_H__
#define __CURL_SHIM_H__

#include <stddef.h>
#include <stdint.h>

/* libcurl-compatible shim for wasi-http
 * Implements common libcurl easy interface functions
 */

/* Forward declarations */
typedef void CURL;
typedef int CURLcode;
typedef int CURLoption;
typedef int CURLINFO;

/* Common curl_slist structure for headers */
struct curl_slist {
    char *data;
    struct curl_slist *next;
};

/* CURLcode return values */
#define CURLE_OK                        0
#define CURLE_UNSUPPORTED_PROTOCOL      1
#define CURLE_FAILED_INIT               2
#define CURLE_URL_MALFORMAT             3
#define CURLE_COULDNT_RESOLVE_HOST      6
#define CURLE_COULDNT_CONNECT           7
#define CURLE_HTTP_RETURNED_ERROR       22
#define CURLE_WRITE_ERROR               23
#define CURLE_OUT_OF_MEMORY             27
#define CURLE_OPERATION_TIMEDOUT        28
#define CURLE_TOO_MANY_REDIRECTS        47
#define CURLE_GOT_NOTHING               52
#define CURLE_RECV_ERROR                56
#define CURLE_SEND_ERROR                55

/* CURLoption values - commonly used options */
#define CURLOPT_URL                     10002
#define CURLOPT_PORT                    3
#define CURLOPT_TIMEOUT                 13
#define CURLOPT_WRITEDATA               10001
#define CURLOPT_WRITEFUNCTION           20011
#define CURLOPT_READDATA                10009
#define CURLOPT_READFUNCTION            20012
#define CURLOPT_HEADERDATA              10029
#define CURLOPT_HEADERFUNCTION          20079
#define CURLOPT_HTTPHEADER              10023
#define CURLOPT_POSTFIELDS              10015
#define CURLOPT_POSTFIELDSIZE           60
#define CURLOPT_POST                    47
#define CURLOPT_HTTPGET                 80
#define CURLOPT_CUSTOMREQUEST           10036
#define CURLOPT_FOLLOWLOCATION          52
#define CURLOPT_USERAGENT               10018
#define CURLOPT_VERBOSE                 41
#define CURLOPT_NOBODY                  44
#define CURLOPT_UPLOAD                  46
#define CURLOPT_PUT                     54

/* CURLINFO values - for retrieving information */
#define CURLINFO_RESPONSE_CODE          0x200002
#define CURLINFO_CONTENT_TYPE           0x100012
#define CURLINFO_EFFECTIVE_URL          0x100001

/* Write callback function type */
typedef size_t (*curl_write_callback)(char *ptr, size_t size, size_t nmemb, void *userdata);

/* Header callback function type */
typedef size_t (*curl_header_callback)(char *buffer, size_t size, size_t nitems, void *userdata);

/* Read callback function type */
typedef size_t (*curl_read_callback)(char *buffer, size_t size, size_t nitems, void *userdata);

/* Global init/cleanup */
CURLcode curl_global_init(long flags);
void curl_global_cleanup(void);

/* Easy interface functions */
CURL *curl_easy_init(void);
void curl_easy_cleanup(CURL *curl);
CURLcode curl_easy_setopt(CURL *curl, CURLoption option, ...);
CURLcode curl_easy_perform(CURL *curl);
CURLcode curl_easy_getinfo(CURL *curl, CURLINFO info, ...);
void curl_easy_reset(CURL *curl);
const char *curl_easy_strerror(CURLcode code);

/* Header list management */
struct curl_slist *curl_slist_append(struct curl_slist *list, const char *string);
void curl_slist_free_all(struct curl_slist *list);

#endif /* __CURL_SHIM_H__ */
