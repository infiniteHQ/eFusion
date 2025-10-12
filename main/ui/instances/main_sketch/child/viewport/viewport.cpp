#include "viewport.hpp"
#include "../../../../../src/module.hpp"

#include <filesystem>
#include <iostream>
#include <string>

namespace ModuleUI {
enum class ExplorerState { MainMenu, ExploringNode, Exit };

struct Explorer {
  ExplorerState state = ExplorerState::MainMenu;
  Node *currentNode = nullptr;
  std::vector<std::string> linkedIDs;
  int selectedPin = -1;
};

Explorer m_Explorer;

ViewportMainSketchAppWindow::ViewportMainSketchAppWindow(
    const std::string &path, const std::string &name) {
  namespace fs = std::filesystem;

  m_AppWindow = std::make_shared<Cherry::AppWindow>(name, name);
  m_AppWindow->SetIcon(
      EmbeddedFusion::GetPath("/resources/icons/viewport.png"));
  std::shared_ptr<Cherry::AppWindow> win = m_AppWindow;

  m_Path = path;

  // -------------------------
  // Init node system context
  // -------------------------
  FetchTypes();
  FetchPrimitives();
  FetchFunctions();

  RegisterBoolVarNode();
  RegisterIntVarNode();
  RegisterFloatVarNode();
  RegisterCharVarNode();

  FetchMainNodeGraph();

  PopulateMinimum(); // inject built-ins (types + primitives)
  AddSchemasToNodeGraphSpawner();

  m_Graph.m_NodeSpawnCallback = [this](const std::string &schema_id, float x,
                                       float y, const std::string &link) {
    this->SpawnNode(schema_id, x, y, link);
  };

  this->ctx = VortexMaker::GetCurrentContext();
}

void ViewportMainSketchAppWindow::PopulateMinimum() {
  // --- Built-in pin types ---
  auto ensureType = [&](const std::string &id, const std::string &name,
                        const std::string &desc, const std::string &color,
                        const std::string &category,
                        const std::string &cpp_type) {
    auto it = std::find_if(g_TypesCache.begin(), g_TypesCache.end(),
                           [&](const PinTypeInfo &t) { return t.id == id; });
    if (it == g_TypesCache.end()) {
      PinTypeInfo t{id, name, desc, color, category, cpp_type};
      g_TypesCache.push_back(t);

      Cherry::NodeSystem::PinShape shape = Cherry::NodeSystem::PinShape::Circle;

      if (category == "flow") {
        shape = Cherry::NodeSystem::PinShape::Flow;
      } else if (category == "primitive") {
        shape = Cherry::NodeSystem::PinShape::Circle;
      } else if (category == "event") {
        shape = Cherry::NodeSystem::PinShape::Square;
      }

      m_NodeCtx.SetupPinFormat(
          Cherry::NodeSystem::PinFormat(t.id, t.name, t.colorHex, shape, desc));
    }
  };

  ensureType("exec", "Execution", "Flow execution pin", "#FFFFFF", "flow",
             "flow");

  ensureType("bool", "Boolean", "Boolean true/false", "#fc0339", "primitive",
             "bool");

  ensureType("bool_input", "Boolean", "Boolean true/false", "#fc0339",
             "primitive",
             "bool"); // bool == fallback

  ensureType("int_input", "Integer Input", "Integer constant input", "#ebd400",
             "primitive", "int");
  ensureType("float_input", "Float Input", "Float constant input", "#b8eb00",
             "primitive", "float");
  ensureType("char_input", "Char Input", "Char constant input", "#0380fc",
             "primitive", "char");

  ensureType("variant", "Variant", "Generic fallback type", "#AAAAAA", "flow",
             "flow");

  ensureType("int", "Integer", "32-bit signed integer", "#ebd400", "primitive",
             "int");

  ensureType("float", "Float", "Floating point number", "#b8eb00", "primitive",
             "float");

  ensureType("char", "Char", "Character", "#0380fc", "primitive", "char");

  ensureType("string", "String", "UTF-8 string", "#03f8fc", "primitive",
             "std::string");

  // --- Built-in primitives ---
  auto ensurePrimitive =
      [&](const std::string &id, const std::string &name,
          const std::string &desc, std::vector<PinDef> inputs,
          std::vector<PinDef> outputs,
          const std::string &hexcolheader = "#B1FF31",
          const std::string &hexcolbg = "#B1FF31",
          const std::string &hexcolborder = "#RR0000",
          const std::string &hexcoltext = "#CCCCCC",
          const std::string &hexcoltextsecondary = "#CCCCCC",
          const std::string &nodetype = "", const std::string &logopath = "",
          const std::string &name_secondary = "",
          const std::string &proper_name = "",
          const std::string &proper_logo = "") {
        auto it = std::find_if(g_SchemasCache.begin(), g_SchemasCache.end(),
                               [&](const SchemaInfo &s) { return s.id == id; });
        if (it == g_SchemasCache.end()) {
          SchemaInfo s;
          s.id = id;
          s.name = name;
          s.name_secondary = name_secondary;
          s.proper_name = proper_name;
          s.proper_logo = proper_logo;
          s.description = desc;
          s.kind = "primitive";
          s.hexcolheader = hexcolheader;
          s.hexcolbg = hexcolbg;
          s.hexcolborder = hexcolborder;
          s.hexcoltext = hexcoltext;
          s.hexcoltextsecondary = hexcoltextsecondary;
          s.logopath = logopath;
          s.nodetype = nodetype;
          s.inputs = std::move(inputs);
          s.outputs = std::move(outputs);
          g_SchemasCache.push_back(s);

          // Register in NodeCtx
          m_NodeCtx.CreateSchema(id);
          auto schema = m_NodeCtx.GetSchema(id);
          if (schema) {
            if (nodetype == "blueprint") {
              schema->SetType(Cherry::NodeSystem::NodeType::Blueprint);
            }

            for (auto &p : s.inputs) {
              schema->AddInputPin(p.id, p.type);
            }

            for (auto &p : s.outputs) {
              schema->AddOutputPin(p.id, p.type);
            }

            if (!name.empty()) {
              schema->SetLabel(name);
            }

            if (!name_secondary.empty()) {
              schema->SetSecondLabel(name_secondary);
            }

            if (!hexcolheader.empty()) {
              schema->SetHexHeaderColor(hexcolheader);
            }

            if (!hexcoltext.empty()) {
              schema->SetLabelHexColor(hexcoltext);
            }

            if (!hexcoltextsecondary.empty()) {
              schema->SetSecondLabelHexColor(hexcoltextsecondary);
            }

            if (!hexcolbg.empty()) {
              if (hexcolbg == "def") {
              } else {
                schema->SetHexBackgroundColor(hexcolbg);
              }
            }

            if (!hexcolborder.empty()) {
              if (hexcolborder == "def") {
              } else {
                schema->SetHexBorderColor(hexcolborder);
              }
            }
            if (!logopath.empty()) {
              schema->SetLogoPath(logopath);
            }
          }
        }
      };

  // Events
  ensurePrimitive("setup", "Setup", "Setup event", {},
                  {{"on_setup", "On setup", "exec", nullptr}}, "#db2c2c", "def",
                  "def", "#CCCCCC", "#e07070", "blueprint",
                  EmbeddedFusion::GetPath("resources/icons/event.png"),
                  "Main program setup");

  ensurePrimitive("loop", "Loop", "Main loop", {},
                  {{"on_loop", "On loop", "exec", nullptr}}, "#db2c2c", "def",
                  "def", "#CCCCCC", "#e07070", "blueprint",
                  EmbeddedFusion::GetPath("resources/icons/event.png"),
                  "Main program loop");

  // Flow control
  ensurePrimitive(
      "branch", "Branch", "Conditional branch",
      {{"exec", "", "exec", nullptr}, {"cond", "Condition", "bool", false}},
      {{"true", "True", "exec", nullptr}, {"false", "False", "exec", nullptr}},
      "#616363", "def", "def", "#CCCCCC", "#CCCCCC", "blueprint",
      EmbeddedFusion::GetPath("resources/icons/if.png"), "", "Branch",
      EmbeddedFusion::GetPath("resources/icons/if.png"));

  ensurePrimitive(
      "is_float_bigger_than_float", ">", "",
      {{"float1", "", "float", nullptr}, {"float1", "", "float", false}},
      {{"bool_result", "", "bool", nullptr}}, "#616363", "def", "def",
      "#CCCCCC", "#CCCCCC", "", "", "", "Float size comparaison",
      EmbeddedFusion::GetPath("resources/icons/if.png"));

  ensurePrimitive("float_to_int", "", "", {{"float1", "", "float", nullptr}},
                  {{"int1", "", "int", nullptr}}, "#616363", "def", "def",
                  "#CCCCCC", "#CCCCCC", "", "", "", "Convert float to int",
                  EmbeddedFusion::GetPath("resources/icons/if.png"));

  ensurePrimitive("bool_variable", "", "",
                  {{"bool_input1", "bool_input1", "bool_input", nullptr}},
                  {{"bool1", "", "bool", nullptr}}, "#616363", "def", "def",
                  "#CCCCCC", "#CCCCCC", "", "", "", "Simple bool var");

  ensurePrimitive("float_variable", "", "",
                  {{"float_input1", "float_input1", "float_input", nullptr}},
                  {{"float1", "", "float", nullptr}}, "#b8eb00", "def", "def",
                  "#CCCCCC", "#CCCCCC", "", "", "", "Simple float var");

  ensurePrimitive("int_variable", "", "",
                  {{"int_input1", "int_input1", "int_input", nullptr}},
                  {{"int1", "", "int", nullptr}}, "#ebd400", "def", "def",
                  "#CCCCCC", "#CCCCCC", "", "", "", "Simple int var");

  ensurePrimitive("char_variable", "", "",
                  {{"char_input1", "char_input1", "char_input", nullptr}},
                  {{"char1", "", "char", nullptr}}, "#0380fc", "def", "def",
                  "#CCCCCC", "#CCCCCC", "", "", "", "Simple char var");
}

void ViewportMainSketchAppWindow::SpawnMinimal() {
  bool hasSetup = false;
  bool hasLoop = false;

  for (const auto &ni : m_Graph.m_InstanciatedNodes) {
    if (ni.TypeID == "setup")
      hasSetup = true;
    else if (ni.TypeID == "loop")
      hasLoop = true;
  }

  if (!hasSetup) {
    SpawnNode("setup", 0.0f, 0.0f, "");
  }

  if (!hasLoop) {
    SpawnNode("loop", 0.0f, 100.0f, "");
  }
}

void ViewportMainSketchAppWindow::AddSchemasToNodeGraphSpawner() {
  auto schemas = m_NodeCtx.GetSchemas();

  for (auto schema : g_SchemasCache) {
    if (schema.description == "Main event")
      continue;

    Cherry::NodeSystem::NodeSpawnPossibility poss;
    poss.proper_name = schema.proper_name;
    poss.proper_description = schema.description;
    poss.proper_logo = schema.proper_logo;
    poss.category = schema.kind;
    poss.schema_id = schema.id;

    m_Graph.AddPossibility(poss);
  }
}

void ViewportMainSketchAppWindow::Refresh() {
  FetchTypes();

  // Primitives
  FetchPrimitives();

  // Functions
  FetchFunctions();

  FetchMainNodeGraph();
  PopulateMinimum();
  AddSchemasToNodeGraphSpawner();

  m_NodeEngine.RefreshNodeGraph();
  m_NodeEngine.RefreshNodeGraphLinks();
}

void ViewportMainSketchAppWindow::Save() {

  m_NodeEngine.SaveNodeGraph();
  SaveTypes();
  SavePrimitives();
  SaveFunctions();
  SaveMainNodeGraph();
}

std::shared_ptr<Cherry::AppWindow> &
ViewportMainSketchAppWindow::GetAppWindow() {
  return m_AppWindow;
}

std::shared_ptr<ViewportMainSketchAppWindow>
ViewportMainSketchAppWindow::Create(const std::string &path,
                                    const std::string &name) {
  auto instance = std::shared_ptr<ViewportMainSketchAppWindow>(
      new ViewportMainSketchAppWindow(path, name));
  instance->SetupRenderCallback();
  return instance;
}

void ViewportMainSketchAppWindow::SetupRenderCallback() {
  auto self = shared_from_this();
  m_AppWindow->SetRenderCallback([self]() {
    if (self) {
      self->Render();
    }
  });
}

void ViewportMainSketchAppWindow::SpawnNode(const std::string &schema_id,
                                            float x, float y,
                                            const std::string &link) {
  Cherry::NodeSystem::NodeInstance ni;
  ni.TypeID = schema_id;
  ni.InstanceID =
      schema_id + "_" + std::to_string(m_Graph.m_InstanciatedNodes.size() + 1);
  ni.Position = {x, y};
  ni.Size = {120.f, 40.f};

  ni.Datas = "{}";

  m_Graph.AddNodeInstance(ni);

  m_NodeEngine.BuildNodes();
  m_NodeEngine.RefreshNodeGraph();
  m_NodeEngine.RefreshNodeGraphLinks();

  Node *nodePtr = m_NodeEngine.FindNodeByInstanceID(ni.InstanceID);
  if (nodePtr) {
    ed::SetNodePosition(nodePtr->ID, ImVec2(x, y));
  }
}

void ViewportMainSketchAppWindow::Render() {
  int width = CherryGUI::GetContentRegionAvail().x;
  int height = CherryGUI::GetContentRegionAvail().y;
  CherryStyle::AddMarginY(5.0f);

  switch (m_Explorer.state) {
  case ExplorerState::MainMenu:
    DrawMainMenu();
    break;
  case ExplorerState::ExploringNode:
    DrawNodeExplorer();
    break;
  case ExplorerState::Exit:
    break;
  }

  auto node_area =
      CherryKit::NodeAreaOpen(CherryID("viewport"), "", width, height,
                              &m_NodeCtx, &m_Graph, &m_NodeEngine);

  static bool first = true;
  if (first) {
    SpawnMinimal();
    first = false;
  }
}

void ViewportMainSketchAppWindow::Transpilation() {
  namespace fs = std::filesystem;

  fs::path buildDir = fs::path(m_Path) / "transpilation" / "build";
  fs::create_directories(buildDir);
  fs::path mainCpp = buildDir / "main.cpp";

  std::ofstream out(mainCpp);
  if (!out.is_open()) {
    std::cerr << "[Transpilation] Error: unable to open " << mainCpp << "\n";
    return;
  }

  out << "// Auto-generated by EmbeddedFusion Transpiler\n";
  out << "#include <Arduino.h>\n";
  out << "#include <cmath>\n";
  out << "#include <string>\n\n";

  std::ostringstream globals;
  std::ostringstream functions;
  std::set<std::string> declaredVars;

  auto Sanitize = [&](const std::string &s) {
    std::string res = s;
    for (auto &c : res)
      if (!std::isalnum(c))
        c = '_';
    return res;
  };

  auto GetCppType = [&](const std::string &pinTypeID) -> std::string {
    if (pinTypeID == "float")
      return "float";
    if (pinTypeID == "int")
      return "int";
    if (pinTypeID == "bool")
      return "bool";
    if (pinTypeID == "char")
      return "char";
    return "auto";
  };

  auto VarName = [&](const NodeSystem::NodeInstance &node,
                     const std::string &pinName) {
    return "var_" + Sanitize(node.InstanceID + "_" + pinName);
  };

  for (auto &instance : m_NodeEngine.m_NodeGraph->m_InstanciatedNodes) {
    auto schemaOpt = FindSchemaInfoById(instance.TypeID);
    if (!schemaOpt.has_value())
      continue;
    const auto &schema = schemaOpt.value();

    for (auto &pin : schema.inputs) {
      std::string name = VarName(instance, pin.id);
      if (declaredVars.insert(name).second)
        globals << GetCppType(pin.type) << " " << name << " = {};\n";
    }

    for (auto &pin : schema.outputs) {
      std::string name = VarName(instance, pin.id);
      if (declaredVars.insert(name).second)
        globals << GetCppType(pin.type) << " " << name << " = {};\n";
    }

    if (instance.Datas.contains("value")) {
      if (instance.Datas["value"].is_boolean()) {
        globals << "bool " << VarName(instance, "value") << " = "
                << (instance.Datas["value"].get<bool>() ? "true" : "false")
                << ";\n";
      } else if (instance.Datas["value"].is_number_float()) {
        globals << "float " << VarName(instance, "value") << " = "
                << instance.Datas["value"].get<float>() << "f;\n";
      } else if (instance.Datas["value"].is_number_integer()) {
        globals << "int " << VarName(instance, "value") << " = "
                << instance.Datas["value"].get<int>() << ";\n";
      }
    }
  }

  out << "// === Variables ===\n" << globals.str() << "\n\n";

  for (auto &instance : m_NodeEngine.m_NodeGraph->m_InstanciatedNodes) {
    auto schemaOpt = FindSchemaInfoById(instance.TypeID);
    if (!schemaOpt.has_value())
      continue;
    const auto &schema = schemaOpt.value();

    std::string fn = "node_" + Sanitize(instance.InstanceID);
    functions << "// --- Node " << instance.InstanceID << " (" << schema.id
              << ") ---\n";
    functions << "void " << fn << "() {\n";

    for (auto &conn : m_NodeEngine.m_NodeGraph->m_Connections) {
      if (conn.NodeInstanceIDB != instance.InstanceID)
        continue;

      auto dstSchemaOpt = FindSchemaInfoById(instance.TypeID);
      if (!dstSchemaOpt.has_value())
        continue;

      bool isExecConn = false;
      for (auto &pin : dstSchemaOpt->inputs)
        if (pin.id == conn.PinIDB && pin.type == "exec")
          isExecConn = true;
      if (isExecConn)
        continue;

      auto *srcNode =
          m_NodeEngine.m_NodeGraph->GetNodeInstance(conn.NodeInstanceIDA);
      if (!srcNode)
        continue;

      std::string src = VarName(*srcNode, conn.PinIDA);
      std::string dst = VarName(instance, conn.PinIDB);
      functions << "    " << dst << " = " << src << ";\n";
    }

    if (schema.id == "bool_variable") {
      std::string in = VarName(instance, "bool_input1");
      std::string out = VarName(instance, "bool1");
      functions << "    " << out << " = " << in << ";\n";
    }

    if (schema.id == "branch") {
      std::string cond = VarName(instance, "cond");
      functions << "    if (" << cond << ") {\n";
      for (auto &conn : m_NodeEngine.m_NodeGraph->m_Connections) {
        if (conn.NodeInstanceIDA == instance.InstanceID &&
            conn.PinIDA == "true")
          functions << "        node_" << Sanitize(conn.NodeInstanceIDB)
                    << "();\n";
      }
      functions << "    } else {\n";
      for (auto &conn : m_NodeEngine.m_NodeGraph->m_Connections) {
        if (conn.NodeInstanceIDA == instance.InstanceID &&
            conn.PinIDA == "false")
          functions << "        node_" << Sanitize(conn.NodeInstanceIDB)
                    << "();\n";
      }
      functions << "    }\n";
    }

    for (auto &conn : m_NodeEngine.m_NodeGraph->m_Connections) {
      if (conn.NodeInstanceIDA != instance.InstanceID)
        continue;

      std::string pin = conn.PinIDA;
      if (pin == "exec" || pin == "on_setup" || pin == "on_loop") {
        functions << "    node_" << Sanitize(conn.NodeInstanceIDB) << "();\n";
      }
    }

    functions << "}\n\n";
  }

  out << "// === Nodes functions ===\n" << functions.str() << "\n";

  out << "void setup() {\n";
  out << "    Serial.begin(115200);\n";
  for (auto &instance : m_NodeEngine.m_NodeGraph->m_InstanciatedNodes)
    if (instance.TypeID == "setup")
      out << "    node_" << Sanitize(instance.InstanceID) << "();\n";
  out << "}\n\n";

  out << "void loop() {\n";
  for (auto &instance : m_NodeEngine.m_NodeGraph->m_InstanciatedNodes)
    if (instance.TypeID == "loop")
      out << "    node_" << Sanitize(instance.InstanceID) << "();\n";
  out << "    delay(1);\n";
  out << "}\n";

  out.close();
}

void ViewportMainSketchAppWindow::DrawMainMenu() {
  ImGui::Text("-------");
  if (ImGui::Button("Setup")) {
    m_Explorer.currentNode = m_NodeEngine.FindNodeByInstanceID("setup_1");
    if (m_Explorer.currentNode)
      m_Explorer.state = ExplorerState::ExploringNode;
  }
  if (ImGui::Button("Loop")) {
    m_Explorer.currentNode = m_NodeEngine.FindNodeByInstanceID("loop_1");
    if (m_Explorer.currentNode)
      m_Explorer.state = ExplorerState::ExploringNode;
  }
  if (ImGui::Button("Quitter")) {
    m_Explorer.state = ExplorerState::Exit;
  }
}
void ViewportMainSketchAppWindow::DrawNodeExplorer() {
  Node *node = m_Explorer.currentNode;
  if (!node) {
    ImGui::Text("Node not founded");
    m_Explorer.state = ExplorerState::MainMenu;
    return;
  }

  CherryKit::SeparatorText(("Exploring: " + node->Name).c_str());
  ImGui::Spacing();

  if (!node->Inputs.empty()) {
    ImGui::Text("Inputs");
    ImGui::Separator();

    for (auto &input : node->Inputs) {
      std::string pinStr = input.Name;
      auto linked =
          m_Graph.GetAllNodesLinkedToInputInstanceID(node->InstanceID, pinStr);

      ImGui::Text("Pin: %s (%s)", input.Name.c_str(), pinStr.c_str());

      if (linked.empty()) {
        ImGui::BulletText("No connection");
      } else {
        for (auto &conn : linked) {
          Node *target = m_NodeEngine.FindNodeByInstanceID(conn);
          if (target) {
            if (ImGui::Selectable(("<- " + target->Name).c_str())) {
              m_Explorer.currentNode = target;
            }
          } else {
            ImGui::BulletText("Invalid connection : %s", conn.c_str());
          }
        }
      }
      ImGui::Spacing();
    }
  }

  if (!node->Outputs.empty()) {
    ImGui::Text("Outputs");
    ImGui::Separator();

    for (auto &output : node->Outputs) {
      std::string pinStr = output.Name;
      auto linked =
          m_Graph.GetAllNodesLinkedToOutputInstanceID(node->InstanceID, pinStr);

      ImGui::Text("Pin: %s (%s)", output.Name.c_str(), pinStr.c_str());

      if (linked.empty()) {
        ImGui::BulletText("No connection");
      } else {
        for (auto &conn : linked) {
          Node *target = m_NodeEngine.FindNodeByInstanceID(conn);
          if (target) {
            if (ImGui::Selectable(("-> " + target->Name).c_str())) {
              m_Explorer.currentNode = target;
            }
          } else {
            ImGui::BulletText("Invalid connection : %s", conn.c_str());
          }
        }
      }
      ImGui::Spacing();
    }
  }

  ImGui::Separator();
  if (ImGui::Button("Back to menu")) {
    m_Explorer.state = ExplorerState::MainMenu;
  }
}

}; // namespace ModuleUI
