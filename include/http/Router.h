#pragma once

#include <cstring>
#include <functional>
#include <regex>
#include <string_view>
#include <vector>

#include "FunctionTraits.h"
#include "Request.h"

namespace ARo::Http {

namespace Internal {

class ServerImpl;
class InternalRequest;

template<int ArgCount>
struct Invoker {

    // Handler with no request payload

    template<typename Func, typename... Args, int Boundary = ArgCount>
    static typename std::enable_if_t<sizeof...(Args) < Boundary, void> run(std::smatch& match,
        Request& req,
        Func&& func,
        Args... args) {
        run<Func>(match, req, std::forward<Func>(func), std::forward<Args>(args)..., match[sizeof...(Args) + 1].str());
    }

    template<typename Func, typename... Args, int Boundary = ArgCount>
    static typename std::enable_if_t<sizeof...(Args) == Boundary, void> run(std::smatch& match,
        Request& req,
        Func&& func,
        Args... args) {
        func(req, std::forward<Args>(args)...);
    }

    // Handler with in-memory request payload as uint8_t vector

    template<typename Func, typename... Args, int Boundary = ArgCount>
    static typename std::enable_if_t<sizeof...(Args) < Boundary, void> run(std::smatch& match,
        Request& req,
        const std::vector<std::uint8_t>& payload,
        Func&& func,
        Args... args) {
        run<Func>(match,
            req,
            payload,
            std::forward<Func>(func),
            std::forward<Args>(args)...,
            match[sizeof...(Args) + 1].str());
    }

    template<typename Func, typename... Args, int Boundary = ArgCount>
    static typename std::enable_if_t<sizeof...(Args) == Boundary, void> run(std::smatch& match,
        Request& req,
        const std::vector<std::uint8_t>& payload,
        Func&& func,
        Args... args) {
        func(req, payload, std::forward<Args>(args)...);
    }

    // Handler with in-memory request payload as std::string_view

    template<typename Func, typename... Args, int Boundary = ArgCount>
    static typename std::enable_if_t<sizeof...(Args) < Boundary, void>
    run(std::smatch& match,
        Request& req,
        std::string_view payload,
        Func&& func,
        Args... args) {

        run<Func>(match,
            req,
            payload,
            std::forward<Func>(func),
            std::forward<Args>(args)...,
            match[sizeof...(Args) + 1].str());
    }

    template<typename Func, typename... Args, int Boundary = ArgCount>
    static typename std::enable_if_t<sizeof...(Args) == Boundary, void>
    run(std::smatch& match,
        Request& req,
        std::string_view payload,
        Func&& func,
        Args... args) {

        func(req, payload, std::forward<Args>(args)...);
    }

    // Handler with more advanced payload handling
};

} // namespace ARo::Http::Internal

enum class RequestResult {
    Success,
    Failure,
};

class Router final {
    friend class Internal::ServerImpl;

    struct HandlerBase {
        enum class PayloadHandling {
            Ignore,
            StringInMemory,
            VectorInMemory,
            PartProcessor,
            RawCallback
        };

        PayloadHandling payloadHandling;
        std::string method;
        std::size_t maxRequestSize;
        std::regex re;
        std::smatch lastMatch;

        HandlerBase(std::string_view method, std::string_view pattern, std::size_t maxRequestSize, PayloadHandling payloadHandling);

        virtual ~HandlerBase();

        virtual std::size_t getRequiredParameterCount() const = 0;

        virtual bool canHandle(Request& req);

        virtual void handleRequest(Request& req);

        virtual void handleRequest(Request& req, const std::vector<std::uint8_t> &payload);

        virtual void handleRequest(Request& req, std::string_view payload);
    };

    template<typename Func>
    struct HandlerNoPayload : public HandlerBase {
        Func func;

        static constexpr std::size_t Arity = Internal::function_traits<Func>::arity;

        HandlerNoPayload(std::string_view method, std::string_view pattern, Func handlerFunc)
        : HandlerBase(method, pattern, -1, PayloadHandling::Ignore), func(handlerFunc)
        {}

        std::size_t getRequiredParameterCount() const override { return Arity - 1; }

