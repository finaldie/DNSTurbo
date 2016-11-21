#ifndef EPHANDLER_H
#define EPHANDLER_H

#include <string>
#include <skullcpp/api.h>

class EPHandler {
public:
    EPHandler();
    ~EPHandler();

public:
    skullcpp::EPClient::Status
    send(const skullcpp::Service& service, const std::string& hostIp,
         int port, int timeout, const std::string& content);
};

#endif

