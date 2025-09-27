#include "main_sketch.hpp"
#include "../../../src/module.hpp"

#include <filesystem>
#include <iostream>
#include <string>

namespace ModuleUI {
MainSketchAppWindow::MainSketchAppWindow(const std::string &path,
                                         const std::string &name) {
  namespace fs = std::filesystem;

  m_AppWindow = std::make_shared<Cherry::AppWindow>(name, name);
  m_AppWindow->SetIcon(
      EmbeddedFusion::GetPath("/resources/icons/main_sketch.png"));
  m_AppWindow->SetLeftMenubarCallback([this]() { RenderMenubar(); });
  m_AppWindow->SetRightMenubarCallback([this]() { RenderRightMenubar(); });
  m_AppWindow->SetSaveMode(true);
  m_AppWindow->SetDockingMode(true);

  // Nodal Viewport
  std::shared_ptr<ModuleUI::ViewportMainSketchAppWindow> viewport =
      ModuleUI::ViewportMainSketchAppWindow::Create(
          path, "Viewport ####viewport" + path);
  viewport->GetAppWindow()->SetParent(m_AppWindow);
  Cherry::AddAppWindow(viewport->GetAppWindow());

  // Nodal Viewport
  std::shared_ptr<ModuleUI::MySketchMainSketchAppWindow> my_sketch =
      ModuleUI::MySketchMainSketchAppWindow::Create(
          path, "My sketch ####my_sketch" + path);
  my_sketch->GetAppWindow()->SetParent(m_AppWindow);
  Cherry::AddAppWindow(my_sketch->GetAppWindow());

  this->ctx = VortexMaker::GetCurrentContext();
}

std::shared_ptr<Cherry::AppWindow> &MainSketchAppWindow::GetAppWindow() {
  return m_AppWindow;
}

std::shared_ptr<MainSketchAppWindow>
MainSketchAppWindow::Create(const std::string &path, const std::string &name) {
  auto instance =
      std::shared_ptr<MainSketchAppWindow>(new MainSketchAppWindow(path, name));
  instance->SetupRenderCallback();
  return instance;
}

void MainSketchAppWindow::SetupRenderCallback() {
  auto self = shared_from_this();
  m_AppWindow->SetRenderCallback([self]() {
    if (self) {
      self->Render();
    }
  });
}

void MainSketchAppWindow::RenderMenubar() {}
void MainSketchAppWindow::RenderRightMenubar() {}
void MainSketchAppWindow::Render() {}

}; // namespace ModuleUI
