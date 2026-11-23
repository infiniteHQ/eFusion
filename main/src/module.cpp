#include "module.hpp"

// runtime pointer
#ifndef CeFusion
std::weak_ptr<eFusion::Context> CeFusion;
#endif

std::shared_ptr<eFusion::Context> eFusion::create_context() {
  auto ctx = std::make_shared<eFusion::Context>();

  set_current_context(ctx);

  return ctx;
}

void eFusion::destroy_context(std::shared_ptr<eFusion::Context> ctx) {
  set_current_context(nullptr);
}

void eFusion::set_current_context(std::shared_ptr<eFusion::Context> ctx) {
  CeFusion = ctx;
}

std::shared_ptr<eFusion::Context> eFusion::get_current_context() {
  return CeFusion.lock();
}

std::string eFusion::get_path(const std::string &path) {
  return get_current_context()->m_interface->cook_path(path);
}

bool eFusion::IsValidFile(const std::string &path) {
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

void eFusion::StartTextEditorInstance(const std::string &path) {
  std::string filename = fs::path(path).filename().string();

  const size_t maxLen = 24;
  if (filename.size() > maxLen) {
    filename = filename.substr(0, maxLen - 3) + "...";
  }

  std::string window_name =
      filename + "####" +
      std::to_string(
          eFusion::get_current_context()->m_text_editor_instances.size());

  auto inst = ModuleUI::TextEditorAppWindow::Create(path, window_name);
  Cherry::AddAppWindow(inst->GetAppWindow());
  eFusion::get_current_context()->m_text_editor_instances.push_back(inst);
}
