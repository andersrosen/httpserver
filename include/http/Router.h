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

    std::vector<std::unique_ptr<HandlerBase>> handlersWithNoPayload_;
    std::vector<std::unique_ptr<HandlerBase>> handlersWithPayload_;

  public:
    Router() = default;
    ~Router();

    template<typename Func>
    void handleGet(std::string_view pattern, Func func) {
        handlersWithNoPayload_.emplace_back(std::make_unique<Handler<Func>>("GET", pattern, 0, func));
    }

    template<typename Func>
    void handleHead(std::string_view pattern, Func func) {
        handlersWithNoPayload_.emplace_back(std::make_unique<Handler<Func>>("HEAD", pattern, 0, func));
    }

    template<typename Func>
    void handleDelete(std::string_view pattern, Func func) {
        handlersWithNoPayload_.emplace_back(std::make_unique<Handler<Func>>("DELETE", pattern, 0, func));
    }

    template<typename Func>
    void handleOptions(std::string_view pattern, Func func) {
        handlersWithNoPayload_.emplace_back(std::make_unique<Handler<Func>>("OPTIONS", pattern, 0, func));
    }

    template<typename Func>
    void handlePost(std::string_view pattern, std::size_t maxDataSize, Func func) {
        handlersWithPayload_.emplace_back(std::make_unique<Handler<Func>>("POST", pattern, maxDataSize, func));
    }

    template<typename Func>
    void handlePut(std::string_view pattern, std::size_t maxDataSize, Func func) {
        handlersWithPayload_.emplace_back(std::make_unique<Handler<Func>>("PUT", pattern, maxDataSize, func));
    }

    // FIXME: Add handlers for custom methods?

    // FIXME: Add handlers for basic auth, not found, method not allowed

  protected:
    RequestResult onIncomingRequest(Request& request) override;

    RequestResult onRequest(Request& request) override;

    RequestResult onRequest(Request& request, const char* data, std::size_t* byteCount) override;

    void onRequestDone(Request& request) override;
};

} // namespace ARo::Http
