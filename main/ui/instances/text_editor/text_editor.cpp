#include "text_editor.hpp"
#include "../../../src/module.hpp"

#include <filesystem>
#include <iostream>
#include <string>

namespace ModuleUI {

void TextEditorAppWindow::PlusMinuxWidget(bool plus) {
  ImVec2 btn_pos = CherryGUI::GetCursorScreenPos();
  float btn_size = 22.0f;
  float rounding = 4.0f;
  float bar_thickness = 2.0f;
  float bar_half = 5.0f;

  ImVec2 btn_min = btn_pos;
  ImVec2 btn_max = ImVec2(btn_pos.x + btn_size, btn_pos.y + btn_size);
  ImVec2 center =
      ImVec2(btn_pos.x + btn_size * 0.5f, btn_pos.y + btn_size * 0.5f);

  CherryGUI::InvisibleButton("##zoom_in", ImVec2(btn_size, btn_size));
  bool hovered = CherryGUI::IsItemHovered();
  bool clicked = CherryGUI::IsItemClicked();

  if (plus) {
    if (clicked)
      ZoomIn();
  } else {
    if (clicked)
      ZoomOut();
  }

  ImDrawList *dl = CherryGUI::GetWindowDrawList();

  ImU32 bg_color =
      hovered ? IM_COL32(80, 80, 80, 255) : IM_COL32(55, 55, 55, 255);

  CherryGUI::AddRectFilled(dl, btn_min, btn_max, bg_color, rounding);

  CherryGUI::AddRect(dl, btn_min, btn_max, IM_COL32(120, 120, 120, 180),
                     rounding, 0, 1.0f);

  ImU32 fg_color = IM_COL32(220, 220, 220, 255);

  CherryGUI::AddRectFilled(
      dl, ImVec2(center.x - bar_half, center.y - bar_thickness * 0.5f),
      ImVec2(center.x + bar_half, center.y + bar_thickness * 0.5f), fg_color);

  if (plus) {
    CherryGUI::AddRectFilled(
        dl, ImVec2(center.x - bar_thickness * 0.5f, center.y - bar_half),
        ImVec2(center.x + bar_thickness * 0.5f, center.y + bar_half), fg_color);
  }
}

TextEditorAppWindow::TextEditorAppWindow(const std::string &path,
                                         const std::string &name) {
  namespace fs = std::filesystem;

  m_Type = detect_file(path);
  m_AppWindow = std::make_shared<Cherry::AppWindow>(name, name);

  DefineWindowIcon();
  m_AppWindow->SetLeftMenubarCallback([this]() { RenderMenubar(); });
  m_AppWindow->SetRightMenubarCallback([this]() { RenderRightMenubar(); });
  m_AppWindow->SetLeftBottombarCallback([this]() { RenderBottombar(); });
  m_AppWindow->SetSaveMode(true);

  m_AppWindow->m_CloseCallback = [=]() {
    Cherry::DeleteAppWindow(m_AppWindow);
    m_AppWindow->SetVisibility(false);
  };

  std::shared_ptr<Cherry::AppWindow> win = m_AppWindow;
  m_FilePath = path;

  RefreshFile();

  this->ctx = vxe::get_current_context();
}

void TextEditorAppWindow::SetLanguage(const std::string &name) {}

// TODO: Let the user customize icons with custom vortex events
// TODO: Let the user desactivate this feature and always show icons/edit.png
void TextEditorAppWindow::DefineWindowIcon() {
  switch (m_Type) {
  case FileTypes::File_CPP: {
    m_AppWindow->SetIcon(
        CodeEdit::get_path("resources/icons/window_icons/cpp.png"));
    break;
  }
  case FileTypes::File_C: {
    m_AppWindow->SetIcon(
        CodeEdit::get_path("resources/icons/window_icons/c.png"));
    break;
  }
  case FileTypes::File_HPP: {
    m_AppWindow->SetIcon(
        CodeEdit::get_path("resources/icons/window_icons/hpp.png"));
    break;
  }
  case FileTypes::File_H: {
    m_AppWindow->SetIcon(
        CodeEdit::get_path("resources/icons/window_icons/h.png"));
    break;
  }
  case FileTypes::File_LUA: {
    m_AppWindow->SetIcon(
        CodeEdit::get_path("resources/icons/window_icons/lua.png"));
    break;
  }
  default: {
    m_AppWindow->SetIcon(CodeEdit::get_path("resources/icons/edit.png"));
    break;
  }
  }
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

  if (!m_FileEdited) {
    CherryGUI::BeginDisabled();
  }

  CherryNextComponent.SetProperty("padding_y", "5.5f");
  CherryNextComponent.SetProperty("padding_x", "6.0f");
  CherryNextComponent.SetProperty("size_x", "18");
  CherryNextComponent.SetProperty("size_y", "18");
  if (CherryKit::ButtonImage(
          CodeEdit::get_path("/resources/icons/icon_save.png"))
          .GetDataAs<bool>("isClicked")) {
    m_SavePending = true;
  }

  if (!m_FileEdited) {
    CherryGUI::EndDisabled();
  }

  CherryNextComponent.SetProperty("padding_y", "5.5f");
  CherryNextComponent.SetProperty("padding_x", "6.0f");
  CherryNextComponent.SetProperty("size_x", "18");
  CherryNextComponent.SetProperty("size_y", "18");
  if (CherryKit::ButtonImage(
          CodeEdit::get_path("/resources/icons/icon_refresh.png"))
          .GetDataAs<bool>("isClicked")) {
    m_RefreshReady = true;
  }

  CherryKit::Separator();

  CherryNextComponent.SetProperty("padding_y", "6.0f");
  CherryNextComponent.SetProperty("padding_x", "10.0f");
  if (CherryKit::ButtonImageText(
          "Find",
          CodeEdit::get_path("/resources/icons/icon_magnifying_glass.png"))
          .GetDataAs<bool>("isClicked")) {
    m_FindPending = true;
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
      {"cpp", FileTypes::File_CPP},
      {"hpp", FileTypes::File_HPP},
      {"lua", FileTypes::File_LUA},
      {"python", FileTypes::File_PYTHON},
      {"c", FileTypes::File_C},
      {"h", FileTypes::File_H},
      {"cs", FileTypes::File_CS},

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

void TextEditorAppWindow::RenderCustomMenu() { CherryGUI::Text("Helo"); }

void TextEditorAppWindow::toggleTrieAutoComplete() {

}
void TextEditorAppWindow::toggleLspBridge() {

}

void TextEditorAppWindow::Render() {
}

void TextEditorAppWindow::RenderBottombar() {
}

}; // namespace ModuleUI
