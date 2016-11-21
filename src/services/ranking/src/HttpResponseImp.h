#ifndef HTTP_RESPONSE_PARSER_IMP_H
#define HTTP_RESPONSE_PARSER_IMP_H

#include <string>
#include <map>

#include "http_parser.h"
#include "HttpResponse.h"

typedef std::map<std::string, std::string> HeaderMap;

typedef enum HdrState {
    FIELD = 0,
    VALUE = 1
} HdrState;

class HttpResponseImp : public HttpResponse {
private:
    http_parser parser;
    http_parser_settings settings;
    size_t      nparsed;

    HeaderMap   headers;
    std::string body;

    bool        completed;

public:
    /**
     * This section is used for temporary storing the header parsing state
     */
    HdrState    hdrState;
    std::string hdrField;
    std::string hdrValue;

public:
    HttpResponseImp();
    virtual ~HttpResponseImp();

public:
    /**
     * Input the buffer data, then return how many size got parsed
     */
    virtual size_t parse(const char* data, size_t len);
    virtual size_t parse(const std::string& data);

    /**
     * Return whether the parsing be completed
     */

public:
    virtual bool isCompleted() const;
    void setCompleted();

public:
    void addHeader(const std::string& field, const std::string& value);
    void appendBody(const std::string& content);
    void appendBody(const char* content, size_t len);

public:
    virtual int statusCode() const;
    virtual const HeaderMap&   getHeaders() const;
    virtual const std::string& getBody() const;
};

#endif

