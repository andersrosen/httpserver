#include "../include/http/Server.h"

#include "ServerImpl.h"

namespace ARo::Http {

Server::Server(std::uint16_t port)
    : impl_(std::make_unique<Internal::ServerImpl>(port))
{}

Server::~Server() = default;

Server &
Server::setHandler(std::unique_ptr<Router> handler) & {
    impl_->setHandler(std::move(handler));
    return *this;
}

} // namespace ARo::Http