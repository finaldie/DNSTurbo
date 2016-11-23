#include <stdlib.h>

#include "skull_protos.h"
#include "HttpResponseImp.h"
#include "EPHandler.h"

static
ssize_t _ep_unpack(const void* data, size_t len,
                   std::shared_ptr<HttpResponse>& httpResponse)
{
    size_t nparsed = httpResponse->parse((const char*)data, len);

    if (!httpResponse->isCompleted()) {
        return 0;
    }

    if (nparsed < len) {
        return -1;
    }

    return (ssize_t)nparsed;
}

static
void _ep_cb(const skullcpp::Service& service, skullcpp::EPClientNPRet& ret,
            const std::shared_ptr<HttpResponse>& httpResponse,
            const std::string& question,
            EPHandler::EPHandlerCb cb)
{
    if (ret.status() == skullcpp::EPClient::OK) {
        SKULLCPP_LOG_DEBUG("ep response: " <<
            std::string((const char*)ret.response(), ret.responseSize() >= 1024 ? 1024 : ret.responseSize()));
    } else {
        SKULLCPP_LOG_DEBUG("ep timeout or error");
    }

    // callback with http response
    cb(service, ret, question, httpResponse);
}

EPHandler::EPHandler() {
}

EPHandler::~EPHandler() {
}

skullcpp::EPClient::Status
EPHandler::send(const skullcpp::Service& service, const std::string& hostIp,
                int port, int timeout, const std::string& content,
                const std::string& question, EPHandlerCb cb) {
    std::shared_ptr<HttpResponse> httpResponse;
    httpResponse.reset(new HttpResponseImp());

    skullcpp::EPClient epClient;
    epClient.setType(skullcpp::EPClient::TCP);
    epClient.setIP(hostIp);
    epClient.setPort((in_port_t)port);
    epClient.setTimeout(timeout);
    epClient.setUnpack(skull_BindEpUnpack(_ep_unpack, httpResponse));

    skullcpp::EPClient::Status st =
        epClient.send(service, content,
                      skull_BindEpNPCb(_ep_cb, httpResponse, question, cb));

    return st;
}

