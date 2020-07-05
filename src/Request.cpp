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

} // namespace ARo::Http

