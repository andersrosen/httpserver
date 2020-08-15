#include "../include/http/Response.h"

using namespace ARo::Http;


// 2xx success
const Response::Status Response::Status::ok{200, "OK"};
const Response::Status Response::Status::created{201, "Created"};
const Response::Status Response::Status::accepted{202, "Accepted"};
const Response::Status Response::Status::nonAuthoritativeInformation{203, "Non-authoritative information"};
const Response::Status Response::Status::noContent{204, "No content"};
const Response::Status Response::Status::resetContent{205, "Reset content"};
const Response::Status Response::Status::partialContent{206, "Partial content"};
const Response::Status Response::Status::multiStatus{207, "Multi-status"};
const Response::Status Response::Status::alreadyReported{208, "Already reported"};
const Response::Status Response::Status::imUsed{226, "IM Used"};

// 3xx redirection
const Response::Status Response::Status::multipleChoices{300, "Multiple choices"};
const Response::Status Response::Status::movedPermanently{301, "Moved permanently"};
const Response::Status Response::Status::found{302, "Found"};
const Response::Status Response::Status::seeOther{303, "See other"};
const Response::Status Response::Status::notModified{304, "Not modified"};
const Response::Status Response::Status::useProxy{305, "Use proxy"};
const Response::Status Response::Status::switchProxy{306, "Switch proxy"};
const Response::Status Response::Status::temporaryRedirect{307, "Temporary redirect"};
const Response::Status Response::Status::permanentRedirect{308, "Permanent redirect"};

const Response::Status Response::Status::badRequest{400, "Bad request"};
const Response::Status Response::Status::unauthorized{401, "Unauthorized"};
const Response::Status Response::Status::paymentRequired{402, "Payment required"};
const Response::Status Response::Status::forbidden{403, "Forbidden"};
const Response::Status Response::Status::notFound{404, "Not found"};
const Response::Status Response::Status::methodNotAllowed{405, "Method not allowed"};
const Response::Status Response::Status::notAcceptable{406, "Not acceptable"};
const Response::Status Response::Status::proxyAuthenticationRequired{407, "Proxy authentication required"};
const Response::Status Response::Status::requestTimeout{408, "Request timeout"};
const Response::Status Response::Status::conflict{409, "Conflict"};
const Response::Status Response::Status::gone{410, "Gone"};
const Response::Status Response::Status::lengthRequired{411, "Length required"};
const Response::Status Response::Status::preconditionFailed{412, "Precondition failed"};
const Response::Status Response::Status::payloadTooLarge{413, "Payload too large"};
const Response::Status Response::Status::uriTooLong{414, "URI too long"};
const Response::Status Response::Status::unsupportedMediaType{415, "Unsupported media type"};
const Response::Status Response::Status::rangeNotSatisfiable{416, "Range not satisfiable"};
const Response::Status Response::Status::expectationFailed{417, "Expectation failed"};
const Response::Status Response::Status::imATeaPot{418, "I'm a teapot"};
const Response::Status Response::Status::misdirectedRequest{421, "Misdirected request"};
const Response::Status Response::Status::unprocessableEntity{422, "Unprocessable entity"};
const Response::Status Response::Status::locked{423, "Locked"};
const Response::Status Response::Status::failedDependency{424, "Failed dependency"};
const Response::Status Response::Status::tooEarly{425, "Too early"};
const Response::Status Response::Status::upgradeRequired{426, "Upgrade required"};
const Response::Status Response::Status::preconditionRequired{428, "Precondition required"};
const Response::Status Response::Status::tooManyRequests{429, "Too many requests"};
const Response::Status Response::Status::requestHeaderFieldsTooLarge{431, "Request header fields too large"};
const Response::Status Response::Status::unavailableForLegalReasons{451, "Unavailable for legal reasons"};

const Response::Status Response::Status::internalServerError{500, "Internal server error"};
const Response::Status Response::Status::notImplemented{501, "Not implemented"};
const Response::Status Response::Status::badGateway{502, "Bad gateway"};
const Response::Status Response::Status::serviceUnavailable{503, "Service unavailable"};
const Response::Status Response::Status::gatewayTimeout{504, "Gateway timeout"};
const Response::Status Response::Status::httpVersionNotSupported{505, "HTTP version not supported"};
const Response::Status Response::Status::variantAlsoNegotiates{506, "Variant also negotiates"};
const Response::Status Response::Status::insufficientStorage{507, "Insufficient storage"};
const Response::Status Response::Status::loopDetected{508, "Loop detected"};
const Response::Status Response::Status::notExtended{510, "Not extended"};
const Response::Status Response::Status::networkAuthenticationRequired{511, "Network authentication required"};

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

BinaryResponse::BinaryResponse(Status status, const std::vector<std::uint8_t>& body)
: Response(Type::Buffer, std::move(status))
{
    body_.assign(reinterpret_cast<const char*>(body.data()), body.size());
}

BinaryResponse::BinaryResponse(Status status, HeaderMap headers, const std::vector<std::uint8_t>& body)
: Response(Type::Buffer, std::move(status), std::move(headers))
{
    body_.assign(reinterpret_cast<const char*>(body.data()), body.size());
}

BinaryResponse::BinaryResponse(Status status, std::string_view contentType, const std::vector<std::uint8_t>& body)
: Response(Type::Buffer, std::move(status))
{
    headers["Content-Type"] = std::string(contentType);
    body_.assign(reinterpret_cast<const char*>(body.data()), body.size());
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

