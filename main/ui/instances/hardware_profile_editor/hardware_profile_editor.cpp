#include "hardware_profile_editor.hpp"
#include "../../../src/module.hpp"

namespace ModuleUI {

namespace fs = std::filesystem;
using Json = HardwareProfile::Json;

HardwareProfile::HardwareProfile(const std::string &path) {
  fs::path p(path);
  m_File = (p.extension() == ".json") ? p : p / "hardware_profile.json";
  vxe::log_info("HardwareProfile: {}", m_File.string());
  Load();
}

Json::json_pointer HardwareProfile::Ptr(const std::string &key) {
  std::string out;
  size_t start = 0;
  while (start <= key.size() && !key.empty()) {
    size_t end = key.find('.', start);
    if (end == std::string::npos)
      end = key.size();
    std::string token = key.substr(start, end - start);
    std::string escaped;
    for (char c : token) {
      if (c == '~')
        escaped += "~0";
      else if (c == '/')
        escaped += "~1";
      else
        escaped += c;
    }
    out += "/" + escaped;
    start = end + 1;
  }
  return Json::json_pointer(out);
}

bool HardwareProfile::Load() {
  m_UndoStack.clear();
  m_RedoStack.clear();
  m_Data = Json::object();

  std::error_code ec;
  bool regenerate = false;

  if (!fs::exists(m_File, ec)) {
    spdlog::info("Profile not found, creating: {}", m_File.string());
    regenerate = true;
  } else {
    std::ifstream in(m_File, std::ios::binary);
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    if (content.find_first_not_of(" \t\r\n") == std::string::npos) {
      spdlog::warn("Profile is empty, regenerating: {}", m_File.string());
      regenerate = true;
    } else {
      Json parsed = Json::parse(content, nullptr, false);
      if (parsed.is_discarded() || !parsed.is_object()) {
        fs::path bak = m_File;
        bak += ".bak";
        fs::copy_file(m_File, bak, fs::copy_options::overwrite_existing, ec);
        spdlog::warn("Profile invalid, backup in {}, regenerating", bak.string());
        regenerate = true;
      } else {
        m_Data = std::move(parsed);
        spdlog::info("Profile loaded: {}", m_File.string());
      }
    }
  }

  m_Revision = m_SavedRevision = ++m_Counter;
  return regenerate ? Save() : true;
}
bool HardwareProfile::Save() {
  fs::path tmp = m_File;
  tmp += ".tmp";
  std::error_code ec;

  if (m_File.has_parent_path())
    fs::create_directories(m_File.parent_path(), ec);

  {
    std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
    if (!out) {
      spdlog::error("Cannot write: {}", tmp.string());
      return false;
    }
    out << m_Data.dump(2);
    out.flush();
    if (!out) {
      spdlog::error("Write failed: {}", tmp.string());
      return false;
    }
  }

  fs::rename(tmp, m_File, ec);
  if (ec) {
    fs::remove(m_File, ec);
    ec.clear();
    fs::rename(tmp, m_File, ec);
    if (ec) {
      spdlog::error("Rename failed: {}", ec.message());
      return false;
    }
  }

  m_SavedRevision = m_Revision;
  spdlog::info("Profile saved: {}", m_File.string());
  return true;
}

bool HardwareProfile::Has(const std::string &key) const {
  try {
    return m_Data.contains(Ptr(key));
  } catch (const std::exception &) {
    return false;
  }
}

Json HardwareProfile::GetJson(const std::string &key) const {
  try {
    return m_Data.at(Ptr(key));
  } catch (const std::exception &) {
    return Json();
  }
}

size_t HardwareProfile::Size(const std::string &key) const {
  Json v = GetJson(key);
  return (v.is_array() || v.is_object()) ? v.size() : 0;
}

std::vector<std::string> HardwareProfile::Keys(const std::string &key) const {
  std::vector<std::string> out;
  Json v = GetJson(key);
  if (v.is_object())
    for (auto it = v.begin(); it != v.end(); ++it)
      out.push_back(it.key());
  return out;
}

bool HardwareProfile::Commit(Json next) {
  if (next == m_Data)
    return false;

  m_UndoStack.push_back({std::move(m_Data), m_Revision});
  if (m_UndoStack.size() > kMaxHistory)
    m_UndoStack.pop_front();
  m_RedoStack.clear();

  m_Data = std::move(next);
  m_Revision = ++m_Counter;
  return true;
}

bool HardwareProfile::Set(const std::string &key, Json value) {
  try {
    Json next = m_Data;
    next[Ptr(key)] = std::move(value);
    return Commit(std::move(next));
  } catch (const std::exception &e) {
    spdlog::error("Set '{}' failed: {}", key, e.what());
    return false;
  }
}

bool HardwareProfile::Remove(const std::string &key) {
  try {
    auto ptr = Ptr(key);
    if (ptr.empty() || !m_Data.contains(ptr))
      return false;

    Json next = m_Data;
    Json &parent = next.at(ptr.parent_pointer());
    std::string last = ptr.back();
    if (parent.is_object())
      parent.erase(last);
    else if (parent.is_array())
      parent.erase(parent.begin() + std::stoul(last));
    else
      return false;
    return Commit(std::move(next));
  } catch (const std::exception &e) {
    spdlog::error("Remove '{}' failed: {}", key, e.what());
    return false;
  }
}

bool HardwareProfile::Push(const std::string &key, Json value) {
  try {
    Json next = m_Data;
    Json &node = next[Ptr(key)];
    if (node.is_null())
      node = Json::array();
    if (!node.is_array())
      return false;
    node.push_back(std::move(value));
    return Commit(std::move(next));
  } catch (const std::exception &e) {
    spdlog::error("Push '{}' failed: {}", key, e.what());
    return false;
  }
}

bool HardwareProfile::Insert(const std::string &key, size_t index, Json value) {
  try {
    Json next = m_Data;
    Json &node = next[Ptr(key)];
    if (node.is_null())
      node = Json::array();
    if (!node.is_array())
      return false;
    if (index > node.size())
      index = node.size();
    node.insert(node.begin() + index, std::move(value));
    return Commit(std::move(next));
  } catch (const std::exception &e) {
    spdlog::error("Insert '{}' failed: {}", key, e.what());
    return false;
  }
}

bool HardwareProfile::Undo() {
  if (m_UndoStack.empty())
    return false;
  m_RedoStack.push_back({std::move(m_Data), m_Revision});
  m_Data = std::move(m_UndoStack.back().data);
  m_Revision = m_UndoStack.back().revision;
  m_UndoStack.pop_back();
  return true;
}

bool HardwareProfile::Redo() {
  if (m_RedoStack.empty())
    return false;
  m_UndoStack.push_back({std::move(m_Data), m_Revision});
  m_Data = std::move(m_RedoStack.back().data);
  m_Revision = m_RedoStack.back().revision;
  m_RedoStack.pop_back();
  return true;
}

HardwareProfileEditorAppWindow::HardwareProfileEditorAppWindow(const std::string &path,
                                                               const std::string &name) {
  m_AppWindow = std::make_shared<Cherry::AppWindow>(name, name);
  m_Profile = std::make_unique<HardwareProfile>(path);

  m_AppWindow->SetLeftMenubarCallback([this]() { menubar(); });
  m_AppWindow->SetSaveMode(true);

  m_AppWindow->m_CloseCallback = [=]() {
    Cherry::DeleteAppWindow(m_AppWindow);
    m_AppWindow->SetVisibility(false);
  };

  this->ctx = vxe::get_current_context();
  Sync();
}

std::shared_ptr<Cherry::AppWindow> &HardwareProfileEditorAppWindow::GetAppWindow() {
  return m_AppWindow;
}

std::shared_ptr<HardwareProfileEditorAppWindow>
HardwareProfileEditorAppWindow::Create(const std::string &path, const std::string &name) {
  auto instance = std::shared_ptr<HardwareProfileEditorAppWindow>(
      new HardwareProfileEditorAppWindow(path, name));
  instance->SetupRenderCallback();
  return instance;
}

void HardwareProfileEditorAppWindow::SetupRenderCallback() {
  auto self = shared_from_this();
  m_AppWindow->SetRenderCallback([self]() {
    if (self) {
      self->render();
    }
  });
}

void HardwareProfileEditorAppWindow::Sync() { m_AppWindow->SetSaved(!m_Profile->IsDirty()); }

bool HardwareProfileEditorAppWindow::SetValue(const std::string &key, Json value) {
  bool changed = m_Profile->Set(key, std::move(value));
  Sync();
  return changed;
}

bool HardwareProfileEditorAppWindow::RemoveValue(const std::string &key) {
  bool changed = m_Profile->Remove(key);
  Sync();
  return changed;
}

bool HardwareProfileEditorAppWindow::PushValue(const std::string &key, Json value) {
  bool changed = m_Profile->Push(key, std::move(value));
  Sync();
  return changed;
}

bool HardwareProfileEditorAppWindow::InsertValue(const std::string &key, size_t index,
                                                 Json value) {
  bool changed = m_Profile->Insert(key, index, std::move(value));
  Sync();
  return changed;
}

bool HardwareProfileEditorAppWindow::Undo() {
  bool done = m_Profile->Undo();
  Sync();
  return done;
}

bool HardwareProfileEditorAppWindow::Redo() {
  bool done = m_Profile->Redo();
  Sync();
  return done;
}

bool HardwareProfileEditorAppWindow::Save() {
  bool done = m_Profile->Save();
  Sync();
  return done;
}

void HardwareProfileEditorAppWindow::menubar() {
  if (CherryGUI::MenuItem("Save", "Ctrl+S", false, m_Profile->IsDirty()))
    Save();
  if (CherryGUI::MenuItem("Undo", "Ctrl+Z", false, m_Profile->CanUndo()))
    Undo();
  if (CherryGUI::MenuItem("Redo", "Ctrl+Y", false, m_Profile->CanRedo()))
    Redo();
}

void HardwareProfileEditorAppWindow::shortcuts() {
  if (!CherryGUI::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
    return;

  ImGuiIO &io = CherryGUI::GetIO();
  if (!io.KeyCtrl)
    return;

  if (CherryGUI::IsKeyPressed(ImGuiKey_S, false))
    Save();
  else if (CherryGUI::IsKeyPressed(ImGuiKey_Z) && !io.KeyShift)
    Undo();
  else if (CherryGUI::IsKeyPressed(ImGuiKey_Y) ||
           (CherryGUI::IsKeyPressed(ImGuiKey_Z) && io.KeyShift))
    Redo();
}

void HardwareProfileEditorAppWindow::render() {
  shortcuts();


}

}; // namespace ModuleUI