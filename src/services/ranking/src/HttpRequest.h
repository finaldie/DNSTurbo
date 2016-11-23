#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string>
#include <map>
#include <google/protobuf/message.h>

typedef std::map<std::string, std::string> HttpReqHdrMap;

class HttpRequest {
private:
    std::string   method_;
    std::string   uri_;
    std::string   body_;
    HttpReqHdrMap headers_;

    bool          hasConnectionHeader;
    bool          hasAcceptHeader;
    bool          hasAcceptCharsetHeader;

public:
    HttpRequest();
    virtual ~HttpRequest();

public:
    void  setMethod(const std::string& method);
    void  setURI(const std::string& uri);
    void  setBody(const std::string& body);
    void  setHeaders(const HttpReqHdrMap& headerMap);

    bool  validate() const;
    const std::string getHttpContent() const;

public:
    const std::string&   method() const;
    const std::string&   uri() const;
    const std::string&   body() const;
    const HttpReqHdrMap& headers() const;
};

#endif

