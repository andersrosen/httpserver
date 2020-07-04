#pragma once

#include <cstdint>
#include <regex>
#include <string>
#include <vector>
#include <any>

#include "../Request.h"

namespace ARo::Http::Internal {

    // Provides the signature of a callback as a function pointer type of
    // the form ReturnType (*) (Arguments...) or ReturnType (ClassName::*) (Arguments...)
    //
    // The resulting type is available in the "type" type alias
    template <typename T, typename Enable = void>
    class Signature;

    template <typename T>
    struct Signature<T, typename std::enable_if_t<!std::is_pointer_v<T>>>
    {
        // Since function pointers don't have operator(), we use is_pointer_v
        // to tell them apart from functors/lambdas
        using type = decltype(&std::remove_reference<T>::type::operator());
    };

    template <typename T>
    struct Signature<T, typename std::enable_if_t<std::is_pointer_v<T>>>
    {
        // Since function pointers don't have operator(), we use is_pointer_v
        // to tell them apart from functors/lambdas
        using type = T;
    };

    template <typename T>
    using SignatureT = typename Signature<T>::type;

    // Simplifies the signature of a callback as a function pointer type
    // of the form ReturnType (*) (Arguments)
    // If a pointer to a functor's operator(), a member function pointer
    // or lambda is passed in, they will be simplified to look as a free
    // function of the pattern above.
    //
    // The resulting type is available in the "type" type alias
    template <typename>
    struct SimplifiedSignature;

    template <typename ReturnType, typename... Args>
    struct SimplifiedSignature<ReturnType (*)(Args...)> {
        // Free function pointers
        using type = ReturnType (*)(Args...);
    };
    template <typename ReturnType, typename Classname, typename... Args>
    struct SimplifiedSignature<ReturnType (Classname::*)(Args...)> {
        // Strip class from pointer to member function (can be operator() or another func)
        using type = ReturnType (*)(Args...);
    };
    template <typename ReturnType, typename Classname, typename... Args>
    struct SimplifiedSignature<ReturnType (Classname::*)(Args...) const> {
        // Strip class from pointer to const member function (can be operator() or another func)
        using type = ReturnType (*)(Args...);
    };

    template <typename T>
    using SimplifiedSignatureT = typename SimplifiedSignature<T>::type;

    enum class PayloadHandling {
        Ignore,
        BufferedString,
        BufferedBinary,
        RawHandling,
    };

    class HandlerBase {
      public:
        const std::size_t urlParameterCount;
        const std::regex regex;
        const std::string method;
        const PayloadHandling payloadHandling;

      protected:
        HandlerBase(std::string_view method, std::string_view pattern, std::size_t paramCount, PayloadHandling payloadHandling)
            : urlParameterCount(paramCount)
            , regex(std::string(pattern))
            , method(method)
            , payloadHandling(payloadHandling)
        {}

      public:
        virtual ~HandlerBase() = default;

        inline bool canHandle(Request& req) {
            return true;
        }

        virtual void run(const std::smatch &match, ARo::Http::Request &req) const {}
        virtual void run(const std::smatch &match, std::string_view payload, ARo::Http::Request &req) const {}
        virtual void run(const std::smatch &match, const std::vector<std::uint8_t>& payload, ARo::Http::Request &req) const {}
        virtual void run(const std::smatch &match, const char *payloadData, std::size_t &size, std::any &userData, ARo::Http::Request &req) const {}
    };

    template <typename, typename> struct HandlerT;

    // The handlers below are differentiated by the signature of the handler callback
    // function provided by the user.
    //
    // The simplified signature of the callback is used for matching the specializations below,
    // and for counting the number of Url parameter strings the callback expects.
    //
    // The matching scheme is as follows:
    // The first arguments relate to the handling of request payload. Then there is a reference to the request object,
    // followed by an arbitrary number of string arguments, which correspond to matches in the URL against the regex.
    //
    // Payload handling arguments:
    // None:                                          Payload is ignored. This is used for example when handling GET requests
    // std::string_view or std::vector<std::uint8_t>: The entire payload is provided as a string view or binary vector
    // const char *, std::size_t &, std::any &:       The handler will be called one or more times with chunks of data
    //                                                as they become available. The user of the API is expected to store
    //                                                information specific to the request in the std::any reference provided
    //                                                - this is needed since multiple requests to the same handler might
    //                                                be active at any time. The size_t argument must be updated to reflect
    //                                                the number of bytes that were consumed by the handler during the call.
    // TODO: Handler that uses MHD's PostData processor


