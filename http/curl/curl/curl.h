typedef struct {
    const char* url;
} CURL;
typedef enum {
    CURLE_OK,
    CURLE_UNKNOWN_OPTION
} CURLcode;

typedef enum {
    CURLOPT_FOLLOWLOCATION,
    CURLOPT_URL
} CURLoption;

CURL* curl_easy_init();
CURLcode curl_easy_setopt(CURL *handle, CURLoption option, ...);
CURLcode curl_easy_perform(CURL *easy_handle);
void curl_easy_cleanup(CURL *handle);
const char* curl_easy_strerror(CURLcode res);
