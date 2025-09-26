#include "module.hpp"

void EmbeddedFusion::CreateContext() {
  EmbeddedFusion::Context *ctx = VX_NEW(EmbeddedFusion::Context);
  CEmbeddedFusion = ctx;
}

void EmbeddedFusion::DestroyContext() { VX_FREE(CEmbeddedFusion); }

void EmbeddedFusion::HelloWorld() {
  std::cout << "Hello Vortex World !!" << std::endl;
}

void EmbeddedFusion::OutputHandleHello() {
  std::cout << "Handling the HEllow output event...." << std::endl;
}

bool EmbeddedFusion::IsValidFile(const std::string &path) {
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

void EmbeddedFusion::StartTextEditorInstance(const std::string &path) {
  std::string filename = fs::path(path).filename().string();

  const size_t maxLen = 24;
  if (filename.size() > maxLen) {
    filename = filename.substr(0, maxLen - 3) + "...";
  }

  std::string window_name =
      filename + "####" +
      std::to_string(CEmbeddedFusion->m_text_editor_instances.size());

  auto inst = ModuleUI::TextEditorAppWindow::Create(path, window_name);
  Cherry::AddAppWindow(inst->GetAppWindow());
  CEmbeddedFusion->m_text_editor_instances.push_back(inst);
}

void EmbeddedFusion::InputHello() {
  std::cout << "Input event hello triggered !!!" << std::endl;
}

void EmbeddedFusion::FunctionWithArg(ArgumentValues &arg) {
  // std::string name = val.GetJsonValue()["name"].get<std::string>();
  std::cout << "print the name given in aguments"
            << arg.GetJsonValue()["name"].get<std::string>() << std::endl;
}

std::string EmbeddedFusion::GetPath(const std::string &path) {
  return CEmbeddedFusion->m_interface->GetPath() + "/" + path;
}

void EmbeddedFusion::FunctionWithRet(ReturnValues &ret) {
  // Set the return value (time for this example)
  ret.SetJsonValue(nlohmann::json::parse("{\"time\":\"current\"}"));
}
void EmbeddedFusion::FunctionWithArgRet(ArgumentValues &arg,
                                        ReturnValues &ret) {
  // std::string name = val.GetJsonValue()["name"].get<std::string>();
  std::string name = arg.GetJsonValue()["name"].get<std::string>();
  ret.SetJsonValue(
      nlohmann::json::parse("{\"time\":\"current_name_" + name + "\"}"));
}
