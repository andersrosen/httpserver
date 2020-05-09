#include "../include/http/Request.h"

namespace ARo::Http {

Request::Request(std::string_view fullUrl)
    : fullUrl_(fullUrl)
{}

Request::~Request() = default;

const std::string&
Request::getHttpVersion() const & {
    return httpVersion_;
}

const std::string&
Request::getMethod() const & {
    return method_;
}

const std::string&
Request::getUrl() const & {
    return url_;
}

const std::string&
Request::getFullUrl() const & {
    return fullUrl_;
}

std::string_view
Request::getRequestData() const & {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return std::string_view(reinterpret_cast<const char*>(requestData_.data()), requestData_.size());
}

std::vector<std::uint8_t>&
Request::getRequestDataAsBytes() & {
    return requestData_;
}

const std::vector<std::uint8_t>&
Request::getRequestDataAsBytes() const & {
    return requestData_;
}

} // namespace ARo::Http

