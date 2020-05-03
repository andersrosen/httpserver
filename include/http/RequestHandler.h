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


class RequestHandler {
    friend class Internal::ServerImpl;

  public:
    enum class RequestResult {
        Success,
        Failure,
        Continue,
    };

    virtual ~RequestHandler();

  protected:
    virtual RequestResult onIncomingRequest(Request &request) = 0;
    virtual RequestResult onRequest(Request &request) = 0;
    virtual void onRequestDone(Request &request) = 0;
};


class Router final : public RequestHandler {

    struct HandlerBase {
        std::string method;
        std::size_t maxRequestSize = 0;
        std::regex re;
        std::smatch lastMatch;

        HandlerBase(std::string_view method, std::string_view pattern, std::size_t maxRequestSize)
            : method(method)
            , maxRequestSize(maxRequestSize)
            , re(std::string(pattern))
        {}

        virtual ~HandlerBase() = default;

        virtual std::size_t getRequiredParameterCount() const = 0;

        virtual bool canHandle(Request &req) {
            if (method != req.getMethod())
                return false;

            if (!std::regex_match(req.getUrl(), lastMatch, re))
                return false;

            if (lastMatch.size() - 1 != getRequiredParameterCount()) {
                lastMatch = {};
                return false;
            }

            return true;
        };

        virtual void handleRequest(Request &req) {}

        virtual void handleRequestWithData(Request &req) {}
    };


    template <typename Func>
    struct Handler : public HandlerBase {
        Func func;

        static constexpr std::size_t Arity = Internal::function_traits<Func>::arity;

        Handler(std::string_view method, std::string_view pattern, std::size_t maxRequestSize, Func handlerFunc)
            : HandlerBase(method, pattern, maxRequestSize)
            , func(handlerFunc)
        {}

        std::size_t getRequiredParameterCount() const override {
            return Arity - 1;
        }

        void handleRequest(Request& req) override {
            Internal::Invoker<Arity - 1>::run(lastMatch, req, func);
        }
    };


    std::vector<std::unique_ptr<HandlerBase>> handlers_;

  public:
    Router() {};
    ~Router();

    template <typename Func>
    void handleGet(std::string_view pattern, Func func) {
        handlers_.emplace_back(std::make_unique<Handler<Func>>("GET", pattern, 0, func));
    }

    // FIXME: Add more methods

  protected:
    RequestResult onIncomingRequest(Request& request) override {
        for (auto &handler : handlers_) {
            if (handler->canHandle(request))
                return RequestResult::Success;
        }
        // FIXME: Do an auth check somewhere
        // FIXME: Call the not found handler, or method not allowed
        return RequestResult::Failure;
    };

    RequestResult onRequest(Request& request) override {
        // FIXME: Maybe save the handler after onIncomingRequest
        for (auto &handler : handlers_) {
            if (handler->canHandle(request)) {
                handler->handleRequest(request);
                return RequestResult::Success;
            }
        }

        // Something is wrong if we didn't return in the loop above
        return RequestResult::Failure;
    }

    void onRequestDone(Request& request) override {};
};

}
