#include "InternalRequest.h"

#include <algorithm>
#include <iostream>

namespace ARo::Http::Internal {

static int
headerCallback(void* cls, MHD_ValueKind kind, const char* key, const char* value) {
    auto map = static_cast<InternalRequest::KeyValues*>(cls);

    std::string k;
    for (auto c = key; *c != '\0'; ++c)
        k += static_cast<char>(::tolower(*c));
    map->insert(std::pair<std::string, std::string>(k, value ? value : ""));

    return MHD_YES;
}

static int
queryArgCallback(void* cls, MHD_ValueKind kind, const char* key, const char* value) {
    auto map = static_cast<InternalRequest::KeyValues*>(cls);

    map->insert(std::pair<std::string, std::string>(key, value ? value : ""));

    return MHD_YES;
}

InternalRequest::InternalRequest(const char* fullUrl)
: Request(fullUrl)
{}

std::string
InternalRequest::toLowerCase(std::string_view s) {
    std::string ret;
    for (auto ch : s)
        ret += static_cast<char>(::tolower(ch));
    return ret;
}

void
InternalRequest::populateHeaders() const {
    if (haveHeaders_)
        return;

    haveHeaders_ = true;

    // FIXME: handle error
    MHD_get_connection_values(connection_, MHD_HEADER_KIND, headerCallback, static_cast<void*>(&headers_));
}

void
InternalRequest::populateQueryArgs() const {
    if (haveQueryArgs_)
        return;

    haveQueryArgs_ = true;

    // FIXME: handle error
    MHD_get_connection_values(connection_, MHD_GET_ARGUMENT_KIND, queryArgCallback, static_cast<void*>(&queryArgs_));
}

std::optional<std::string>
InternalRequest::getHeader(std::string_view header) const {
    populateHeaders();

    auto it = headers_.find(toLowerCase(header));
    if (it == headers_.end())
        return std::nullopt;
    return it->second;
}

std::vector<std::string>
InternalRequest::getHeaderValues(std::string_view header) const {
    populateHeaders();

    auto [start, end] = headers_.equal_range(toLowerCase(header));
    std::vector<std::string> result;
    std::for_each(start, end, [&result](auto &val) {
        result.push_back(val.second);
    });
    return result;
}

std::optional<std::string>
InternalRequest::getQueryArg(std::string_view key) const {
    populateQueryArgs();

    auto it = queryArgs_.find(std::string(key));
    if (it == queryArgs_.end())
        return std::nullopt;
    return it->second;
}

std::vector<std::string>
InternalRequest::getQueryArgValues(std::string_view key) const {
    populateQueryArgs();

    auto [start, end] = queryArgs_.equal_range(std::string(key));
    std::vector<std::string> result;
    std::for_each(start, end, [&result](auto &val) {
        result.push_back(val.second);
    });
    return result;
}

MHD_Connection*
InternalRequest::getConnection() const {
    return connection_;
}

void
InternalRequest::setConnection(MHD_Connection* connection) {
    connection_ = connection;
}

void
InternalRequest::setUrl(const char* url) {
    url_ = url;
}

void
InternalRequest::setHttpVersion(const char* httpVersion) {
    httpVersion_ = httpVersion;
}

void
InternalRequest::setMethod(const char* method) {
    method_ = method;
}

} // namespace ARo::Http::Internal
