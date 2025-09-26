#pragma once
#include "../../../../lib/vortex/main/include/vortex.h"
#include "../../../../lib/vortex/main/include/vortex_internals.h"
#include "text_editor_core.hpp"

#ifndef TEXT_EDITOR_HPP
#define TEXT_EDITOR_HPP

namespace ModuleUI {

enum class FileTypes {
  // Web and Markup
  File_XML,

  // Config
  File_CFG,
  File_JSON,
  File_YAML,
  File_INI,

  // Documents
  File_TXT,
  File_MD,

  // Archives
  File_ARCHIVE,

  // Miscellaneous
  File_LOG,
  File_BACKUP,
  File_TEMP,
  File_DATA,

  // Other
  File_UNKNOWN,
};

class TextEditorAppWindow
    : public std::enable_shared_from_this<TextEditorAppWindow> {
public:
  TextEditorAppWindow(const std::string &path, const std::string &name);

  void menubar();
  FileTypes detect_file(const std::string &path);
  std::string GetFileTypeStr(FileTypes type);
  std::shared_ptr<Cherry::AppWindow> &GetAppWindow();
  static std::shared_ptr<TextEditorAppWindow> Create(const std::string &path,
                                                     const std::string &name);
  void SetupRenderCallback();
  void Render();
  void RenderMenubar();
  void RenderRightMenubar();

  std::string get_extension(const std::string &path) {
    size_t dot_pos = path.find_last_of('.');
    if (dot_pos == std::string::npos)
      return "";
    return path.substr(dot_pos + 1);
  }

  void RefreshFile();
  void SaveFile();
  void Undo();
  void Redo();

private:
  VxContext *ctx;
  bool opened;
  std::string m_FileEditBuffer;
  std::string m_FilePath;
  FileTypes m_Type;

  // Editor actions
  bool m_UndoPending = false;
  bool m_RedoPending = false;
  bool m_SavePending = false;
  bool m_FileEdited = true;
  bool m_FileUpdated = true;

  // Editor flags
  bool m_SaveReady = false;
  bool m_RefreshReady = false;

  bool m_AutoRefresh = false;
  std::filesystem::file_time_type m_LastWriteTime{};

  // Cherry
  std::shared_ptr<Cherry::AppWindow> m_AppWindow;
  ComponentsPool m_ComponentPool;
};
}; // namespace ModuleUI

#endif // LOGUTILITY_H
