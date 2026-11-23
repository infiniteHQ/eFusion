#include "./src/module.hpp"

class infintiehq_codeedit : public ModuleInterface {
public:
  std::shared_ptr<CodeEdit::Context> ctx;

  void execute() override {
    ctx = CodeEdit::create_context();

    auto m = ModuleInterface::get_editor_module_by_name(this->name());
    CodeEdit::get_current_context()->m_interface = m;

    this->add_content_browser_item_handler(ItemHandlerInterface(
        "file_cpp", CodeEdit::StartTextEditorInstance, "Edit",
        "Edit this C++ file", CodeEdit::get_path("resources/icons/edit.png")));
    this->add_content_browser_item_handler(ItemHandlerInterface(
        "file_lua", CodeEdit::StartTextEditorInstance, "Edit",
        "Edit this Lua file", CodeEdit::get_path("resources/icons/edit.png")));
    this->add_content_browser_item_handler(ItemHandlerInterface(
        "file_json", CodeEdit::StartTextEditorInstance, "Edit",
        "Edit this JSON file", CodeEdit::get_path("resources/icons/edit.png")));
    this->add_content_browser_item_handler(
        ItemHandlerInterface("file_hpp", CodeEdit::StartTextEditorInstance,
                             "Edit", "Edit this C++ header file",
                             CodeEdit::get_path("resources/icons/edit.png")));
    this->add_content_browser_item_handler(ItemHandlerInterface(
        "file_c", CodeEdit::StartTextEditorInstance, "Edit", "Edit this C file",
        CodeEdit::get_path("resources/icons/edit.png")));
    this->add_content_browser_item_handler(
        ItemHandlerInterface("file_h", CodeEdit::StartTextEditorInstance,
                             "Edit", "Edit this C header file",
                             CodeEdit::get_path("resources/icons/edit.png")));
    this->add_content_browser_item_handler(
        ItemHandlerInterface("file_python", CodeEdit::StartTextEditorInstance,
                             "Edit", "Edit this Python file",
                             CodeEdit::get_path("resources/icons/edit.png")));

    this->add_content_browser_item_identifier(ItemIdentifierInterface(
        CodeEdit::IsValidFile, "text_edit:superfile", "Super file", "#553333"));

    this->set_credits_file(CodeEdit::get_path("CREDITS"));
    this->add_documentation("Take the editor", "Edit a txt file",
                            CodeEdit::get_path("docs/main.md"));
    this->add_documentation("Take the editor", "Find specific text",
                            CodeEdit::get_path("docs/main.md"));
  }

  void init_ui() override {
    // CherryApp.AddFont(
    //     "JetBrainsMono",
    //     CodeEdit::get_path("resources/fonts/JetBrainsMono-Regular.ttf"), 40.0f);
  }

  void destroy() override {
    // Reset module
    this->reset_module();

    // Clear windows
    for (auto i : CodeEdit::get_current_context()->m_text_editor_instances) {
      CherryApp.DeleteAppWindow(i->GetAppWindow());
    }

    CodeEdit::destroy_context(ctx);
    ctx.reset();
  }
};

#ifdef _WIN32
extern "C" __declspec(dllexport) ModuleInterface *create_em() {
  return new infintiehq_codeedit();
}
#else
extern "C" ModuleInterface *create_em() { return new infintiehq_codeedit(); }
#endif
