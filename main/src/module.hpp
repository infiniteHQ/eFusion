#include "../ui/instances/text_editor/text_editor.hpp"
#include <main/include/vortex.h>
#include <main/include/vortex_internals.h>
#include <ui/editor/app/src/editor.hpp>

#ifndef SAMPLE_MODULE_HPP
#define SAMPLE_MODULE_HPP

namespace SampleCppModule {
struct Context {
  std::shared_ptr<ModuleInterface> m_interface;
  std::vector<std::shared_ptr<ModuleUI::TextEditorAppWindow>>
      m_text_editor_instances;
};
} // namespace SampleCppModule

#ifndef SAMPLE_MODULE_API
#define SAMPLE_MODULE_API
#endif

#ifndef CSampleModule
extern SAMPLE_MODULE_API SampleCppModule::Context
    *CSampleModule; // Current implicit context pointer
#endif

// The code API of the module.
namespace SampleCppModule {
SAMPLE_MODULE_API void CreateContext();
SAMPLE_MODULE_API void DestroyContext();
SAMPLE_MODULE_API void HelloWorld();
SAMPLE_MODULE_API void FunctionWithArg(ArgumentValues &val);
SAMPLE_MODULE_API void FunctionWithRet(ReturnValues &ret);
SAMPLE_MODULE_API void FunctionWithArgRet(ArgumentValues &val,
                                          ReturnValues &ret);
SAMPLE_MODULE_API void OutputHandleHello();
SAMPLE_MODULE_API void InputHello();
SAMPLE_MODULE_API std::string GetPath(const std::string &path);

SAMPLE_MODULE_API void StartTextEditorInstance(const std::string &path);
SAMPLE_MODULE_API bool IsValidFile(const std::string &path);
} // namespace SampleCppModule

#endif // SAMPLE_MODULE_HPP