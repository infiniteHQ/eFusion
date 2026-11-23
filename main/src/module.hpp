
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

EFUSION_API std::string get_path(const std::string &path);

EFUSION_API void StartTextEditorInstance(const std::string &path);
EFUSION_API bool IsValidFile(const std::string &path);
} // namespace eFusion

#endif // EFUSION_HPP