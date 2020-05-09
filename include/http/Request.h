#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace ARo::Http {

class Request {
  protected:
    std::string httpVersion_;
    std::string method_;
    std::string url_;
    std::string fullUrl_;

    std::vector<std::uint8_t> requestData_;

  public:
    explicit Request(std::string_view fullUrl);
    virtual ~Request();

    Request(const Request &) = delete;
    Request(Request &&) = delete;
    Request &operator=(const Request &) = delete;
    Request &operator=(Request &&) = delete;

    const std::string &getHttpVersion() const &;
    const std::string &getMethod() const &;
    const std::string &getUrl() const &;
    const std::string &getFullUrl() const &;

    virtual std::optional<std::string> getHeader(std::string_view header) const = 0;
    virtual std::vector<std::string> getHeaderValues(std::string_view header) const = 0;
    virtual std::optional<std::string> getQueryArg(std::string_view key) const = 0;
    virtual std::vector<std::string> getQueryArgValues(std::string_view key) const = 0;

    std::string_view getRequestData() const &;
    std::vector<std::uint8_t>& getRequestDataAsBytes() &;
    const std::vector<std::uint8_t>& getRequestDataAsBytes() const &;

//    Response respondWithString(int responseCode, std::string_view contentType, std::string_view data);
};

}

