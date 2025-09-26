#include "../ui/instances/text_editor/text_editor.hpp"
#include <main/include/vortex.h>
#include <main/include/vortex_internals.h>
#include <ui/editor/app/src/editor.hpp>

#ifndef SAMPLE_MODULE_HPP
#define SAMPLE_MODULE_HPP

namespace EmbeddedFusion {
struct Context {
  std::shared_ptr<ModuleInterface> m_interface;
  std::vector<std::shared_ptr<ModuleUI::TextEditorAppWindow>>
      m_text_editor_instances;
};
} // namespace EmbeddedFusion

#ifndef EMBEDDED_FUSION_API
#define EMBEDDED_FUSION_API
#endif

#ifndef CEmbeddedFusion
extern EMBEDDED_FUSION_API EmbeddedFusion::Context
    *CEmbeddedFusion; // Current implicit context pointer
#endif

// The code API of the module.
namespace EmbeddedFusion {
// Main
EMBEDDED_FUSION_API void CreateContext();
EMBEDDED_FUSION_API void DestroyContext();

// Input/Output API
EMBEDDED_FUSION_API void I_OpenMainSketch();
EMBEDDED_FUSION_API void I_OpenSketchFunction();
EMBEDDED_FUSION_API void I_OpenSketchEvent();

// Content browser
EMBEDDED_FUSION_API void IsMainSketch();
EMBEDDED_FUSION_API void CreateMainSketch();

EMBEDDED_FUSION_API void IsSketchFunction();
EMBEDDED_FUSION_API void CreateSketchFunction();

// Project settings
EMBEDDED_FUSION_API void AddModuleSettingsToProjectSettings();
EMBEDDED_FUSION_API void GetModuleSettingsToProjectSettings();

// UI
EMBEDDED_FUSION_API void SpawnMainSketchEditor();
EMBEDDED_FUSION_API void SpawnSketchFunctionEditor();

EMBEDDED_FUSION_API std::string GetPath(const std::string &path);
} // namespace EmbeddedFusion

// The code of the module.
namespace EmbeddedFusion {
// Input/Output API
EMBEDDED_FUSION_API void TranspileSketchToCPP(const std::string path);
} // namespace EmbeddedFusion

#endif // SAMPLE_MODULE_HPP