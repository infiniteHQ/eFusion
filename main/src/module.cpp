#include "module.hpp"

void EmbeddedFusion::CreateContext() {
  EmbeddedFusion::Context *ctx = VX_NEW(EmbeddedFusion::Context);
  CEmbeddedFusion = ctx;
}

void EmbeddedFusion::DestroyContext() { VX_FREE(CEmbeddedFusion); }

std::string EmbeddedFusion::GetPath(const std::string &path) {
  return CEmbeddedFusion->m_interface->GetBinaryPath() + "/" + path;
}

bool EmbeddedFusion::IsMainSketch(const std::string &path) {
  fs::path base(path);

  if (!fs::is_directory(base / "src_nodal"))
    return false;
  if (!fs::is_directory(base / "src"))
    return false;
  if (!fs::is_directory(base / "src/setup"))
    return false;
  if (!fs::is_directory(base / "src/main"))
    return false;
  if (!fs::is_directory(base / "configs"))
    return false;

  if (!fs::is_regular_file(base / "src/setup/pin_setup.json"))
    return false;
  if (!fs::is_regular_file(base / "src/main/main_sketch.json"))
    return false;
  if (!fs::is_regular_file(base / "main_sketch.json"))
    return false;

  return true;
}

void EmbeddedFusion::CreateMainSketch(const std::string &path) {
  fs::path base(path);
  fs::path sketchRoot = base / "New Main Sketch";

  fs::create_directories(sketchRoot / "src_nodal");
  fs::create_directories(sketchRoot / "src/setup");
  fs::create_directories(sketchRoot / "src/main");
  fs::create_directories(sketchRoot / "configs");

  auto createFile = [](const fs::path &p) {
    std::ofstream ofs(p.string());
    ofs << "{}";
  };

  createFile(sketchRoot / "src/setup/pin_setup.json");
  createFile(sketchRoot / "src/main/main_sketch.json");
  createFile(sketchRoot / "main_sketch.json");
}

void EmbeddedFusion::OpenMainSketch(const std::string &path) {
  std::string filename = fs::path(path).filename().string();

  const size_t maxLen = 24;
  if (filename.size() > maxLen) {
    filename = filename.substr(0, maxLen - 3) + "...";
  }

  std::string window_name =
      filename + "####" +
      std::to_string(CEmbeddedFusion->m_main_sketch_instances.size());
  std::shared_ptr<ModuleUI::MainSketchAppWindow> big_win =
      ModuleUI::MainSketchAppWindow::Create(path, window_name);
  Cherry::AddAppWindow(big_win->GetAppWindow());
}