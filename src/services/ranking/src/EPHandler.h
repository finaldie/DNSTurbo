#ifndef EPHANDLER_H
#define EPHANDLER_H

#include <string>
#include <skullcpp/api.h>

#include "HttpResponse.h"

class EPHandler {
public:
    typedef void (*EPHandlerCb)(const skullcpp::Service& service,
                           skullcpp::EPClientNPRet& ret,
                           const std::string& question,
                           const std::shared_ptr<HttpResponse>& httpResponse);

public:
    EPHandler();
    ~EPHandler();

public:
    skullcpp::EPClient::Status
    send(const skullcpp::Service& service, const std::string& hostIp,
         int port, int timeout, const std::string& content,
         const std::string& question, EPHandlerCb cb);
};

#endif

