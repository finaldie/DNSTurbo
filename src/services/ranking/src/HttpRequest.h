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

private:
    bool validate() const;

public:
    HttpRequest();
    virtual ~HttpRequest();

public:
    bool parse(const google::protobuf::Message& request);
    const std::string getHttpContent() const;

public:
    const std::string&   method() const;
    const std::string&   uri() const;
    const std::string&   body() const;
    const HttpReqHdrMap& headers() const;
};

#endif

