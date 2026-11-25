#include "./src/module.hpp"

class infintiehq_efusion : public ModuleInterface {
public:
  std::shared_ptr<eFusion::Context> ctx;

  void execute() override {
    ctx = eFusion::create_context();

    auto m = ModuleInterface::get_editor_module_by_name(this->name());
    eFusion::get_current_context()->m_interface = m;

    // Item handlers
    this->add_content_browser_item_handler(
        ItemHandlerInterface("efusion:hardware_profile", eFusion::open_hardware_profile_editor,
                             "Open", "View or edit this hardware schema",
                             eFusion::get_path("assets/resources/icons/edit.png")));
                             this->set_toolbar_main_logo_path(eFusion::get_path("assets/resources/icons/mcu.png"));

    // Item identifiers
    this->add_content_browser_item_identifier(ItemIdentifierInterface(
        eFusion::is_hardware_profile, "efusion:hardware_profile", "Hardware Profile", "#ff7300", eFusion::get_path("/assets/resources/icons/mcu_simple.png")));

    this->add_content_browser_item_identifier(ItemIdentifierInterface(
        eFusion::is_firmware_code_function, "efusion:firmware_code_function", "Code function", "#ffc800", eFusion::get_path("/assets/resources/icons/code_simple.png")));

    this->add_content_browser_item_identifier(ItemIdentifierInterface(
        eFusion::is_firmware_node_function, "efusion:firmware_node_funtion", "Node function", "#ffc800", eFusion::get_path("/assets/resources/icons/node_simple.png")));

    // Toolbars items
    this->add_toolbar_handler(ToolbarHandlerInterface(eFusion::create_hardware_profile, "Hardware Profile", "Create hardware profile", "Create new", eFusion::get_path("assets/resources/icons/mcu.png")));
    this->add_toolbar_handler(ToolbarHandlerInterface(eFusion::create_firmware_code_function, "Firmware Code Function", "Create firmware code function", "Create new", eFusion::get_path("assets/resources/icons/code.png")));
    this->add_toolbar_handler(ToolbarHandlerInterface(eFusion::create_firmware_node_function, "Firmware Node Function", "Create firmware node function", "Create new", eFusion::get_path("assets/resources/icons/node.png")));
    this->add_toolbar_handler(ToolbarHandlerInterface(eFusion::create_firmware_node_function, "Uploader", "Open the firmware upload utility", "Utilities"));

    this->add_toolbar_topic_logo("Create new", eFusion::get_path("assets/resources/icons/add.png"));
    this->add_toolbar_topic_logo("Utilities", eFusion::get_path("assets/resources/icons/tool.png"));

    // Content browser creator
    this->add_content_browser_item_creator(ItemCreatorInterface([this](const std::string& path){
      eFusion::create_hardware_profile_in_path(path);
    }, "Hardware Profile", "desc", "#ff7300", eFusion::get_path("/assets/resources/icons/mcu.png")));
    this->add_content_browser_item_creator(ItemCreatorInterface([this](const std::string& path){
      eFusion::create_firmware_code_function_in_path(path);
    }, "Firmware Code Function", "f", "#ffc800", eFusion::get_path("assets/resources/icons/code.png")));
    this->add_content_browser_item_creator(ItemCreatorInterface([this](const std::string& path){
      eFusion::create_firmware_node_function_in_path(path);
    }, "Firmware Node Function", "f", "#ffc800", eFusion::get_path("assets/resources/icons/node.png")));
    this->add_content_browser_item_creator(ItemCreatorInterface([this](const std::string& path){
      eFusion::create_firmware_schematic_in_path(path);
    }, "Firmware Schematic", "f", "#ffc800", eFusion::get_path("assets/resources/icons/schematic.png")));



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
    for (auto i : eFusion::get_current_context()->hardware_profile_editor_instances) {
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
