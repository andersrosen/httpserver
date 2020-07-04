#pragma once

#include <cstdint>
#include <microhttpd.h>
#include <unordered_map>
#include <vector>

#include "../include/http/Request.h"
#include "../include/http/Server.h"
#include "InternalRequest.h"

namespace ARo::Http {
    class OldRouter;
}

namespace ARo::Http::Internal {

class ServerImpl {
    struct MhdDaemonDeleter {
        void operator()(struct MHD_Daemon* p) { MHD_stop_daemon(p); }
    };

    std::unique_ptr<struct MHD_Daemon, MhdDaemonDeleter> daemon_;
    std::vector<InternalRequest> requests_;
    std::unique_ptr<Router> router_;

  public:
    explicit ServerImpl(std::uint16_t port);
    ~ServerImpl();

    void setRouter(std::unique_ptr<Router> handler);

    InternalRequest *createNewRequest(const char* url);
    int onRequestBegin(InternalRequest *req);
    int onRequestData(InternalRequest *req, const char *uploadData, std::size_t *uploadDataSize);
    void onRequestCompleted(InternalRequest *req, MHD_RequestTerminationCode);
};


}
