#include "./src/module.hpp"

#ifndef CEmbeddedFusion
EmbeddedFusion::Context *CEmbeddedFusion = NULL;
#endif

class Module : public ModuleInterface {
public:
  void execute() override {
    // Create the context pointer of this modulse
    EmbeddedFusion::CreateContext();

    // Get the interface pointer (for GUI launcher, from other modules)
    CEmbeddedFusion->m_interface =
        ModuleInterface::GetEditorModuleByName(this->m_name);

    LogInfo(EmbeddedFusion::GetPath("resources/images/main_sketch.png"));
    // Content browser (HIGH LEVEL)
    AddContentBrowserItemIdentifier(ItemIdentifierInterface(
        EmbeddedFusion::IsMainSketch, "efusion:main_sketch", "Main Sketch",
        "#A9193A", EmbeddedFusion::GetPath("resources/images/main_sketch.png"),
        EmbeddedFusion::GetPath("resources/backgrounds/main_sketch.png")));

    AddContentBrowserItemHandler(ItemHandlerInterface(
        "efusion:main_sketch", EmbeddedFusion::OpenMainSketch, "Open",
        "Edit this main sketch",
        EmbeddedFusion::GetPath("resources/icons/main_sketch.png")));

    AddContentBrowserItemCreator(ItemCreatorInterface(
        EmbeddedFusion::CreateMainSketch, "Main Sketch",
        "Edit this main sketch", "#A9193A",
        EmbeddedFusion::GetPath("resources/icons/main_sketch.png")));

    // Content browser (LOW LEVEL), inside HIGH LEVEL components
    // Function
    // Entity
  }

  void destroy() override {
    // Reset module
    this->ResetModule();

    // Clear windows
    // for (auto i : CEmbeddedFusion->m_text_editor_instances) {
    //  CherryApp.DeleteAppWindow(i->GetAppWindow());
    //}

    // Clear context
    // DestroyContext();
  }
};

#ifdef _WIN32
extern "C" __declspec(dllexport) ModuleInterface *create_em() {
  return new Module();
}
#else
extern "C" ModuleInterface *create_em() { return new Module(); }
#endif
