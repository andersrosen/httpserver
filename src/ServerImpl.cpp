#include "ServerImpl.h"

#include <iostream>

#include "../include/http/Request.h"
#include "../include/http/RequestHandler.h"

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

    auto impl = static_cast<ARo::Http::Internal::ServerImpl*>(cls);
    auto req = static_cast<ARo::Http::Internal::InternalRequest*>(*conCls);

    if (!req->getConnection()) {
        // First call
        req->setConnection(connection);
        req->setUrl(url);
        req->setHttpVersion(version);
        req->setMethod(method);
        return impl->onRequestBegin(req);
    }
    return impl->onRequestData(req, uploadData, *uploadDataSize);
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
ServerImpl::setHandler(std::unique_ptr<RequestHandler> handler) {
    handler_ = std::move(handler);
}

InternalRequest*
ServerImpl::createNewRequest(const char* url) {
    return new InternalRequest(url); // NOLINT(cppcoreguidelines-owning-memory)
}

int
ServerImpl::onRequestBegin(InternalRequest* req) {
    // Auth stuff - look up handler, etc
    std::cerr << "full url = " << req->getFullUrl()
              << ", url = " << req->getUrl()
              << ", method = " << req->getMethod()
              << ", version = " << req->getHttpVersion()
              << std::endl;
    handler_->onIncomingRequest(*req);
    return MHD_YES;
}

int
ServerImpl::onRequestData(
    InternalRequest* req,
    const char* uploadData,
    std::size_t& uploadDataSize
) {
    handler_->onRequest(*req);
    // Actual request handling
    return MHD_NO;
}

void
ServerImpl::onRequestCompleted(
    InternalRequest* req,
    MHD_RequestTerminationCode terminationCode
) {
    // Clean up
    handler_->onRequestDone(*req);
    delete req; // NOLINT(cppcoreguidelines-owning-memory)
}

} // namespace ARo::Http::Internal
