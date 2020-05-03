#pragma once

#include "RequestHandler.h"

namespace ARo::Http {

class Router final : public RequestHandler {

    struct HandlerBase {
        std::string method;
        std::size_t maxRequestSize = 0;
        std::regex re;
        std::smatch lastMatch;

        HandlerBase(std::string_view method, std::string_view pattern, std::size_t maxRequestSize);

        virtual ~HandlerBase();

        virtual std::size_t getRequiredParameterCount() const = 0;

        virtual bool canHandle(Request& req);

        virtual void handleRequest(Request& req);

        virtual void handleRequestWithData(Request& req);
    };

    template<typename Func>
    struct Handler : public HandlerBase {
        Func func;

        static constexpr std::size_t Arity = Internal::function_traits<Func>::arity;

        Handler(std::string_view method, std::string_view pattern, std::size_t maxRequestSize, Func handlerFunc)
        : HandlerBase(method, pattern, maxRequestSize), func(handlerFunc)
        {}

        std::size_t getRequiredParameterCount() const override { return Arity - 1; }

        void handleRequest(Request& req) override {
            Internal::Invoker<Arity - 1>::run(lastMatch, req, func);
        }
    };

    std::vector<std::unique_ptr<HandlerBase>> handlers_;

  public:
    Router() = default;
    ~Router();

    template<typename Func>
    void handleGet(std::string_view pattern, Func func) {
        handlers_.emplace_back(std::make_unique<Handler<Func>>("GET", pattern, 0, func));
    }

    // FIXME: Add more methods

  protected:
    RequestResult onIncomingRequest(Request& request) override;

    RequestResult onRequest(Request& request) override;

    void onRequestDone(Request& request) override;
};

} // namespace ARo::Http
