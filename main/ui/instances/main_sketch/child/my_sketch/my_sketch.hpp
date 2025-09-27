#pragma once
#include "../../../../../../lib/vortex/main/include/vortex.h"
#include "../../../../../../lib/vortex/main/include/vortex_internals.h"

#ifndef MY_SKETCH_MAIN_SKETCH_APP_WINDOW_HPP
#define MY_SKETCH_MAIN_SKETCH_APP_WINDOW_HPP

namespace ModuleUI {

class MySketchMainSketchAppWindow
    : public std::enable_shared_from_this<MySketchMainSketchAppWindow> {
public:
  MySketchMainSketchAppWindow(const std::string &path, const std::string &name);

  void menubar();
  std::shared_ptr<Cherry::AppWindow> &GetAppWindow();
  static std::shared_ptr<MySketchMainSketchAppWindow>
  Create(const std::string &path, const std::string &name);
  void SetupRenderCallback();
  void Render();
  void RenderMenubar();
  void RenderRightMenubar();

  void Refresh();
  void Save();
  void Undo();
  void Redo();

private:
  VxContext *ctx;
  bool opened;

  // Cherry
  std::shared_ptr<Cherry::AppWindow> m_AppWindow;
  ComponentsPool m_ComponentPool;
};

}; // namespace ModuleUI

#endif // LOGUTILITY_H
