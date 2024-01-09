#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <string>

class QueryParams {
    private:
        std::map<std::string, std::string> params;
    
    public:
        QueryParams(const std::string& querystring) {
            std::size_t last = 0;
            std::size_t pos = querystring.find("&");
            while (pos != std::string::npos)
            {
                this->addKey(querystring.substr(last, pos-last));
                last = pos + 1;
                pos = querystring.find("&", last);
            }
            this->addKey(querystring.substr(last));
        }

        const char* value(const std::string& name) {
            if (params.find(name) != params.end()) {
                return params.find(name)->second.c_str();
            }
            return NULL;
        }

    private:
        void addKey(const std::string& elt) {
            std::size_t eq_pos = elt.find("=");
            params.insert(make_pair(elt.substr(0, eq_pos), elt.substr(eq_pos + 1)));
        }
};

int main(int argc, char**argv)
{
    fprintf(stdout, "Content-type: text/html\n\n");
    const char* query = getenv("QUERY_STRING");
    QueryParams params(query);
    
    const char* name = params.value("name");
    if (name == NULL) {
        name = "unknown";
    }
    fprintf(stdout, "<body><h3>Hello C++ %s</h3></body>", name);
}