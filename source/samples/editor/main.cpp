#include <editor/editor.h>

int main(int argc, char **argv) {
  mango::Editor *editor = new mango::Editor;
  editor->init();
  editor->run();
  editor->destroy();
  delete editor;
  return 0;
}