        void handleRequest(Request& req) override {
            Internal::Invoker<Arity - 1>::run(lastMatch, req, func);
        }
    };

    template<typename Func>
    struct HandlerVectorPayload : public HandlerBase {
        Func func;

        static constexpr std::size_t Arity = Internal::function_traits<Func>::arity;

        HandlerVectorPayload(std::string_view method, std::string_view pattern, std::size_t maxRequestSize, Func handlerFunc)
            : HandlerBase(method, pattern, maxRequestSize, PayloadHandling::VectorInMemory), func(handlerFunc)
        {}

        std::size_t getRequiredParameterCount() const override { return Arity - 2; }

        void handleRequest(Request& req, const std::vector<std::uint8_t>& payload) override {
            Internal::Invoker<Arity - 2>::run(lastMatch, req, payload, func);
        }
    };

    template<typename Func>
    struct HandlerStringPayload : public HandlerBase {
        Func func;

        static constexpr std::size_t Arity = Internal::function_traits<Func>::arity;

        HandlerStringPayload(std::string_view method, std::string_view pattern, std::size_t maxRequestSize, Func handlerFunc)
            : HandlerBase(method, pattern, maxRequestSize, PayloadHandling::StringInMemory), func(handlerFunc)
        {}

        std::size_t getRequiredParameterCount() const override { return Arity - 2; }

        void handleRequest(Request& req, std::string_view payload) override {
            Internal::Invoker<Arity - 2>::run(lastMatch, req, payload, func);
        }
    };

    std::vector<std::unique_ptr<HandlerBase>> handlers_;

  public:
    Router() = default;
    ~Router();

    template<typename Func>
    void handleNoPayload(std::string_view method, std::string_view pattern, Func func) {
        handlers_.emplace_back(std::make_unique<HandlerNoPayload<Func>>(method, pattern, func));
    }

    template<typename Func>
    void handleVectorPayload(std::string_view method, std::string_view pattern, std::size_t maxSize, Func func) {
        handlers_.emplace_back(std::make_unique<HandlerVectorPayload<Func>>(method, pattern, maxSize, func));
    }

    template<typename Func>
    void handleStringPayload(std::string_view method, std::string_view pattern, std::size_t maxSize, Func func) {
        handlers_.emplace_back(std::make_unique<HandlerStringPayload<Func>>(method, pattern, maxSize, func));
    }

    template<typename Func>
    void handleGet(std::string_view pattern, Func func) {
        handleNoPayload("GET", pattern, func);
    }

    template<typename Func>
    void handleHead(std::string_view pattern, Func func) {
        handleNoPayload("HEAD", pattern, func);
    }

    template<typename Func>
    void handleDelete(std::string_view pattern, Func func) {
        handleNoPayload("DELETE", pattern, func);
    }

    template<typename Func>
    void handleOptions(std::string_view pattern, Func func) {
        handleNoPayload("OPTIONS", pattern, func);
    }

    template<typename Func>
    void handlePostVectorPayload(std::string_view pattern, std::size_t maxDataSize, Func func) {
        handleVectorPayload("POST", pattern, maxDataSize, func);
    }

    template<typename Func>
    void handlePostStringPayload(std::string_view pattern, std::size_t maxDataSize, Func func) {
        handleStringPayload("POST", pattern, maxDataSize, func);
    }

    template<typename Func>
    void handlePutVectorPayload(std::string_view pattern, std::size_t maxDataSize, Func func) {
        handleVectorPayload("PUT", pattern, maxDataSize, func);
    }

    template<typename Func>
    void handlePutStringPayload(std::string_view pattern, std::size_t maxDataSize, Func func) {
        handleStringPayload("PUT", pattern, maxDataSize, func);
    }

    // FIXME: Add handlers for custom methods?

    // FIXME: Add handlers for basic auth, not found, method not allowed

  protected:
    RequestResult onIncomingRequest(Internal::InternalRequest& request);

    RequestResult onRequest(Internal::InternalRequest& request);

    RequestResult onRequest(Internal::InternalRequest& request, const char* data, std::size_t* byteCount);

    void onRequestDone(Internal::InternalRequest& request);
};

} // namespace ARo::Http
