#pragma once

#include <any>
#include <map>
#include <memory>
#include <microhttpd.h>
#include <regex>

#include "../include/http/Request.h"

namespace ARo::Http::Internal {

class ServerImpl;
class HandlerBase;

class InternalRequest final : public ARo::Http::Request {
  public:
    friend class ServerImpl;

    using KeyValues = std::multimap<std::string, std::string>;
    enum class State {
        Uninitialized, // The request object is mostly uninitialized. The only thing available is the full URL
        Initialized, // The object is initialized. The callback has not yet been called
        Ongoing // The request handling is ongoing. The request callback has already been called at least once
    };

    std::optional<HandlerBase*> handler;
    std::smatch match;
    std::vector<std::uint8_t> payload;
    std::any userData;

  private:
    State state_ = State::Uninitialized;

    MHD_Connection* connection_ = nullptr;

    mutable bool haveHeaders_ = false;
    mutable KeyValues headers_;
    mutable bool haveQueryArgs_ = false;
    mutable KeyValues queryArgs_;


    explicit InternalRequest(const char* fullUrl);

    static std::string toLowerCase(std::string_view s);

    void populateHeaders() const;
    void populateQueryArgs() const;

  public:
    State getState() const;
    void setState(State state);
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