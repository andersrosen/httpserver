#include "../include/http/Router.h"
#include "InternalRequest.h"

namespace ARo::Http {

Router::HandlerBase::HandlerBase(
    std::string_view method,
    std::string_view pattern,
    std::size_t maxRequestSize,
    PayloadHandling payloadHandling)
: payloadHandling(payloadHandling), method(method), maxRequestSize(maxRequestSize), re(std::string(pattern))
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

void
Router::HandlerBase::handleRequest(ARo::Http::Request& req, const std::vector<std::uint8_t>& payload) {
}

void
Router::HandlerBase::handleRequest(ARo::Http::Request& req, std::string_view payload) {
}

Router::~Router() = default;

RequestResult
Router::onIncomingRequest(Internal::InternalRequest& request) {
    for (auto& handler : handlers_) {
        if (handler->canHandle(request))
            return RequestResult::Success;
    }
    // FIXME: Do an auth check somewhere
    // FIXME: Call the not found handler, or method not allowed
    return RequestResult::Failure;
}

RequestResult
Router::onRequest(Internal::InternalRequest& request) {
    // FIXME: Maybe save the handler after onIncomingRequest
    for (auto& handler : handlers_) {
        if (handler->canHandle(request)) {
            handler->handleRequest(request);
            return RequestResult::Success;
        }
    }

    // Something is wrong if we didn't return in the loop above
    return RequestResult::Failure;
}

RequestResult
Router::onRequest(Internal::InternalRequest& request, const char* data, std::size_t* byteCount) {
    // FIXME: Maybe save the handler after onIncomingRequest
    for (auto& handler : handlers_) {
        if (!handler->canHandle(request))
            continue;

        if (*byteCount == 0) {
            // We got the last chunk of data
            if (handler->payloadHandling == HandlerBase::PayloadHandling::StringInMemory) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                const char* pl = reinterpret_cast<const char*>(request.payload_.data());
                handler->handleRequest(request, std::string_view(pl, request.payload_.size()));
            } else if (handler->payloadHandling == HandlerBase::PayloadHandling::VectorInMemory)
                handler->handleRequest(request, request.payload_);
            return RequestResult::Failure; // If no response had been enqueued
        }

        if (handler->maxRequestSize > 0 && (request.payload_.size() + *byteCount) < handler->maxRequestSize) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            request.payload_.insert(request.payload_.end(), data, data + *byteCount);
            *byteCount = 0;
            return RequestResult::Success;
        }

        return RequestResult::Failure;
    }

    // Something is wrong if we didn't return in the loop above
    return RequestResult::Failure;
}

void
Router::onRequestDone(Internal::InternalRequest& request) {

}

} // namespace ARo::Http