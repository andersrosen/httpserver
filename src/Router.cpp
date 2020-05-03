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

void
Router::HandlerBase::handleRequestWithData(Request& req) {
}

Router::~Router() = default;

RequestHandler::RequestResult
Router::onIncomingRequest(Request& request) {
    for (auto& handler : handlers_) {
        if (handler->canHandle(request))
            return RequestResult::Success;
    }
    // FIXME: Do an auth check somewhere
    // FIXME: Call the not found handler, or method not allowed
    return RequestResult::Failure;
}

RequestHandler::RequestResult
Router::onRequest(Request& request) {
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

void
Router::onRequestDone(Request& request) {

}

} // namespace ARo::Http