#pragma once

#include <functional>
#include <memory>
#include <vector>

namespace mango {
class Editor {
public:
  void init();
  void destroy();
  // exit_check: called after each frame; return true to stop the loop early.
  void run(std::function<bool()> exit_check = nullptr);

private:
  void constructUI();

  std::vector<std::shared_ptr<class EditorUI>> editor_uis_;
  std::shared_ptr<class EditorUI> simulation_ui_;
};
} // namespace mango