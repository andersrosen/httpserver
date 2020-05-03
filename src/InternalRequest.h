#pragma once

#include <microhttpd.h>
#include <map>

#include "../include/http/Request.h"

namespace ARo::Http::Internal {

class ServerImpl;

class InternalRequest final : public ARo::Http::Request {
  public:
    friend class ServerImpl;

    using KeyValues = std::multimap<std::string, std::string>;

  private:
    MHD_Connection* connection_ = nullptr;

    mutable bool haveHeaders_ = false;
    mutable KeyValues headers_;
    mutable bool haveQueryArgs_ = false;
    mutable KeyValues queryArgs_;

    InternalRequest(const char* fullUrl);

    static std::string toLowerCase(std::string_view s);

    void populateHeaders() const;
    void populateQueryArgs() const;

  public:
    MHD_Connection *getConnection() const;
    void setConnection(MHD_Connection *connection);
    void setUrl(const char *url);
    void setHttpVersion(const char *httpVersion);
    void setMethod(const char *method);

    std::optional<std::string> getHeader(std::string_view header) const override;
    std::vector<std::string> getHeaderValues(std::string_view header) const override;
    std::optional<std::string> getQueryArg(std::string_view key) const override;
    std::vector<std::string> getQueryArgValues(std::string_view key) const override;
};

} // namespace ARo::Http::Internal