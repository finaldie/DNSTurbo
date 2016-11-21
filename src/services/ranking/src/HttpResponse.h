#ifndef HTTP_RESPONSE_PARSER_H
#define HTTP_RESPONSE_PARSER_H

#include <string>
#include <map>

typedef std::map<std::string, std::string> HeaderMap;

class HttpResponse {
public:
    HttpResponse() {}
    virtual ~HttpResponse() {}

public:
    /**
     * Input the buffer data, then return how many size got parsed
     */
    virtual size_t parse(const char* data, size_t len) = 0;
    virtual size_t parse(const std::string& data) = 0;

    /**
     * Return whether the parsing be completed
     */
    virtual bool isCompleted() const = 0;

public:
    virtual int statusCode() const = 0;
    virtual const HeaderMap&   getHeaders() const = 0;
    virtual const std::string& getBody() const = 0;
};

#endif

