#include "../include/http/Response.h"

using namespace ARo::Http;

const Response::Status Response::Status::ok{200, "OK"};
const Response::Status Response::Status::created{201, "Created"};
const Response::Status Response::Status::notFound{404, "Not found"};

Response::~Response() = default;

Response::Response(Status status)
: status_(std::move(status))
{}

Response::Response(Status status, HeaderMap headers)
: status_(std::move(status))
, headers(std::move(headers))
{}

Response::Response(Type type, Status status)
: type_(type)
, status_(std::move(status))
{}

Response::Response(Type type, Status status, HeaderMap headers)
: type_(type)
, status_(std::move(status))
, headers(std::move(headers))
{}

StringResponse::StringResponse(Status status, std::string_view body)
: Response(Type::Buffer, std::move(status))
{
    body_ = body;
}

StringResponse::StringResponse(Status status, HeaderMap headers, std::string_view body)
: Response(Type::Buffer, std::move(status), std::move(headers))
{
    body_ = body;
}

StringResponse::StringResponse(Status status, std::string_view contentType, std::string_view body)
: Response(Type::Buffer, std::move(status))
{
    headers["Content-Type"] = std::string(contentType);
    body_ = body;
}

FileResponse::FileResponse(Status status, std::filesystem::path path)
: Response(Type::File, std::move(status))
{
    filePath_ = std::move(path);
    fileLength_ = static_cast<uint64_t>(std::filesystem::file_size(filePath_));
}

FileResponse::FileResponse(Status status, HeaderMap headers, std::filesystem::path path)
: Response(Type::File, std::move(status), std::move(headers))
{
    filePath_ = std::move(path);
    fileLength_ = static_cast<uint64_t>(std::filesystem::file_size(filePath_));
}

FileResponse::FileResponse(Status status, std::string_view contentType, std::filesystem::path path)
    : Response(Type::File, std::move(status))
{
    headers["Content-Type"] = std::string(contentType);
    filePath_ = std::move(path);
    fileLength_ = static_cast<uint64_t>(std::filesystem::file_size(filePath_));
}

FileResponse::FileResponse(Status status, std::filesystem::path path, std::uint64_t offset, std::uint64_t length)
    : Response(Type::PartialFile, std::move(status))
{
    filePath_ = std::move(path);
    fileOffset_ = offset;
    fileLength_ = length;
}

FileResponse::FileResponse(Status status, HeaderMap headers, std::filesystem::path path, std::uint64_t offset, std::uint64_t length)
    : Response(Type::PartialFile, std::move(status), std::move(headers))
{
    filePath_ = std::move(path);
    fileOffset_ = offset;
    fileLength_ = length;
}

FileResponse::FileResponse(Status status, std::string_view contentType, std::filesystem::path path, std::uint64_t offset, std::uint64_t length)
    : Response(Type::PartialFile, std::move(status))
{
    headers["Content-Type"] = std::string(contentType);
    filePath_ = std::move(path);
    fileOffset_ = offset;
    fileLength_ = length;
}

