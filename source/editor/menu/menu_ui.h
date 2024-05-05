#pragma once

#include <editor/base/editor_ui.h>
#include <editor/base/folder_tree_ui.h>

namespace mango {
class MenuUI : public EditorUI, public IFolderTreeUI {
public:
  virtual void init() override;
  virtual void construct() override;

private:
  void constructFileMenu();
  void constructEditMenu();
  void constructViewMenu();
  void constructHelpMenu();

  void pollShortcuts();

  void importScene();
  void newWorld();
  void openWorld();
  void saveWorld();
  void saveAsWorld();
  void quit();

  void undo();
  void redo();
  void cut();
  void copy();
  void paste();

  void editorSettings();
  void projectSettings();

  void constructTemplateWorldPanel();
  void constructWorldURLPanel();

  void clearEntitySelection();

  std::string m_layout_path;

  bool showing_new_world_popup = false;
  bool showing_open_world_popup = false;
  bool showing_save_as_world_popup = false;
  bool showing_import_scene_popup = false;

  // new world
  struct TemplateWorld {
    std::string name;
    std::string url;
    std::shared_ptr<ImGuiImage> icon;
  };
  std::vector<TemplateWorld> template_worlds_;
  std::map<std::string, HoverState> template_world_hover_states_;
  uint32_t selected_template_world_index_;

  // open world
  std::vector<std::string> current_world_urls_;
  std::string selected_world_url_;
};
} // namespace mango