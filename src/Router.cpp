#include "../include/http/Router.h"
#include "InternalRequest.h"
#include <cassert>

using namespace ARo::Http;

RequestResult
Router::onIncomingRequest(Internal::InternalRequest& request) {
    for (auto& handler : handlers_) {
        if (request.getMethod() != handler->method)
            continue;

        if (!std::regex_match(request.getUrl(), request.match, handler->regex))
            continue;

        if (request.match.size() - 1 != handler->urlParameterCount)
            continue;

        if (handler->canHandle(request)) {
            request.handler = handler.get();
            return RequestResult::Success;
        }
    }
    // FIXME: Do an auth check somewhere
    // FIXME: Call the not found handler, or method not allowed
    return RequestResult::Failure;
}

RequestResult
Router::onRequest(Internal::InternalRequest& request, const char* data, std::size_t* byteCount) {
    assert(request.handler);
    auto *handler = *request.handler;

    const std::size_t MaxRequestSize = 10*1024; // FIXME: Make this configurable per handler

    auto payloadHandling = handler->payloadHandling;

    if (payloadHandling == Internal::PayloadHandling::RawHandling) {
        handler->run(request.match, data, *byteCount, request.userData, request);
        return RequestResult::Success;
    }

    if (*byteCount == 0) {
        // There is no more data
        switch (payloadHandling) {
        case Internal::PayloadHandling::Ignore:
            handler->run(request.match, request);
            break;

        case Internal::PayloadHandling::BufferedString: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            const char* pl = reinterpret_cast<const char*>(request.payload.data());
            handler->run(request.match, std::string_view(pl, request.payload.size()), request);
            break;
        }

        case Internal::PayloadHandling::BufferedBinary:
            handler->run(request.match, request.payload, request);
            break;

        default:
            assert(false);
        }

        return RequestResult::Success;
    }

    // There is more payload data to handle
    switch (payloadHandling) {
    case Internal::PayloadHandling::BufferedString:
        [[fallthrough]];

    case Internal::PayloadHandling::BufferedBinary: {
        if ((request.payload.size() + *byteCount) < MaxRequestSize) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            request.payload.insert(request.payload.end(), data, data + *byteCount);
        }
        *byteCount = 0; // FIXME: Handle oversize request payload!
        break;
    }
    case Internal::PayloadHandling::Ignore:
        *byteCount = 0;
        break;

    default:
        assert(false);
    }

    return RequestResult::Success;
}

void
Router::onRequestDone(Internal::InternalRequest& request) {

}
