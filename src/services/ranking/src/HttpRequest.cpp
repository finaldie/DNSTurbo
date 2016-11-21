#include <stdlib.h>
#include <string.h>
#include <sstream>

#include "skull_protos.h"
#include "HttpRequest.h"

HttpRequest::HttpRequest() : hasConnectionHeader(false), hasAcceptHeader(false),
  hasAcceptCharsetHeader(false) {
}

HttpRequest::~HttpRequest() {
}

bool HttpRequest::parse(const google::protobuf::Message& request) {
    const auto& query = (skull::service::http::query_req&)request;

    // Extract query items
    this->method_  = query.has_method()  ? query.method()  : "GET";
    this->uri_     = query.has_uri()     ? query.uri()     : "";
    this->body_    = query.has_body()    ? query.body()    : "";

    HttpReqHdrMap headers;
    int headerSz = query.header_size();
    for (int i = 0; i < headerSz; i++) {
        const auto& headerItem = query.header(i);

        if (!headerItem.has_name() || !headerItem.name().empty()) {
            continue;
        }

        if (!headerItem.has_value() || headerItem.value().empty()) {
            continue;
        }

        // Filter Content-Length header, will re-calculate base on body
        if (headerItem.name() == "Content-Length") {
            continue;
        }

        if (headerItem.name() == "Connection") {
            this->hasConnectionHeader = true;
        }

        if (headerItem.name() == "Accept") {
            this->hasAcceptHeader = true;
        }

        if (headerItem.name() == "Accept-Charset") {
            this->hasAcceptCharsetHeader = true;
        }

        headers.insert(std::pair<std::string, std::string>(headerItem.name(), headerItem.value()));
    }

    // Validate Request
    return this->validate();
}

bool HttpRequest::validate() const {
    if (this->method_ != "GET" && this->method_ != "POST") return false;

    return true;
}

const std::string HttpRequest::getHttpContent() const {
    std::ostringstream oss;
    bool postMethod = this->method_ == "POST";

    oss << this->method_ << " " << this->uri_ << " " << "HTTP/1.1\r\n";
    for (const auto& hdr : this->headers_) {
        oss << hdr.first << ": " << hdr.second << "\r\n";
    }

    if (postMethod) {
        oss << "Content-Length: " << this->body().length() << "\r\n";
    }

    if (!this->hasConnectionHeader) {
        oss << "Connection: Keep-Alive\r\n";
    }

    if (!this->hasAcceptHeader) {
        oss << "Accept: text/plain\r\n";
    }

    if (!hasAcceptCharsetHeader) {
        oss << "Accept-Charset: UTF-8\r\n";
    }

    oss << "\r\n";

    if (postMethod) {
        oss << this->body_;
    }

    return oss.str();
}

const std::string& HttpRequest::method() const {
    return this->method_;
}

const std::string& HttpRequest::uri() const {
    return this->uri_;
}

const std::string& HttpRequest::body() const {
    return this->body_;
}

const HttpReqHdrMap& HttpRequest::headers() const {
    return this->headers_;
}
