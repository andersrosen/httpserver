#include "../include/http/Router.h"

namespace ARo::Http {

Router::HandlerBase::HandlerBase(std::string_view method, std::string_view pattern, std::size_t maxRequestSize)
: method(method), maxRequestSize(maxRequestSize), re(std::string(pattern))
{}

Router::HandlerBase::~HandlerBase() = default;

bool
Router::HandlerBase::canHandle(Request& req) {
    if (method != req.getMethod())
        return false;

    if (!std::regex_match(req.getUrl(), lastMatch, re))
        return false;

    if (lastMatch.size() - 1 != getRequiredParameterCount()) {
        lastMatch = {};
        return false;
    }

    return true;
}

void
Router::HandlerBase::handleRequest(Request& req) {
}

Router::~Router() = default;

RequestResult
Router::onIncomingRequest(Request& request) {
    for (auto& handler : handlersWithNoPayload_) {
        if (handler->canHandle(request))
            return RequestResult::Success;
    }
    for (auto& handler : handlersWithPayload_) {
        if (handler->canHandle(request))
            return RequestResult::Success;
    }
    // FIXME: Do an auth check somewhere
    // FIXME: Call the not found handler, or method not allowed
    return RequestResult::Failure;
}

RequestResult
Router::onRequest(Request& request) {
    // FIXME: Maybe save the handler after onIncomingRequest
    for (auto& handler : handlersWithNoPayload_) {
        if (handler->canHandle(request)) {
            handler->handleRequest(request);
            return RequestResult::Success;
        }
    }

    // Something is wrong if we didn't return in the loop above
    return RequestResult::Failure;
}

RequestResult
Router::onRequest(Request& request, const char* data, std::size_t* byteCount) {
    // FIXME: Maybe save the handler after onIncomingRequest
    for (auto& handler : handlersWithPayload_) {
        if (!handler->canHandle(request))
            continue;

        if (*byteCount == 0) {
            // We got the last chunk of data
            handler->handleRequest(request);
            return RequestResult::Failure; // If no response had been enqueued
        }

        auto &reqdata = request.getRequestDataAsBytes();

        if (handler->maxRequestSize > 0 && (reqdata.size() + *byteCount) < handler->maxRequestSize) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            reqdata.insert(reqdata.end(), data, data + *byteCount);
            *byteCount = 0;
            return RequestResult::Success;
        }

        return RequestResult::Failure;
    }

    // Something is wrong if we didn't return in the loop above
    return RequestResult::Failure;
}

void
Router::onRequestDone(Request& request) {

}

} // namespace ARo::Http