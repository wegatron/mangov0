#pragma once

#include <memory>
#include <vector>

namespace mango {
class Editor {
public:
  void init();
  void destroy();
  void run();

private:
  void constructUI();

  // std::vector<std::shared_ptr<class EditorUI>> editor_uis_;
  // std::shared_ptr<class EditorUI> simulation_ui_;
};
} // namespace mango