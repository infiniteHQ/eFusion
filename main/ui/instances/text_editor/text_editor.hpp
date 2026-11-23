#pragma once

#include <vxcore/include/vortex.h>
#include <vxcore/include/vortex_internals.h>

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

  File_SQL,
  File_LUA,
  File_PYTHON,
  File_CS,
  File_CPP,
  File_C,
  File_HPP,
  File_H,

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
  void RenderBottombar();

  void RenderCustomMenu();

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

  void DefineWindowIcon();

  void SetLanguage(const std::string &name);
  void AutoSetLanguage();

  void PlusMinuxWidget(bool plus);


  float m_TextSize = 0.5f;
  float m_TextSizeMin = 0.3f;
  float m_TextSizeMax = 2.0f;

  int m_CurrentLine = 0;
  int m_CurrentColumn = 0;
  int m_TotalLines = 0;
  std::string m_CurrentLanguageDef = "";
  bool m_CanOverrite = false;

  void ZoomIn() {
    m_TextSize += 0.05f;
    if (m_TextSize > 1.15f)
      m_TextSize = 1.15f;
  }

  void ZoomOut() {
    m_TextSize -= 0.05f;
    if (m_TextSize < 0.35f)
      m_TextSize = 0.35f;
  }

  void ResetZoom() {
    m_TextSize = 0.50f; // 100%
  }

  void toggleTrieAutoComplete();

  void toggleLspBridge();

private:
  std::shared_ptr<VxContext> ctx;
  bool opened;
  std::string m_FileEditBuffer;
  std::string m_FilePath;
  FileTypes m_Type;

  // Editor actions
  bool m_FindPending = false;
  bool m_UndoPending = false;
  bool m_RedoPending = false;
  bool m_SavePending = false;
  bool m_CopyPending = false;
  bool m_PastePending = false;
  bool m_FileEdited = true;
  bool m_FileUpdated = true;

  bool demoTrieAutoComplete = false;
  bool demoLspBridge = false;
  bool showWordAtMouse = false;
  bool showLineMarkers = false;
  bool showLineDecorator = false;
  bool showCustomCaret = false;
  bool showContextMenus = false;
  bool enableUnicodeLineBreakAlgorithm = false;
  bool showDebugInformation = false;
  bool showDebugWindow = false;
  // Editor flags
  bool m_SaveReady = false;
  bool m_RefreshReady = false;

  bool m_AutoRefresh = false;
  bool show_spaces_ = false;
  bool show_scrollbar_minimap_ = false;
  bool show_minimap_ = false;
  bool word_wrap_ = false;
  bool line_folding_ = false;
  bool show_doc_pos_at_mouse_ = false;
  std::filesystem::file_time_type m_LastWriteTime{};

  // Cherry
  std::shared_ptr<Cherry::AppWindow> m_AppWindow;
  ComponentsPool m_ComponentPool;
};
}; // namespace ModuleUI

#endif // LOGUTILITY_H
