#include "main_sketch.hpp"
#include "../../../src/module.hpp"

#include <filesystem>
#include <iostream>
#include <string>

namespace ModuleUI {
MainSketchAppWindow::MainSketchAppWindow(const std::string &path,
                                         const std::string &name) {
  namespace fs = std::filesystem;

  m_Type = detect_file(path);
  m_AppWindow = std::make_shared<Cherry::AppWindow>(name, name);
  std::string image_name = GetFileTypeStr(m_Type) + ".png";
  m_AppWindow->SetIcon(
      SampleCppModule::GetPath("/resources/files/" + image_name));
  m_AppWindow->SetLeftMenubarCallback([this]() { RenderMenubar(); });
  m_AppWindow->SetRightMenubarCallback([this]() { RenderRightMenubar(); });
  m_AppWindow->SetSaveMode(true);

  std::shared_ptr<Cherry::AppWindow> win = m_AppWindow;
  m_FilePath = path;

  RefreshFile();

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
  CherryGUI::SetCursorPosX(CherryGUI::GetCursorPosX() + 3.0f);
  CherryNextComponent.SetProperty("padding_y", "6.0f");
  CherryNextComponent.SetProperty("padding_x", "10.0f");

  if (!m_FileEdited) {
    CherryGUI::BeginDisabled();
  }

  if (CherryKit::ButtonImageText(
          "Save", SampleCppModule::GetPath("/resources/icons/icon_save.png"))
          .GetDataAs<bool>("isClicked")) {
    m_SavePending = true;
  }

  if (!m_FileEdited) {
    CherryGUI::EndDisabled();
  }

  CherryNextComponent.SetProperty("padding_y", "6.0f");
  CherryNextComponent.SetProperty("padding_x", "10.0f");
  if (CherryKit::ButtonImageText(
          "Refresh",
          SampleCppModule::GetPath("/resources/icons/icon_refresh.png"))
          .GetDataAs<bool>("isClicked")) {
    m_RefreshReady = true;
  }

  if (m_AutoRefresh) {
    CherryKit::TextSimple("Auto refresh activated");
  }
}

void MainSketchAppWindow::Render() {
  CherryApp.PushComponentPool(&m_ComponentPool);

  auto test = CherryGUI::GetContentRegionAvail();
  auto &editor = ModuleUI::TextArea(&test.x, &test.y, &m_FileEditBuffer);

  if (!m_FileUpdated) {
    if (editor.GetDataAs<bool>("text_changed")) {
      m_FileEdited = true;
    }
  } else {
    m_FileUpdated = false;
  }

  if (m_FileEdited) {
    this->m_AppWindow->SetSaved(false);
  } else {
    this->m_AppWindow->SetSaved(true);
  }

  if (editor.GetData("save_ready") == "true") {
    m_SaveReady = true;
    editor.SetData("save_ready", "false");
  }

  if (m_SavePending) {
    editor.SetProperty("save_pending", "true");
    m_SavePending = false;
  }

  if (m_UndoPending) {
    editor.SetProperty("undo_pending", "true");
    m_UndoPending = false;
  }

  if (m_RedoPending) {
    editor.SetProperty("redo_pending", "true");
    m_RedoPending = false;
  }

  if (m_SaveReady) {
    SaveFile();
    m_SaveReady = false;
  }

  if (m_AutoRefresh && !m_FilePath.empty()) {
    namespace fs = std::filesystem;
    std::error_code ec;
    auto currentWriteTime = fs::last_write_time(m_FilePath, ec);
    if (!ec && currentWriteTime != m_LastWriteTime) {
      RefreshFile();
      editor.SetProperty("refresh_pending", "true");
    }
  }

  if (m_RefreshReady) {
    RefreshFile();
    editor.SetProperty("refresh_pending", "true");
    m_RefreshReady = false;
  }

  CherryApp.PopComponentPool();
}

}; // namespace ModuleUI
