#include "text_editor.hpp"
#include "../../../src/module.hpp"

#include <filesystem>
#include <iostream>
#include <string>

namespace ModuleUI {
TextEditorAppWindow::TextEditorAppWindow(const std::string &path,
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

std::string TextEditorAppWindow::GetFileTypeStr(FileTypes type) {
  switch (type) {
  // Web and Markup
  case FileTypes::File_XML:
    return "file_xml";

  // Config
  case FileTypes::File_CFG:
    return "file_cfg";
  case FileTypes::File_JSON:
    return "file_json";
  case FileTypes::File_YAML:
    return "file_yaml";
  case FileTypes::File_INI:
    return "file_ini";

  // Documents
  case FileTypes::File_TXT:
    return "file_txt";
  case FileTypes::File_MD:
    return "file_md";

  // Miscellaneous
  case FileTypes::File_LOG:
    return "file_log";
  case FileTypes::File_BACKUP:
    return "file_backup";
  case FileTypes::File_TEMP:
    return "file_temp";
  case FileTypes::File_DATA:
    return "file_data";

  // Other
  case FileTypes::File_UNKNOWN:
    return "file_unknown";
  }

  return "file_unknown"; // fallback
}

std::shared_ptr<Cherry::AppWindow> &TextEditorAppWindow::GetAppWindow() {
  return m_AppWindow;
}

std::shared_ptr<TextEditorAppWindow>
TextEditorAppWindow::Create(const std::string &path, const std::string &name) {
  auto instance =
      std::shared_ptr<TextEditorAppWindow>(new TextEditorAppWindow(path, name));
  instance->SetupRenderCallback();
  return instance;
}

void TextEditorAppWindow::SetupRenderCallback() {
  auto self = shared_from_this();
  m_AppWindow->SetRenderCallback([self]() {
    if (self) {
      self->Render();
    }
  });
}

void TextEditorAppWindow::RenderMenubar() {
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

void TextEditorAppWindow::RefreshFile() {
  m_FileEdited = false;
  m_FileUpdated = true;
  try {
    if (m_FilePath.empty()) {
      std::cerr << "RefreshFile: no file path set\n";
      return;
    }

    namespace fs = std::filesystem;
    std::error_code ec;

    if (!fs::exists(m_FilePath, ec)) {
      std::cerr << "RefreshFile: file does not exist: " << m_FilePath << "\n";
      return;
    }

    std::ifstream ifs(m_FilePath, std::ios::binary);
    if (!ifs.is_open()) {
      std::cerr << "RefreshFile: failed to open file: " << m_FilePath << "\n";
      return;
    }

    std::string content;
    ifs.seekg(0, std::ios::end);
    auto size = ifs.tellg();
    if (size > 0) {
      content.resize(static_cast<size_t>(size));
      ifs.seekg(0, std::ios::beg);
      ifs.read(&content[0], size);
    } else {
      content.clear();
    }
    ifs.close();

    if (content != m_FileEditBuffer) {
      m_FileEditBuffer = std::move(content);
    }

    m_LastWriteTime = fs::last_write_time(m_FilePath, ec);

  } catch (const std::exception &e) {
    std::cerr << "RefreshFile: exception: " << e.what() << "\n";
  }
}

void TextEditorAppWindow::SaveFile() {
  m_FileEdited = false;
  m_FileUpdated = true;
  try {
    if (m_FilePath.empty()) {
      std::cerr << "SaveFile: no file path set\n";
      return;
    }

    namespace fs = std::filesystem;
    std::error_code ec;

    fs::path target = m_FilePath;
    fs::path parent = target.parent_path();
    if (!parent.empty() && !fs::exists(parent, ec)) {
      if (!fs::create_directories(parent, ec)) {
        std::cerr << "SaveFile: unable to create parent directories: " << parent
                  << " (" << ec.message() << ")\n";
        return;
      }
    }

    auto timestamp =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    fs::path tempPath = parent / (target.filename().string() + ".tmp." +
                                  std::to_string(timestamp));

    {
      std::ofstream ofs(tempPath, std::ios::binary | std::ios::trunc);
      if (!ofs.is_open()) {
        std::cerr << "SaveFile: failed to open temp file for writing: "
                  << tempPath << "\n";
        std::error_code rmec;
        fs::remove(tempPath, rmec);
        return;
      }

      ofs.write(m_FileEditBuffer.data(),
                static_cast<std::streamsize>(m_FileEditBuffer.size()));
      if (!ofs) {
        std::cerr << "SaveFile: write failed to temp file: " << tempPath
                  << "\n";
        ofs.close();
        fs::remove(tempPath, ec);
        return;
      }
      ofs.flush();
      ofs.close();
    }

    if (fs::exists(target, ec)) {
      std::error_code removeEc;
      fs::remove(target, removeEc);
      std::error_code renameEc;
      fs::rename(tempPath, target, renameEc);
      if (renameEc) {
        std::error_code copyEc;
        fs::copy_file(tempPath, target, fs::copy_options::overwrite_existing,
                      copyEc);
        if (copyEc) {
          std::cerr << "SaveFile: failed to replace target file: " << target
                    << " (rename: " << renameEc.message()
                    << ", copy: " << copyEc.message() << ")\n";
          fs::remove(tempPath, ec);
          return;
        }
        fs::remove(tempPath, ec);
      }
    } else {
      std::error_code renameEc;
      fs::rename(tempPath, target, renameEc);
      if (renameEc) {
        std::error_code copyEc;
        fs::copy_file(tempPath, target, fs::copy_options::none, copyEc);
        if (copyEc) {
          std::cerr << "SaveFile: failed to move temp file to target: "
                    << target << " (rename: " << renameEc.message()
                    << ", copy: " << copyEc.message() << ")\n";
          fs::remove(tempPath, ec);
          return;
        }
        fs::remove(tempPath, ec);
      }
    }
  } catch (const std::exception &e) {
    std::cerr << "SaveFile: exception: " << e.what() << "\n";
  }
}

void TextEditorAppWindow::Undo() { m_UndoPending = true; }
void TextEditorAppWindow::Redo() { m_RedoPending = true; }

FileTypes TextEditorAppWindow::detect_file(const std::string &path) {
  static const std::unordered_map<std::string, FileTypes> extension_map = {
      // Web and Markup
      {"xml", FileTypes::File_XML},
      {"json", FileTypes::File_JSON},
      {"yaml", FileTypes::File_YAML},
      {"yml", FileTypes::File_YAML},

      // Config
      {"cfg", FileTypes::File_CFG},
      {"ini", FileTypes::File_INI},
      {"env", FileTypes::File_INI},

      // Documents
      {"txt", FileTypes::File_TXT},
      {"md", FileTypes::File_MD},
      {"rst", FileTypes::File_MD},

      // Miscellaneous
      {"log", FileTypes::File_LOG},
      {"bak", FileTypes::File_BACKUP},
      {"tmp", FileTypes::File_TEMP},
      {"dat", FileTypes::File_DATA},
  };

  std::string extension = get_extension(path);
  auto it = extension_map.find(extension);
  if (it != extension_map.end()) {
    return it->second;
  } else {
    return FileTypes::File_UNKNOWN;
  }
}

void TextEditorAppWindow::Render() {
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

void TextEditorAppWindow::RenderRightMenubar() {
  CherryGUI::PushStyleColor(ImGuiCol_Border, ImVec4(0.4f, 0.4f, 0.4f, 0.7f));

  CherryNextComponent.SetProperty("padding_y", "6.0f");
  CherryNextComponent.SetProperty("padding_x", "10.0f");
  CherryNextComponent.SetProperty("disable_callback", "true");
  if (CherryKit::ButtonImageTextDropdown(
          "Settings", GetPath("resources/imgs/icons/misc/icon_settings.png"))
          .GetDataAs<bool>("isClicked")) {
    ImVec2 mousePos = CherryGUI::GetMousePos();
    ImVec2 displaySize = CherryGUI::GetIO().DisplaySize;
    ImVec2 popupSize(150, 100);

    if (mousePos.x + popupSize.x > displaySize.x) {
      mousePos.x -= popupSize.x;
    }
    if (mousePos.y + popupSize.y > displaySize.y) {
      mousePos.y -= popupSize.y;
    }

    CherryGUI::SetNextWindowSize(ImVec2(150, 100), ImGuiCond_Appearing);
    CherryGUI::SetNextWindowPos(mousePos, ImGuiCond_Appearing);
    CherryGUI::OpenPopup("SettingsMenuPopup");
  }
  if (CherryGUI::BeginPopup("SettingsMenuPopup")) {
    CherryKit::CheckboxText("Auto refresh", &m_AutoRefresh);
    CherryGUI::EndPopup();
  }

  CherryGUI::SetCursorPosY(CherryGUI::GetCursorPosY() - 3.0f);

  CherryGUI::PopStyleColor();
  CherryGUI::SetCursorPosY(CherryGUI::GetCursorPosY() - 1.5f);
}

}; // namespace ModuleUI
