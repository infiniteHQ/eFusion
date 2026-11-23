#include "module.hpp"

// runtime pointer
#ifndef CCodeEdit
std::weak_ptr<CodeEdit::Context> CCodeEdit;
#endif

std::shared_ptr<CodeEdit::Context> CodeEdit::create_context() {
  auto ctx = std::make_shared<CodeEdit::Context>();

  set_current_context(ctx);

  return ctx;
}

void CodeEdit::destroy_context(std::shared_ptr<CodeEdit::Context> ctx) {
  set_current_context(nullptr);
}

void CodeEdit::set_current_context(std::shared_ptr<CodeEdit::Context> ctx) {
  CCodeEdit = ctx;
}

std::shared_ptr<CodeEdit::Context> CodeEdit::get_current_context() {
  return CCodeEdit.lock();
}

std::string CodeEdit::get_path(const std::string &path) {
  return get_current_context()->m_interface->cook_path(path);
}

bool CodeEdit::IsValidFile(const std::string &path) {
  namespace fs = std::filesystem;

  if (!fs::is_directory(path)) {
    return false;
  }

  for (const auto &entry : fs::directory_iterator(path)) {
    if (entry.is_regular_file() &&
        entry.path().filename() == "SampleConfig.txt") {
      return true;
    }
  }

  return false;
}

void CodeEdit::StartTextEditorInstance(const std::string &path) {
  std::string filename = fs::path(path).filename().string();

  const size_t maxLen = 24;
  if (filename.size() > maxLen) {
    filename = filename.substr(0, maxLen - 3) + "...";
  }

  std::string window_name =
      filename + "####" +
      std::to_string(
          CodeEdit::get_current_context()->m_text_editor_instances.size());

  auto inst = ModuleUI::TextEditorAppWindow::Create(path, window_name);
  Cherry::AddAppWindow(inst->GetAppWindow());
  CodeEdit::get_current_context()->m_text_editor_instances.push_back(inst);
}
