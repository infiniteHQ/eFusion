#include "./src/module.hpp"

class infintiehq_efusion : public ModuleInterface {
public:
  std::shared_ptr<eFusion::Context> ctx;

  void execute() override {
    ctx = eFusion::create_context();

    auto m = ModuleInterface::get_editor_module_by_name(this->name());
    eFusion::get_current_context()->m_interface = m;

    this->add_content_browser_item_handler(ItemHandlerInterface(
        "file_cpp", eFusion::StartTextEditorInstance, "Edit",
        "Edit this C++ file", eFusion::get_path("resources/icons/edit.png")));
    this->add_content_browser_item_handler(ItemHandlerInterface(
        "file_lua", eFusion::StartTextEditorInstance, "Edit",
        "Edit this Lua file", eFusion::get_path("resources/icons/edit.png")));
    this->add_content_browser_item_handler(ItemHandlerInterface(
        "file_json", eFusion::StartTextEditorInstance, "Edit",
        "Edit this JSON file", eFusion::get_path("resources/icons/edit.png")));
    this->add_content_browser_item_handler(
        ItemHandlerInterface("file_hpp", eFusion::StartTextEditorInstance,
                             "Edit", "Edit this C++ header file",
                             eFusion::get_path("resources/icons/edit.png")));
    this->add_content_browser_item_handler(ItemHandlerInterface(
        "file_c", eFusion::StartTextEditorInstance, "Edit", "Edit this C file",
        eFusion::get_path("resources/icons/edit.png")));
    this->add_content_browser_item_handler(
        ItemHandlerInterface("file_h", eFusion::StartTextEditorInstance,
                             "Edit", "Edit this C header file",
                             eFusion::get_path("resources/icons/edit.png")));
    this->add_content_browser_item_handler(
        ItemHandlerInterface("file_python", eFusion::StartTextEditorInstance,
                             "Edit", "Edit this Python file",
                             eFusion::get_path("resources/icons/edit.png")));

    this->add_content_browser_item_identifier(ItemIdentifierInterface(
        eFusion::IsValidFile, "text_edit:superfile", "Super file", "#553333"));

    this->set_credits_file(eFusion::get_path("CREDITS"));
    this->add_documentation("Take the editor", "Edit a txt file",
                            eFusion::get_path("docs/main.md"));
    this->add_documentation("Take the editor", "Find specific text",
                            eFusion::get_path("docs/main.md"));
  }

  void init_ui() override {
    // CherryApp.AddFont(
    //     "JetBrainsMono",
    //     eFusion::get_path("resources/fonts/JetBrainsMono-Regular.ttf"), 40.0f);
  }

  void destroy() override {
    // Reset module
    this->reset_module();

    // Clear windows
    for (auto i : eFusion::get_current_context()->m_text_editor_instances) {
      CherryApp.DeleteAppWindow(i->GetAppWindow());
    }

    eFusion::destroy_context(ctx);
    ctx.reset();
  }
};

#ifdef _WIN32
extern "C" __declspec(dllexport) ModuleInterface *create_em() {
  return new infintiehq_efusion();
}
#else
extern "C" ModuleInterface *create_em() { return new infintiehq_efusion(); }
#endif
