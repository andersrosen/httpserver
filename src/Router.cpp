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
Router::HandlerBase::handleRequest(Request& req, const std::vector<std::uint8_t>& payload) {
}

void
Router::HandlerBase::handleRequest(Request& req, std::string_view payload) {
}

void
Router::HandlerBase::handleRequest(Request& req, std::string_view payload, std::any& userData) {
}

void
Router::HandlerBase::handleRequest(Request& req, const std::vector<std::uint8_t>& payload, std::any& userData) {
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
            switch (handler->payloadHandling) {
            case HandlerBase::PayloadHandling::StringInMemory: {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                const char* pl = reinterpret_cast<const char*>(request.payload.data());
                handler->handleRequest(request, std::string_view(pl, request.payload.size()));
                break;
            }
            case HandlerBase::PayloadHandling::VectorInMemory: {
                handler->handleRequest(request, request.payload);
                break;
            }
            case HandlerBase::PayloadHandling::LargeString: {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                const char* pl = reinterpret_cast<const char*>(request.payload.data());
                handler->handleRequest(request, std::string_view(pl, request.payload.size()), request.userData);
                break;
            }
            default:
                break;
            }

            return RequestResult::Failure; // If no response had been enqueued
        } else {

            switch (handler->payloadHandling) {
            case HandlerBase::PayloadHandling::StringInMemory:
                [[fallthrough]];
            case HandlerBase::PayloadHandling::VectorInMemory: {
                if (handler->maxRequestSize > 0 && (request.payload.size() + *byteCount) < handler->maxRequestSize) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    request.payload.insert(request.payload.end(), data, data + *byteCount);
                    *byteCount = 0;
                }
                // FIXME: Handle oversize request payload!
                break;
            }
            case HandlerBase::PayloadHandling::LargeVector: {
                // FIXME: Unnecessary copying!
                std::vector<std::uint8_t> vec;
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                vec.insert(vec.begin(), data, data + *byteCount);
                handler->handleRequest(request, vec, request.userData);
                *byteCount = 0;
                break;
            }
            case HandlerBase::PayloadHandling::LargeString: {
                handler->handleRequest(request, data, request.userData);
                *byteCount = 0;
                break;
            }
            default:
                break;
            }
        }
        return RequestResult::Success;
    }

    // Something is wrong if we didn't return in the loop above
    return RequestResult::Failure;
}

void
Router::onRequestDone(Internal::InternalRequest& request) {

}

} // namespace ARo::Http