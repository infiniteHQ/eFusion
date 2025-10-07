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

  ensurePrimitive("test", "", "",
                  {{"bool_input1", "bool_input1", "bool_input", nullptr}},
                  {{"bool1", "", "bool", nullptr}}, "#616363", "def", "def",
                  "#CCCCCC", "#CCCCCC", "", "", "", "Simple bool var");
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
  std::cout << "2a" << std::endl;
  Cherry::NodeSystem::NodeInstance ni;
  ni.TypeID = schema_id;
  ni.InstanceID =
      schema_id + "_" + std::to_string(m_Graph.m_InstanciatedNodes.size() + 1);
  ni.Position = {x, y};
  ni.Size = {120.f, 40.f};

  ni.Datas = "{}";
  std::cout << "2112" << std::endl;

  // ⚡ Ajoute le node à la graph
  m_Graph.AddNodeInstance(ni);

  // Reconstruire et rafraîchir
  std::cout << "2b1" << std::endl;
  m_NodeEngine.BuildNodes();
  std::cout << "2b2" << std::endl;
  m_NodeEngine.RefreshNodeGraph();
  std::cout << "2b3" << std::endl;
  m_NodeEngine.RefreshNodeGraphLinks();
  std::cout << "2b4" << std::endl;

  // Positionner le node
  Node *nodePtr = m_NodeEngine.FindNodeByInstanceID(ni.InstanceID);
  if (nodePtr) {
    ed::SetNodePosition(nodePtr->ID, ImVec2(x, y));
  }
  std::cout << "2b" << std::endl;
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

  try {
    // 1) prepare paths
    fs::path buildDir = fs::path(m_Path) / "transpilation" / "build";
    fs::create_directories(buildDir);

    fs::path mainCpp = buildDir / "main.cpp";
    std::ofstream out(mainCpp);
    if (!out.is_open()) {
      std::cerr << "Transpilation: failed to open " << mainCpp << "\n";
      return;
    }

    // 2) header includes
    out << "// Auto-generated transpilation\n";
    out << "#include <Arduino.h>\n";
    out << "#include <string>\n";
    out << "\n";

    // 3) collect all node instances
    auto &nodes = m_Graph.m_InstanciatedNodes;

    // 4) collect variable declarations for data pins
    std::set<std::string> declaredVars;           // var names
    std::map<std::string, std::string> varToType; // var -> cpp type

    // Helper lambda to register a pin variable
    auto registerPinVar = [&](const Cherry::NodeSystem::NodeInstance &ni,
                              const std::string &pinName,
                              const std::string &pinTypeId) {
      if (pinTypeId == "exec")
        return; // no variable for exec
      std::string var = VarNameForPin(ni, pinName);
      std::string cppType = GetCppTypeForPinType(pinTypeId);
      declaredVars.insert(var);
      varToType[var] = cppType;
    };

    // Pre-pass: for each node instance, look up its schema in g_SchemasCache
    // and register variables for inputs/outputs non-exec
    for (const auto &ni : nodes) {
      auto maybeSchema = FindSchemaInfoById(ni.TypeID);
      if (!maybeSchema) {
        // unknown schema: skip but keep remark
        continue;
      }
      const SchemaInfo &schema = *maybeSchema;
      // inputs
      for (const auto &p : schema.inputs) {
        registerPinVar(ni, p.id.empty() ? p.name : p.id, p.type);
      }
      // outputs
      for (const auto &p : schema.outputs) {
        registerPinVar(ni, p.id.empty() ? p.name : p.id, p.type);
      }
    }

    // 5) write global declarations
    out << "// Global pin variables (automatically declared)\n";
    for (const auto &v : declaredVars) {
      auto it = varToType.find(v);
      std::string cppType = (it != varToType.end()) ? it->second : "auto";
      out << cppType << " " << v << ";\n";
    }
    out << "\n";

    // 6) forward prototypes for node functions
    for (const auto &ni : nodes) {
      std::string inst = SanitizeIdentifier(ni.InstanceID);
      out << "void node_" << inst << "();\n";
    }
    out << "\n";

    // 7) Build bodies for each node instance
    std::ostringstream bodies; // accumulate bodies before output
    for (const auto &ni : nodes) {
      auto maybeSchema = FindSchemaInfoById(ni.TypeID);
      if (!maybeSchema) {
        // fallback: stub
        std::string inst = SanitizeIdentifier(ni.InstanceID);
        bodies << "// Stub for unknown schema: " << ni.TypeID << " ("
               << ni.InstanceID << ")\n";
        bodies << "void node_" << inst << "() {\n";
        bodies << "    // Unknown node type '" << ni.TypeID
               << "'. Implement or provide skeleton.\n";
        bodies << "}\n\n";
        continue;
      }

      const SchemaInfo &schema = *maybeSchema;

      // special-case branch (we generate inline)
      if (schema.id == "branch") {
        PopulatePrimitiveBranch(schema, ni, bodies, declaredVars);
        continue;
      }

      // Otherwise try to find an existing skeleton file named <id>.cpp
      // in primitives/ or functions/ (we search both)
      std::vector<fs::path> candidateDirs = {
          fs::path(m_Path) / "primitives" / schema.id,
          fs::path(m_Path) / "functions" / schema.id,
          fs::path(m_Path) / "types" / schema.id,
      };

      bool usedExternalSkeleton = false;
      for (auto &d : candidateDirs) {
        fs::path skeleton = d / (schema.id + ".cpp");
        if (fs::exists(skeleton)) {
          // include skeleton contents as a helper function primitive_<id>
          std::ifstream sk(skeleton);
          if (sk.is_open()) {
            std::string content((std::istreambuf_iterator<char>(sk)),
                                std::istreambuf_iterator<char>());
            // Option A: insert the skeleton content directly into bodies.
            bodies << "// Included skeleton for primitive " << schema.id
                   << " (from " << skeleton << ")\n";
            bodies << content << "\n\n";
            usedExternalSkeleton = true;
            break;
          }
        }
      }

      // If external skeleton present, create a wrapper node function that calls
      // it
      std::string inst = SanitizeIdentifier(ni.InstanceID);
      if (usedExternalSkeleton) {
        bodies << "void node_" << inst << "() {\n";
        bodies << "    // wrapper for primitive " << schema.id << "\n";
        bodies << "    primitive_" << schema.id << "();\n";
        bodies << "}\n\n";
        continue;
      }

      // Otherwise produce a minimal stub that calls a primitive_<id>()
      // placeholder
      bodies << "// Primitive " << schema.id
             << " (auto-generated stub for instance " << ni.InstanceID << ")\n";
      bodies << "void primitive_" << schema.id << "() {\n";
      bodies << "    // TODO: implement primitive '" << schema.id
             << "' or provide a skeleton file in primitives/" << schema.id
             << "/" << schema.id << ".cpp\n";
      bodies << "}\n\n";

      bodies << "void node_" << inst << "() {\n";
      bodies << "    // calls primitive for " << schema.id << "\n";
      bodies << "    primitive_" << schema.id << "();\n";
      bodies << "}\n\n";
    }

    // write bodies to main
    out << bodies.str() << "\n";

    // 8) write setup() and loop()
    // find setup and loop instances
    std::string setupInstance, loopInstance;
    for (const auto &ni : nodes) {
      if (ni.TypeID == "setup")
        setupInstance = ni.InstanceID;
      else if (ni.TypeID == "loop")
        loopInstance = ni.InstanceID;
    }

    out << "// ---- Arduino entry points ----\n";
    out << "void setup() {\n";
    out << "    Serial.begin(115200);\n";
    if (!setupInstance.empty()) {
      out << "    // Transpiled setup node\n";
      out << "    node_" << SanitizeIdentifier(setupInstance) << "();\n";
    } else {
      out << "    // No setup node found in graph\n";
    }
    out << "}\n\n";

    out << "void loop() {\n";
    if (!loopInstance.empty()) {
      out << "    // Transpiled loop node (single call per loop)\n";
      out << "    node_" << SanitizeIdentifier(loopInstance) << "();\n";
    } else {
      out << "    // No loop node found in graph - idle\n";
      out << "    delay(1000);\n";
    }
    out << "}\n";

    out.close();

    std::cout << "Transpilation: main.cpp written to " << mainCpp << "\n";

  } catch (const std::exception &e) {
    std::cerr << "Transpilation exception: " << e.what() << "\n";
  }
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
