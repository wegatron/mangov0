#include <engine/asset/url.h>
#include <engine/utils/base/macro.h>
#include <engine/functional/global/engine_context.h>

namespace mango {
std::string URL::getAbsolute() const { return TO_ABSOLUTE(url_); }

std::string URL::getBareName() const {
  std::string name = g_engine.getFileSystem()->basename(url_);
  std::string::size_type underline_pos = name.find_first_of('_');
  return name.substr(underline_pos + 1, name.length() - (underline_pos + 1));
}

std::string URL::getFolder() const { return g_engine.getFileSystem()->dir(url_); }

bool URL::empty() const { return url_.empty(); }

void URL::clear() { url_.clear(); }

URL URL::combine(const std::string &lhs, const std::string &rhs) {
  return URL(g_engine.getFileSystem()->combine(lhs, rhs));
}

void URL::toRelative() {
  if (!url_.empty() && url_[0] == '.') {
    url_ = g_engine.getFileSystem()->relative(url_);
  }
}

} // namespace Bamboo