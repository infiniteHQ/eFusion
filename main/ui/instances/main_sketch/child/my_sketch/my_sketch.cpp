#include "my_sketch.hpp"
#include "../../../../../src/module.hpp"

#include <filesystem>
#include <iostream>
#include <string>

namespace ModuleUI {
MySketchMainSketchAppWindow::MySketchMainSketchAppWindow(
    const std::string &path, const std::string &name) {
  namespace fs = std::filesystem;

  m_AppWindow = std::make_shared<Cherry::AppWindow>(name, name);
  m_AppWindow->SetIcon(
      EmbeddedFusion::GetPath("/resources/icons/viewport.png"));

  this->ctx = VortexMaker::GetCurrentContext();
}

std::shared_ptr<Cherry::AppWindow> &
MySketchMainSketchAppWindow::GetAppWindow() {
  return m_AppWindow;
}

std::shared_ptr<MySketchMainSketchAppWindow>
MySketchMainSketchAppWindow::Create(const std::string &path,
                                    const std::string &name) {
  auto instance = std::shared_ptr<MySketchMainSketchAppWindow>(
      new MySketchMainSketchAppWindow(path, name));
  instance->SetupRenderCallback();
  return instance;
}

void MySketchMainSketchAppWindow::SetupRenderCallback() {
  auto self = shared_from_this();
  m_AppWindow->SetRenderCallback([self]() {
    if (self) {
      self->Render();
    }
  });
}

void MySketchMainSketchAppWindow::Render() {
  CherryKit::TextSimple("Hello world from my sketch");
}

}; // namespace ModuleUI
