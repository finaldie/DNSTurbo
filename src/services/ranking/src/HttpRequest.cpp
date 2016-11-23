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

void HttpRequest::setMethod(const std::string& method) {
    this->method_ = method.empty() ? "GET" : method;
}

void HttpRequest::setURI(const std::string& uri) {
    this->uri_ = uri;
}

void HttpRequest::setBody(const std::string& body) {
    this->body_ = body;
}

void HttpRequest::setHeaders(const HttpReqHdrMap& headerMap) {
    if (headerMap.empty()) {
        return;
    }

    for (const auto& headerItem : headerMap) {
        const auto& name  = headerItem.first;
        const auto& value = headerItem.second;

        if (name.empty()) {
            continue;
        }

        if (value.empty()) {
            continue;
        }

        // Filter Content-Length header, will re-calculate base on body
        if (name == "Content-Length") {
            continue;
        }

        if (name == "Connection") {
            this->hasConnectionHeader = true;
        }

        if (name == "Accept") {
            this->hasAcceptHeader = true;
        }

        if (name == "Accept-Charset") {
            this->hasAcceptCharsetHeader = true;
        }

        this->headers_.insert(std::make_pair(name, value));
    }
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
