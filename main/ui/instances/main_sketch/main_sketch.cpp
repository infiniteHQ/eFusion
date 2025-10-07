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

  m_Path = path;

  // Nodal Viewport
  m_Viewport = ModuleUI::ViewportMainSketchAppWindow::Create(
      path, "Viewport ####viewport" + path);
  m_Viewport->GetAppWindow()->SetParent(m_AppWindow);
  Cherry::AddAppWindow(m_Viewport->GetAppWindow());

  // Nodal Viewport
  m_MySketch = ModuleUI::MySketchMainSketchAppWindow::Create(
      path, "My sketch ####m_MySketch" + path);
  m_MySketch->GetAppWindow()->SetParent(m_AppWindow);
  Cherry::AddAppWindow(m_MySketch->GetAppWindow());

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

void MainSketchAppWindow::RenderMenubar() {

  static bool tt = true;
  static bool first_Frame = true;
  if (tt) {
    if (first_Frame) {
      first_Frame = false;
    } else {
      drag_dropstate = std::make_shared<Cherry::WindowDragDropState>();
      CherryApp.SetCurrentDragDropState(this->drag_dropstate);
      CherryApp.SetCurrentDragDropStateAppWindow(
          m_Viewport->GetAppWindow()->m_IdName);
      CherryApp.SetCurrentDragDropStateWindow(
          Cherry::GetCurrentRenderedWindow()->GetName());
      CherryApp.SetCurrentDragDropStateAppWindowHost(
          m_MySketch->GetAppWindow()->m_IdName);
      CherryApp.SetCurrentDragDropStateDraggingPlace(
          Cherry::DockEmplacement::DockLeft);

      CherryApp.PushRedockEvent(CherryApp.GetCurrentDragDropState());
      tt = false;
    }
  }

  CherryGUI::SetCursorPosX(CherryGUI::GetCursorPosX() + 3.0f);
  CherryNextComponent.SetProperty("padding_y", "6.0f");
  CherryNextComponent.SetProperty("padding_x", "10.0f");
  if (CherryKit::ButtonImageText(
          "Save", GetPath("resources/imgs/icons/misc/icon_add.png"))
          .GetDataAs<bool>("isClicked")) {
    m_Viewport->Save();
  }
  CherryGUI::SetCursorPosX(CherryGUI::GetCursorPosX() + 3.0f);
  CherryNextComponent.SetProperty("padding_y", "6.0f");
  CherryNextComponent.SetProperty("padding_x", "10.0f");
  if (CherryKit::ButtonImageText(
          "Refresh", GetPath("resources/imgs/icons/misc/icon_add.png"))
          .GetDataAs<bool>("isClicked")) {
    m_Viewport->Refresh();
    g_NeedRefresh = true;
  }
  CherryGUI::SetCursorPosX(CherryGUI::GetCursorPosX() + 3.0f);
  CherryNextComponent.SetProperty("padding_y", "6.0f");
  CherryNextComponent.SetProperty("padding_x", "10.0f");
  if (CherryKit::ButtonImageText(
          "Transpilation", GetPath("resources/imgs/icons/misc/icon_add.png"))
          .GetDataAs<bool>("isClicked")) {
    m_Viewport->Transpilation();
  }
}
void MainSketchAppWindow::RenderRightMenubar() {}
void MainSketchAppWindow::Render() {}
}; // namespace ModuleUI
