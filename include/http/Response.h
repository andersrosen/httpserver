#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace ARo::Http {

    namespace Internal {
        class ServerImpl;
    }

    class Response {
        friend class Internal::ServerImpl;
    public:
        struct Status {
            uint16_t code;
            std::string description;

            // 2xx success
            static const Status ok;
            static const Status created;
            static const Status accepted;
            static const Status nonAuthoritativeInformation;
            static const Status noContent;
            static const Status resetContent;
            static const Status partialContent;
            static const Status multiStatus;
            static const Status alreadyReported;
            static const Status imUsed;

            // 3xx redirection
            static const Status multipleChoices;
            static const Status movedPermanently;
            static const Status found;
            static const Status seeOther;
            static const Status notModified;
            static const Status useProxy;
            static const Status switchProxy;
            static const Status temporaryRedirect;
            static const Status permanentRedirect;

            // 4xx Client errors
            static const Status badRequest;
            static const Status unauthorized;
            static const Status paymentRequired;
            static const Status forbidden;
            static const Status notFound;
            static const Status methodNotAllowed;
            static const Status notAcceptable;
            static const Status proxyAuthenticationRequired;
            static const Status requestTimeout;
            static const Status conflict;
            static const Status gone;
            static const Status lengthRequired;
            static const Status preconditionFailed;
            static const Status payloadTooLarge;
            static const Status uriTooLong;
            static const Status unsupportedMediaType;
            static const Status rangeNotSatisfiable;
            static const Status expectationFailed;
            static const Status imATeaPot;
            static const Status misdirectedRequest;
            static const Status unprocessableEntity;
            static const Status locked;
            static const Status failedDependency;
            static const Status tooEarly;
            static const Status upgradeRequired;
            static const Status preconditionRequired;
            static const Status tooManyRequests;
            static const Status requestHeaderFieldsTooLarge;
            static const Status unavailableForLegalReasons;

            // 5xx Server errors
            static const Status internalServerError;
            static const Status notImplemented;
            static const Status badGateway;
            static const Status serviceUnavailable;
            static const Status gatewayTimeout;
            static const Status httpVersionNotSupported;
            static const Status variantAlsoNegotiates;
            static const Status insufficientStorage;
            static const Status loopDetected;
            static const Status notExtended;
            static const Status networkAuthenticationRequired;

        };

    protected:
        enum class Type {
            Buffer,
            File,
            PartialFile,
        };

    private:
        Type type_ = Type::Buffer;
        Status status_;

    public:
        using HeaderMap = std::unordered_map<std::string, std::string>;
        HeaderMap headers;

        explicit Response(Status status);
        Response(Status status, HeaderMap headers);
        virtual ~Response();

    protected:
        std::string body_;
        std::filesystem::path filePath_;
        std::uint64_t fileOffset_;
        std::uint64_t fileLength_;

        explicit Response(Type type, Status status);
        Response(Type type, Status status, HeaderMap headers);
    };

    class StringResponse final : public Response {
    public:
        StringResponse(Status status, std::string_view body);
        StringResponse(Status status, HeaderMap headers, std::string_view body);
        StringResponse(Status status, std::string_view contentType, std::string_view body);
    };

    class BinaryResponse final : public Response {
    public:
        BinaryResponse(Status status, const std::vector<std::uint8_t>& body);
        BinaryResponse(Status status, HeaderMap headers, const std::vector<std::uint8_t>& body);
        BinaryResponse(Status status, std::string_view contentType, const std::vector<std::uint8_t>& body);
        // FIXME! Start & End iterator versions
    };

    class FileResponse final : public Response {
    public:
        FileResponse(Status status, std::filesystem::path path);
        FileResponse(Status status, HeaderMap headers, std::filesystem::path path);
        FileResponse(Status status, std::string_view contentType, std::filesystem::path path);

        FileResponse(Status status, std::filesystem::path path, std::uint64_t offset, std::uint64_t length);
        FileResponse(Status status, HeaderMap headers, std::filesystem::path path, std::uint64_t offset, std::uint64_t length);
        FileResponse(Status status, std::string_view contentType, std::filesystem::path path, std::uint64_t offset, std::uint64_t length);
    };

} // namespace ARo::Http