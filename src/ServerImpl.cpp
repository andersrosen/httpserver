#include "ServerImpl.h"

#include <iostream>

#include "../include/http/Request.h"
#include "http/Router.h"

///////////////////////////////////////////////////////////////////////////
// libmicrohttpd callbacks

static void*
urlLogCallback(void* cls, const char* url, MHD_Connection* connection) {
    auto impl = static_cast<ARo::Http::Internal::ServerImpl*>(cls);
    return static_cast<void*>(impl->createNewRequest(url));
}

static int
requestCallback(void* cls,
    MHD_Connection* connection,
    const char* url,
    const char* method,
    const char* version,
    const char* uploadData,
    std::size_t* uploadDataSize,
    void** conCls
) {
    using State = ARo::Http::Internal::InternalRequest::State;
    auto impl = static_cast<ARo::Http::Internal::ServerImpl*>(cls);
    auto req = static_cast<ARo::Http::Internal::InternalRequest*>(*conCls);

    if (req->getState() == State::Uninitialized) {
        // First call
        req->setConnection(connection);
        req->setUrl(url);
        req->setHttpVersion(version);
        req->setMethod(method);
        req->setState(State::Initialized);
        return impl->onRequestBegin(req);
    }

    return impl->onRequestData(req, uploadData, uploadDataSize);
}

static void
requestCompletedCallback(void* cls,
    MHD_Connection* connection,
    void** conCls,
    MHD_RequestTerminationCode terminationCode
) {

    auto impl = static_cast<ARo::Http::Internal::ServerImpl*>(cls);
    auto req = static_cast<ARo::Http::Internal::InternalRequest*>(*conCls);

    impl->onRequestCompleted(req, terminationCode);
}

///////////////////////////////////////////////////////////////////////////
// ServerImpl implementation

namespace ARo::Http::Internal {

ServerImpl::ServerImpl(std::uint16_t port)
{
    // NOLINTNEXTLINE
    auto dp = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, port,
        nullptr, nullptr, // Accept policy callback & param
        requestCallback, static_cast<void*>(this), // callback &  param
        MHD_OPTION_URI_LOG_CALLBACK, urlLogCallback, static_cast<void*>(this), // callback & param
        MHD_OPTION_NOTIFY_COMPLETED, requestCompletedCallback, static_cast<void*>(this), // callback & param
        MHD_OPTION_END);
    if (!dp) {
        std::ostringstream oss;
        oss << "Failed to start microhttpd daemon listening on port " << port;
        throw std::runtime_error(oss.str());
    }
    daemon_.reset(dp);
}

ServerImpl::~ServerImpl() = default;

void
ServerImpl::setRouter(std::unique_ptr<Router> router) {
    router_ = std::move(router);
}

InternalRequest*
ServerImpl::createNewRequest(const char* url) {
    return new InternalRequest(url); // NOLINT(cppcoreguidelines-owning-memory)
}

int
ServerImpl::onRequestBegin(InternalRequest* req) {
    // Auth stuff - look up handler, etc
    auto result = router_->onIncomingRequest(*req);
    if (result == RequestResult::Success)
        return MHD_YES;
    return MHD_NO; // Failure - close the connection
}

int
ServerImpl::onRequestData(
    InternalRequest* req,
    const char* uploadData,
    std::size_t* uploadDataSize
) {
    if (req->getState() == InternalRequest::State::Initialized) {
        req->setState(InternalRequest::State::Ongoing);
    }
    if (router_->onRequest(*req, uploadData, uploadDataSize) == RequestResult::Success)
        return MHD_YES; // Everything fine
    return MHD_NO; // Failure - close the connection
}

void
ServerImpl::onRequestCompleted(
    InternalRequest* req,
    MHD_RequestTerminationCode terminationCode
) {
    // Clean up
    router_->onRequestDone(*req);
    delete req; // NOLINT(cppcoreguidelines-owning-memory)
}

} // namespace ARo::Http::Internal
