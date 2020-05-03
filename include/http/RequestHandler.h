#pragma once

#include <functional>
#include <regex>

#include "FunctionTraits.h"
#include "Request.h"

namespace ARo::Http {

class Request;

namespace Internal {

class ServerImpl;

template<int ArgCount>
struct Invoker {
    template<typename Func, typename... Args, int Boundary = ArgCount>
    static typename std::enable_if_t<sizeof...(Args) < Boundary, void>
    run(std::smatch& match,
        Request& req,
        Func&& func,
        Args... args) {
        std::string val = match[sizeof...(Args) + 1].str();
        run<Func>(match, req, std::forward<Func>(func), std::forward<Args>(args)..., std::move(val));
    }

    template<typename Func, typename... Args, int Boundary = ArgCount>
    static typename std::enable_if_t<sizeof...(Args) == Boundary, void>
    run(std::smatch& match,
        Request& req,
        Func&& func,
        Args... args) {
        func(req, std::move(args)...);
    }
};

} // namespace Internal


enum class RequestResult {
    Success,
    Failure,
};

class RequestHandler {
    friend class Internal::ServerImpl;

  public:

    virtual ~RequestHandler();

  protected:
    virtual RequestResult onIncomingRequest(Request &request) = 0;
    virtual RequestResult onRequest(Request &request) = 0;
    virtual void onRequestDone(Request &request) = 0;
};



}
