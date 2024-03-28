#pragma once

#include <array>
#include <memory>
#include <vector>

namespace mango {
class EditorContext {
public:
  void init();
  void destroy();

  void toggleFullscreen();
  bool isSimulationPanelFullscreen();

private:
  bool m_simulation_panel_fullscreen = false;
};

extern EditorContext g_editor;
} // namespace mango
