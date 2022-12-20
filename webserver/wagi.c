#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* get_query_string(const char* name, const char* query)
{
    char buff[32];
    snprintf(buff, 32, "%s=", name);
    const char* res = strstr(query, buff);
    if (res == NULL) {
        return NULL;
    }
    int ix = 0;
    while (ix+5 < strlen(res) && res[ix+5] != '&') {
        ix++;
    }
    char *result = (char*) malloc(sizeof(char) * (ix + 1));
    for (int i = 0; i < ix; i++) {
        result[i] = res[5 + i];
    }
    return result;
}

int main(int argc, char**argv)
{
    fprintf(stdout, "Content-type: text/html\n\n");
    const char* query = getenv("QUERY_STRING");
    char* name = get_query_string("name", query);
    if (name != NULL) {
        fprintf(stdout, "<body><h3>Hello C %s!</h3></body>", name);
    }
    else {
        fprintf(stdout, "<body><h3>Hello C Unknown!</h3></body>");
    }
    free(name);
}