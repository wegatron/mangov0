#pragma once

#include "editor/base/editor_ui.h"

namespace mango {
class LogUI : public EditorUI {
public:
  virtual void init() override;
  virtual void construct() override;
  virtual void destroy() override;

private:
  void showLogText(const std::string &log);
};
} // namespace mango