#pragma once

#include <vxcore/include/vortex.h>
#include <vxcore/include/vortex_internals.h>

#ifndef HARDWARE_PROFILE_EDITOR_HPP
#define HARDWARE_PROFILE_EDITOR_HPP

#include <array>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace ModuleUI {

class HardwareProfile {
public:
  using Json = nlohmann::json;

  explicit HardwareProfile(const std::string &path);

  bool Load();
  bool Save();

  bool Has(const std::string &key) const;
  Json GetJson(const std::string &key) const;
  size_t Size(const std::string &key) const;
  std::vector<std::string> Keys(const std::string &key) const;

  template <typename T> T Get(const std::string &key, T fallback = T{}) const {
    try {
      return m_Data.at(Ptr(key)).template get<T>();
    } catch (const std::exception &) {
      return fallback;
    }
  }

  bool Set(const std::string &key, Json value);
  bool Remove(const std::string &key);
  bool Push(const std::string &key, Json value);
  bool Insert(const std::string &key, size_t index, Json value);

  bool Undo();
  bool Redo();
  bool CanUndo() const { return !m_UndoStack.empty(); }
  bool CanRedo() const { return !m_RedoStack.empty(); }
  bool IsDirty() const { return m_Revision != m_SavedRevision; }

  const std::filesystem::path &GetFilePath() const { return m_File; }

private:
  struct Snapshot {
    Json data;
    uint64_t revision;
  };

  static Json::json_pointer Ptr(const std::string &key);
  bool Commit(Json next);

  std::filesystem::path m_File;
  Json m_Data = Json::object();
  bool m_LoadFailed = false;
  uint64_t m_Counter = 0;
  uint64_t m_Revision = 0;
  uint64_t m_SavedRevision = 0;
  std::deque<Snapshot> m_UndoStack;
  std::deque<Snapshot> m_RedoStack;
  static constexpr size_t kMaxHistory = 200;
};

class HardwareProfileEditorAppWindow
    : public std::enable_shared_from_this<HardwareProfileEditorAppWindow> {
public:
  HardwareProfileEditorAppWindow(const std::string &path, const std::string &name);

  void menubar();
  void render();
  std::shared_ptr<Cherry::AppWindow> &GetAppWindow();
  static std::shared_ptr<HardwareProfileEditorAppWindow> Create(const std::string &path,
                                                                const std::string &name);
  void SetupRenderCallback();

  template <typename T> T GetValue(const std::string &key, T fallback = T{}) const {
    return m_Profile->Get<T>(key, std::move(fallback));
  }
  bool SetValue(const std::string &key, HardwareProfile::Json value);
  bool RemoveValue(const std::string &key);
  bool PushValue(const std::string &key, HardwareProfile::Json value);
  bool InsertValue(const std::string &key, size_t index, HardwareProfile::Json value);
  bool Undo();
  bool Redo();
  bool Save();

private:
  void Sync();
  void shortcuts();

  std::shared_ptr<VxContext> ctx;
  bool opened;

  std::unique_ptr<HardwareProfile> m_Profile;
  std::array<char, 128> m_NameBuf{};
  bool m_NameEditing = false;

  // Cherry
  std::shared_ptr<Cherry::AppWindow> m_AppWindow;
  ComponentsPool m_ComponentPool;
};
}; // namespace ModuleUI

#endif // HARDWARE_PROFILE_EDITOR_HPP