#pragma once

#include <cereal/access.hpp>
#include <cereal/cereal.hpp>
#include <string>

namespace mango {
struct URL {
public:
  URL() = default;
  URL(const std::string &url) : url_(url) { toRelative(); }

  URL(std::string &&url) : url_(std::move(url)) { toRelative(); }

  URL(const char *url) : url_(std::string(url)) { toRelative(); }

  bool operator<(const URL &other) const { return url_ < other.url_; }

  bool operator==(const URL &other) const { return url_ == other.url_; }

  bool operator!=(const URL &other) const { return url_ != other.url_; }

  std::string getAbsolute() const;
  std::string getBareName() const;
  std::string getFolder() const;
  std::string getExtension() const;

  bool empty() const;
  void clear();
  const std::string &str() const { return url_; }

  static URL combine(const std::string &lhs, const std::string &rhs);

private:
  friend class cereal::access;
  template <class Archive> void serialize(Archive &ar) {
    ar(cereal::make_nvp("url", url_));
  }

  void toRelative();

  std::string url_;
};
} // namespace mango