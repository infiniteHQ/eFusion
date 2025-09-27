#include "../ui/instances/main_sketch/main_sketch.hpp"
#include <filesystem>
#include <fstream>
#include <main/include/vortex.h>
#include <main/include/vortex_internals.h>
#include <string>
#include <ui/editor/app/src/editor.hpp>

#ifndef SAMPLE_MODULE_HPP
#define SAMPLE_MODULE_HPP

namespace EmbeddedFusion {
struct Context {
  std::shared_ptr<ModuleInterface> m_interface;
  std::vector<std::shared_ptr<ModuleUI::MainSketchAppWindow>>
      m_main_sketch_instances;
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
// EMBEDDED_FUSION_API void I_OpenMainSketch();
// Simply call OpenMainSketch with parameters
// EMBEDDED_FUSION_API void I_OpenSketchFunction();
// EMBEDDED_FUSION_API void I_OpenSketchEvent();

// API
EMBEDDED_FUSION_API void OpenMainSketch(const std::string &path);
// EMBEDDED_FUSION_API void OpenSketchFunction();
// EMBEDDED_FUSION_API void OpenSketchEvent();

// Content browser
EMBEDDED_FUSION_API bool IsMainSketch(const std::string &path);
EMBEDDED_FUSION_API void CreateMainSketch(const std::string &path);

// EMBEDDED_FUSION_API void IsSketchFunction();
// EMBEDDED_FUSION_API void CreateSketchFunction();

// Project settings
// EMBEDDED_FUSION_API void AddModuleSettingsToProjectSettings();
// EMBEDDED_FUSION_API void GetModuleSettingsToProjectSettings();

// UI
// EMBEDDED_FUSION_API void SpawnMainSketchEditor();
// EMBEDDED_FUSION_API void SpawnSketchFunctionEditor();

EMBEDDED_FUSION_API std::string GetPath(const std::string &path);
} // namespace EmbeddedFusion

// The code of the module.
namespace EmbeddedFusion {
// Input/Output API
// EMBEDDED_FUSION_API void TranspileSketchToCPP(const std::string path);
} // namespace EmbeddedFusion

#endif // SAMPLE_MODULE_HPP