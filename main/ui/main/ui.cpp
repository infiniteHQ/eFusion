#include "ui.hpp"

#include <iostream>

SampleAppWindow::SampleAppWindow(const std::string &name) {
  m_AppWindow = std::make_shared<AppWindow>(name, name);
  m_AppWindow->SetIcon("/usr/local/include/Vortex/imgs/vortex.png");
  std::shared_ptr<AppWindow> win = m_AppWindow;

  this->ctx = VortexMaker::GetCurrentContext();
}

std::shared_ptr<Cherry::AppWindow> &SampleAppWindow::GetAppWindow() {
  return m_AppWindow;
}

std::shared_ptr<SampleAppWindow>
SampleAppWindow::Create(const std::string &name) {
  auto instance = std::shared_ptr<SampleAppWindow>(new SampleAppWindow(name));
  instance->SetupRenderCallback();
  return instance;
}

void SampleAppWindow::SetupRenderCallback() {
  auto self = shared_from_this();
  m_AppWindow->SetRenderCallback([self]() {
    if (self) {
      self->Render();
    }
  });
}

void SampleAppWindow::Render() { CherryKit::TitleOne("OU"); }
