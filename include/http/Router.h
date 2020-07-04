#pragma once

#include "internal/RequestHandler.h"
#include <string_view>

namespace ARo::Http::Internal {

class ServerImpl;
class InternalRequest;

} // namespace ARo::Http::Internal


namespace ARo::Http {

enum class RequestResult {
    Success,
    Failure,
};

class Router {
    friend class Internal::ServerImpl;
    std::vector<std::unique_ptr<Internal::HandlerBase>> handlers_;

  public:
    template <typename Func>
    void handleRequest(std::string_view method, std::string_view pattern, Func func) {
        handlers_.emplace_back(std::make_unique<Internal::Handler<Func>>(method, pattern, func));
    }

    template <typename Func>
    void handleGet(std::string_view pattern, Func func) {
        handleRequest("GET", pattern, func);
    }

    template <typename Func>
    void handleHead(std::string_view pattern, Func func) {
        handleRequest("HEAD", pattern, func);
    }

    template <typename Func>
    void handleDelete(std::string_view pattern, Func func) {
        handleRequest("DELETE", pattern, func);
    }

    template <typename Func>
    void handleOptions(std::string_view pattern, Func func) {
        handleRequest("OPTIONS", pattern, func);
    }

    template <typename Func>
    void handlePost(std::string_view pattern, Func func) {
        handleRequest("POST", pattern, func);
    }

    template <typename Func>
    void handlePut(std::string_view pattern, Func func) {
        handleRequest("PUT", pattern, func);
    }

    template <typename Func>
    void handlePatch(std::string_view pattern, Func func) {
        handleRequest("PATCH", pattern, func);
    }

  protected:
    RequestResult onIncomingRequest(Internal::InternalRequest& request);

    RequestResult onRequest(Internal::InternalRequest& request, const char* data, std::size_t* byteCount);

    void onRequestDone(Internal::InternalRequest& request);
};

} // namespace ARo::Http

