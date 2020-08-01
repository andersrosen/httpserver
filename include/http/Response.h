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

            static const Status ok;
            static const Status created;
            static const Status notFound;
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