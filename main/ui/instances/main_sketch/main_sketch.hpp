#pragma once
#include "../../../../lib/vortex/main/include/vortex.h"
#include "../../../../lib/vortex/main/include/vortex_internals.h"
#include "./child/my_sketch/my_sketch.hpp"
#include "./child/viewport/viewport.hpp"

#ifndef MAIN_SKETCH_APP_WINDOW_HPP
#define MAIN_SKETCH_APP_WINDOW_HPP

namespace ModuleUI {

class MainSketchAppWindow
    : public std::enable_shared_from_this<MainSketchAppWindow> {
public:
  MainSketchAppWindow(const std::string &path, const std::string &name);

  void menubar();
  std::shared_ptr<Cherry::AppWindow> &GetAppWindow();
  static std::shared_ptr<MainSketchAppWindow> Create(const std::string &path,
                                                     const std::string &name);
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

  std::vector<std::function<void()>> m_SaveCallbacks;
  std::vector<std::function<void()>> m_RefreshCallbacks;

  std::shared_ptr<Cherry::WindowDragDropState> drag_dropstate;
  // Instances
  std::shared_ptr<ModuleUI::ViewportMainSketchAppWindow> m_Viewport;
  std::shared_ptr<ModuleUI::MySketchMainSketchAppWindow> m_MySketch;

  // Cherry
  std::shared_ptr<Cherry::AppWindow> m_AppWindow;
  ComponentsPool m_ComponentPool;

  std::string m_Path;
};

}; // namespace ModuleUI

#endif // LOGUTILITY_H
