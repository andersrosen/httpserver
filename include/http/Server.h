#pragma once

#include <cstdint>
#include <memory>

#include "RequestHandler.h"

namespace ARo::Http
{

namespace Internal {
    class ServerImpl;
}

class Server
{
  private:
    std::unique_ptr<Internal::ServerImpl> impl_;

  public:
    Server(std::uint16_t port);
    ~Server();

    Server &setHandler(std::unique_ptr<RequestHandler> handler) &;
};

}