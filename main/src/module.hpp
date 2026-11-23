
#include <vxcore/include/vortex.h>
#include <vxcore/include/vortex_internals.h>
#include <vxgui/editor/main/editor.hpp>

#include "../ui/instances/text_editor/text_editor.hpp"

#ifndef EFUSION_HPP
#define EFUSION_HPP

namespace eFusion {
struct Context {
  std::shared_ptr<ModuleInterface> m_interface;
  std::vector<std::shared_ptr<ModuleUI::TextEditorAppWindow>>
      m_text_editor_instances;
};
} // namespace eFusion

#ifndef EFUSION_API
#define EFUSION_API
#endif

#ifndef CeFusion
extern EFUSION_API std::weak_ptr<eFusion::Context> CeFusion;
#endif

namespace eFusion {
// Context
EFUSION_API std::shared_ptr<eFusion::Context> create_context();
EFUSION_API void destroy_context(std::shared_ptr<eFusion::Context> ctx);
EFUSION_API void set_current_context(std::shared_ptr<eFusion::Context> ctx);
EFUSION_API std::shared_ptr<eFusion::Context> get_current_context();

// Utils
EFUSION_API std::string get_path(const std::string &path);
EFUSION_API bool is_hardware_profile(const std::string &path);
EFUSION_API bool is_firmware_node_function(const std::string &path);
EFUSION_API bool is_firmware_code_function(const std::string &path);

// Editors
EFUSION_API void open_hardware_profile_editor(const std::string &path);
EFUSION_API void open_firmware_code_editor(const std::string &path);
EFUSION_API void open_firmware_node_editor(const std::string &path);

// Toolbar actions
EFUSION_API void create_hardware_profile();
EFUSION_API void create_firmware_code_function();
EFUSION_API void create_firmware_node_function();

EFUSION_API void StartTextEditorInstance(const std::string &path);
} // namespace eFusion

#endif // EFUSION_HPP