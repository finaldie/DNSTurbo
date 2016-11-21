#include <stdlib.h>
#include <string.h>

#include "HttpResponseImp.h"

static
int _on_msg_begin(http_parser* parser) {
    return 0;
}

static
int _on_header_field(http_parser* parser, const char *at, size_t length) {
    auto* httpParser = (HttpResponseImp*)parser->data;
    HdrState state = httpParser->hdrState;

    if (state != HdrState::FIELD) {
        httpParser->addHeader(httpParser->hdrField, httpParser->hdrValue);
        httpParser->hdrField.clear();
    }

    httpParser->hdrState = HdrState::FIELD;
    httpParser->hdrField.append(at, length);
    return 0;
}

static
int _on_header_value(http_parser* parser, const char *at, size_t length) {
    auto* httpParser = (HttpResponseImp*)parser->data;
    HdrState state = httpParser->hdrState;

    if (state != HdrState::VALUE) {
        httpParser->hdrValue.clear();
    }

    httpParser->hdrState = HdrState::VALUE;
    httpParser->hdrValue.append(at, length);
    return 0;
}

static
int _on_headers_complete(http_parser* parser) {
    auto* httpParser = (HttpResponseImp*)parser->data;
    httpParser->addHeader(httpParser->hdrField, httpParser->hdrValue);
    return 0;
}

static
int _on_body(http_parser* parser, const char *at, size_t length) {
    auto* httpParser = (HttpResponseImp*)parser->data;
    httpParser->appendBody(at, length);
    return 0;
}

static
int _on_msg_complete(http_parser* parser) {
    auto* httpParser = (HttpResponseImp*)parser->data;
    httpParser->setCompleted();
    return 0;
}

HttpResponseImp::HttpResponseImp() : nparsed(0), completed(false), hdrState(HdrState::FIELD) {
    http_parser_init(&this->parser, HTTP_RESPONSE);
    this->parser.data = this;

    memset(&this->settings, 0, sizeof(this->settings));
    this->settings.on_message_begin    = _on_msg_begin;
    this->settings.on_header_field     = _on_header_field;
    this->settings.on_header_value     = _on_header_value;
    this->settings.on_body             = _on_body;
    this->settings.on_message_complete = _on_msg_complete;
}

HttpResponseImp::~HttpResponseImp() {
}

size_t HttpResponseImp::parse(const char* data, size_t len) {
    if (!data || len == 0) return this->nparsed;
    if (this->nparsed == len) return this->nparsed;

    size_t nparsed =
        http_parser_execute(&this->parser, &this->settings, data + this->nparsed, len - this->nparsed);
    this->nparsed += nparsed;

    return this->nparsed;
}

size_t HttpResponseImp::parse(const std::string& data) {
    return parse(data.c_str(), data.length());
}

bool HttpResponseImp::isCompleted() const {
    return this->completed;
}

void HttpResponseImp::setCompleted() {
    this->completed = true;
}

void HttpResponseImp::addHeader(const std::string& field, const std::string& value) {
    if (field.empty() || value.empty()) return;

    this->headers.insert(std::pair<std::string, std::string>(field, value));
}

void HttpResponseImp::appendBody(const std::string& content) {
    if (content.empty()) return;

    appendBody(content.c_str(), content.length());
}

void HttpResponseImp::appendBody(const char* content, size_t len) {
    if (!content || len == 0) return;

    this->body.append(content, len);
}

int HttpResponseImp::statusCode() const {
    return this->parser.status_code;
}

const HeaderMap& HttpResponseImp::getHeaders() const {
    return this->headers;
}

const std::string& HttpResponseImp::getBody() const {
    return this->body;
}
