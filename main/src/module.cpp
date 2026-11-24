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


// Utils
bool eFusion::is_hardware_profile(const std::string &path){
  namespace fs = std::filesystem;

  if (!fs::is_directory(path)) {
    return false;
  }

  for (const auto &entry : fs::directory_iterator(path)) {
    if (entry.is_regular_file() &&
        entry.path().filename() == "hardware_profile.json") {
      return true;
    }
  }

  return false;
}

bool eFusion::is_firmware_node_function(const std::string &path){
  namespace fs = std::filesystem;

  if (!fs::is_directory(path)) {
    return false;
  }

  for (const auto &entry : fs::directory_iterator(path)) {
    if (entry.is_regular_file() &&
        entry.path().filename() == "firmware_node_function.json") {
      return true;
    }
  }

  return false;
}

bool eFusion::is_firmware_schematic(const std::string &path){
  namespace fs = std::filesystem;

  if (!fs::is_directory(path)) {
    return false;
  }

  for (const auto &entry : fs::directory_iterator(path)) {
    if (entry.is_regular_file() &&
        entry.path().filename() == "firmware_schematic.json") {
      return true;
    }
  }

  return false;
}

bool eFusion::is_firmware_code_function(const std::string &path){
  namespace fs = std::filesystem;

  if (!fs::is_directory(path)) {
    return false;
  }

  for (const auto &entry : fs::directory_iterator(path)) {
    if (entry.is_regular_file() &&
        entry.path().filename() == "firmware_code_function.json") {
      return true;
    }
  }

  return false;
}

// Editors
void eFusion::open_hardware_profile_editor(const std::string &path){
  std::string window_name = "Hardware profile";

  auto inst = ModuleUI::HardwareProfileEditorAppWindow::Create(path, window_name);
  Cherry::AddAppWindow(inst->GetAppWindow());
  eFusion::get_current_context()->hardware_profile_editor_instances.push_back(inst);
}

void eFusion::open_firmware_schematic_editor(const std::string &path){}
void eFusion::open_firmware_code_function_editor(const std::string &path){}
void eFusion::open_firmware_node_function_editor(const std::string &path){}

// Toolbar actions
void eFusion::create_hardware_profile(){}
void eFusion::create_firmware_schematic(){}
void eFusion::create_firmware_code_function(){}
void eFusion::create_firmware_node_function(){}

// Content browser creations 
void eFusion::create_hardware_profile_in_path(const std::string &path){}
void eFusion::create_firmware_schematic_in_path(const std::string &path){}
void eFusion::create_firmware_code_function_in_path(const std::string &path){}
void eFusion::create_firmware_node_function_in_path(const std::string &path){}