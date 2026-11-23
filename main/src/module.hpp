
#include <vxcore/include/vortex.h>
#include <vxcore/include/vortex_internals.h>
#include <vxgui/editor/main/editor.hpp>

#include "../ui/instances/text_editor/text_editor.hpp"

#ifndef CODEEDIT_HPP
#define CODEEDIT_HPP

namespace CodeEdit {
struct Context {
  std::shared_ptr<ModuleInterface> m_interface;
  std::vector<std::shared_ptr<ModuleUI::TextEditorAppWindow>>
      m_text_editor_instances;
};
} // namespace CodeEdit

#ifndef CODEEDIT_API
#define CODEEDIT_API
#endif

#ifndef CCodeEdit
extern CODEEDIT_API std::weak_ptr<CodeEdit::Context> CCodeEdit;
#endif

namespace CodeEdit {
// Context
CODEEDIT_API std::shared_ptr<CodeEdit::Context> create_context();
CODEEDIT_API void destroy_context(std::shared_ptr<CodeEdit::Context> ctx);
CODEEDIT_API void set_current_context(std::shared_ptr<CodeEdit::Context> ctx);
CODEEDIT_API std::shared_ptr<CodeEdit::Context> get_current_context();

CODEEDIT_API std::string get_path(const std::string &path);

CODEEDIT_API void StartTextEditorInstance(const std::string &path);
CODEEDIT_API bool IsValidFile(const std::string &path);
} // namespace CodeEdit

#endif // CODEEDIT_HPP