    //
    // Handlers that ignore the payload
    //
    template <typename FuncType, typename... Rest>
    struct HandlerT<FuncType, void (*)(ARo::Http::Request &, Rest...)>
            : public HandlerBase
    {
        enum { UrlParameterCount = sizeof...(Rest) };
        FuncType func;

        HandlerT(std::string_view method, std::string_view pattern, FuncType f)
                : HandlerBase(method, pattern, UrlParameterCount, PayloadHandling::Ignore)
                , func(f)
        {}

        template <std::size_t... Is>
        void do_run(const std::smatch &match, ARo::Http::Request &req, std::index_sequence<Is...>) const {
            func(req, match[Is + 1].str()...);
        }
        void run(const std::smatch &match, ARo::Http::Request &req) const final {
            do_run(match, req, std::make_index_sequence<UrlParameterCount>{});
        }
    };

    //
    // Handlers that take the entire payload as a std::string_view
    //
    template <typename FuncType, typename... Rest>
    struct HandlerT<FuncType, void (*)(std::string_view, ARo::Http::Request &, Rest...)>
            : public HandlerBase {
        enum { UrlParameterCount = sizeof...(Rest) };

        FuncType func;

        HandlerT(std::string_view method, std::string_view pattern, FuncType f)
                : HandlerBase(method, pattern, UrlParameterCount, PayloadHandling::BufferedString)
                , func(f)
        {}

        template <std::size_t... Is>
        void do_run(const std::smatch &match, std::string_view &payload, ARo::Http::Request &req, std::index_sequence<Is...>) const {
            func(payload, req, match[Is + 1].str()...);
        }
        void run(const std::smatch &match, std::string_view payload, ARo::Http::Request &req) const final {
            do_run(match, payload, req, std::make_index_sequence<UrlParameterCount>{});
        }
    };

    //
    // Handlers that take the entire payload as a std::vector<uint8_t>
    //
    template <typename FuncType, typename... Rest>
    struct HandlerT<FuncType, void (*)(const std::vector<std::uint8_t>&, ARo::Http::Request &, Rest...)>
            : public HandlerBase {
        enum { UrlParameterCount = sizeof...(Rest) };

        FuncType func;

        HandlerT(std::string_view method, std::string_view pattern, FuncType f)
                : HandlerBase(method, pattern, UrlParameterCount, PayloadHandling::BufferedBinary)
                , func(f)
        {}

        template <std::size_t... Is>
        void do_run(const std::smatch &match, const std::vector<std::uint8_t> &payload, ARo::Http::Request &req, std::index_sequence<Is...>) const {
            func(payload, req, match[Is + 1].str()...);
        }
        void run(const std::smatch &match, const std::vector<std::uint8_t> &payload, ARo::Http::Request &req) const final {
            do_run(match, payload, req, std::make_index_sequence<UrlParameterCount>{});
        }
    };

    //
    // Handlers that take payload by repeatedly being called with chunks of data.
    //
    template <typename FuncType, typename... Rest>
    struct HandlerT<FuncType, void (*)(const char *, std::size_t &, std::any &, ARo::Http::Request &, Rest...)>
            : public HandlerBase {
        enum { UrlParameterCount = sizeof...(Rest) };

        FuncType func;

        HandlerT(std::string_view method, std::string_view pattern, FuncType f)
                : HandlerBase(method, pattern, UrlParameterCount)
                , func(f)
        {}

        template <std::size_t... Is>
        void do_run(const std::smatch &match, const char *payloadData, std::size_t &size, std::any &userData, ARo::Http::Request &req, std::index_sequence<Is...>) const {
            func(payloadData, size, userData, req, match[Is + 1].str()...);
        }

        void run(const std::smatch &match, const char *payloadData, std::size_t &size, std::any &userData, ARo::Http::Request &req) const final {
            do_run(match, payloadData, size, userData, req, std::make_index_sequence<UrlParameterCount>{});
        }
    };

    template <typename T>
    using Handler = HandlerT<T, SimplifiedSignatureT<SignatureT<T>>>;

} // namespace ARo::Http::Internal
