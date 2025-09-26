#include "./src/module.hpp"
#include "./ui/main/ui.hpp"

#ifndef CEmbeddedFusion
EmbeddedFusion::Context *CEmbeddedFusion = NULL;
#endif

class Module : public ModuleInterface {
public:
  void execute() override {
    // Create the context pointer of this module
    EmbeddedFusion::CreateContext();

    // Get the interface pointer (for GUI launcher, from other modules)
    CEmbeddedFusion->m_interface =
        ModuleInterface::GetEditorModuleByName(this->m_name);

    // Adding functions
    this->AddFunction(EmbeddedFusion::HelloWorld, "HelloWorld");
    this->AddFunction(EmbeddedFusion::FunctionWithArg, "Arg");
    this->AddFunction(EmbeddedFusion::FunctionWithArgRet, "ArgRet");
    this->AddFunction(EmbeddedFusion::FunctionWithRet, "Ret");

    this->AddOutputEvent(EmbeddedFusion::OutputHandleHello,
                         "OutputHandleHello");
    this->AddInputEvent(EmbeddedFusion::InputHello, "InputHello");

    this->AddContentBrowserItemHandler(ItemHandlerInterface(
        "file_txt", EmbeddedFusion::StartTextEditorInstance, "Edit",
        "Edit this txt file",
        EmbeddedFusion::GetPath("resources/icons/edit.png")));

    this->AddContentBrowserItemHandler(ItemHandlerInterface(
        "text_edit:superfile", EmbeddedFusion::StartTextEditorInstance,
        "Super Edit", "Edit this txt file",
        EmbeddedFusion::GetPath("resources/icons/edit.png")));

    this->AddContentBrowserItemIdentifier(ItemIdentifierInterface(
        EmbeddedFusion::IsValidFile, "text_edit:superfile", "Super file",
        "#553333"));

    // SetContentBrowserSaveAllCallback();
    // AddMainSettingsEntry()
    // AddContentBrowserCreationPossibility();

    {
      ArgumentValues values("{\"name\":\"hohoho\"}");
      ReturnValues ret;
      this->CallInputEvent(this->m_name, "InputHello", values, ret);
    }

    {
      ArgumentValues values("{\"name\":\"hohoho\"}");
      ReturnValues ret;
      this->CallOutputEvent("OutputHandleHello", values, ret);
    }

    {
      ArgumentValues values("{\"name\":\"hohoho\"}");
      this->ExecuteFunction("Arg", values);
    }

    {
      ArgumentValues values("{\"name\":\"hohoho\"}");
      ReturnValues ret;
      this->ExecuteFunction("ArgRet", values, ret);
      std::cout << "The return of ArgRet is : "
                << ret.GetJsonValue()["time"].get<std::string>() << std::endl;
    }

    {
      ReturnValues ret;
      this->ExecuteFunction("Ret", ret);
      std::cout << "The return of Ret is : "
                << ret.GetJsonValue()["time"].get<std::string>() << std::endl;
    }
  }

  void destroy() override {
    // Reset module
    this->ResetModule();

    // Clear windows
    for (auto i : CEmbeddedFusion->m_text_editor_instances) {
      CherryApp.DeleteAppWindow(i->GetAppWindow());
    }

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
