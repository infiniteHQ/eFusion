#pragma once
#include "../../../../../../lib/vortex/main/include/vortex.h"
#include "../../../../../../lib/vortex/main/include/vortex_internals.h"

#include <set>

#ifndef VIEWPORT_MAIN_SKETCH_APP_WINDOW_HPP
#define VIEWPORT_MAIN_SKETCH_APP_WINDOW_HPP

static bool g_NeedRefresh = false;
namespace ModuleUI {

class ViewportMainSketchAppWindow
    : public std::enable_shared_from_this<ViewportMainSketchAppWindow> {
public:
  ViewportMainSketchAppWindow(const std::string &path, const std::string &name);

  void menubar();
  std::shared_ptr<Cherry::AppWindow> &GetAppWindow();
  static std::shared_ptr<ViewportMainSketchAppWindow>
  Create(const std::string &path, const std::string &name);
  void SetupRenderCallback();
  void Render();
  void RenderMenubar();
  void RenderRightMenubar();
  void AddSchemasToNodeGraphSpawner();

  void Refresh();
  void Save();

  // ---------------------- Internal caches / helpers ------------------------

  struct PinTypeInfo {
    std::string id;
    std::string name;
    std::string description;
    std::string colorHex;
    std::string category; // "primitive" or "custom"
    std::string cpp_type;
  };

  struct PinDef {
    std::string id;
    std::string name;
    std::string type; // pin type id (refers to PinTypeInfo.id)
    json defaultValue;
  };

  struct SchemaInfo {
    std::string id;
    std::string proper_name;
    std::string proper_logo;
    std::string name;
    std::string name_secondary;
    std::string description;
    std::vector<PinDef> inputs;
    std::vector<PinDef> outputs;
    std::string kind; // "primitive" or "function" or other
    std::string hexcolheader;
    std::string hexcolbg;
    std::string hexcolborder;
    std::string hexcoltext;
    std::string hexcoltextsecondary;
    std::string nodetype;
    std::string logopath;
  };

  std::vector<PinTypeInfo> g_TypesCache;
  std::vector<SchemaInfo> g_SchemasCache;   // primitives + functions
  std::vector<SchemaInfo> g_FunctionsCache; // separate storage optionally

  // Helpers: path helpers
  fs::path typesDir() { return fs::path(m_Path) / "types"; }
  fs::path primitivesDir() { return fs::path(m_Path) / "primitives"; }
  fs::path functionsDir() { return fs::path(m_Path) / "functions"; }
  fs::path srcSetupPinFile() {
    return fs::path(m_Path) / "src" / "setup" / "pin_setup.json";
  }
  fs::path srcMainSketchFile() {
    return fs::path(m_Path) / "src" / "main" / "main_sketch.json";
  }
  fs::path pinSetupDir() { return fs::path(m_Path) / "src" / "setup"; }
  fs::path srcMainDir() { return fs::path(m_Path) / "src" / "main"; }
  static bool writeSchemaToFolder(const SchemaInfo &s, const fs::path &folder) {
    try {
      fs::create_directories(folder);
      fs::path f = folder / "config.json";

      json j;
      j["id"] = s.id;
      j["name"] = s.name;
      j["name_secondary"] = s.name_secondary;
      j["proper_name"] = s.proper_name;
      j["proper_logo"] = s.proper_logo;
      j["description"] = s.description;
      j["kind"] = s.kind;
      j["hexcolheader"] = s.hexcolheader;
      j["hexcolbg"] = s.hexcolbg;
      j["hexcolborder"] = s.hexcolborder;
      j["hexcoltext"] = s.hexcoltext;
      j["hexcoltextsecondary"] = s.hexcoltextsecondary;

      j["nodetype"] = s.nodetype;
      j["logopath"] = s.logopath;

      j["inputs"] = json::array();
      for (auto &pin : s.inputs) {
        json jp;
        jp["id"] = pin.id;
        jp["name"] = pin.name;
        jp["type"] = pin.type;
        if (!pin.defaultValue.is_null())
          jp["default"] = pin.defaultValue;
        j["inputs"].push_back(jp);
      }

      j["outputs"] = json::array();
      for (auto &pin : s.outputs) {
        json jp;
        jp["id"] = pin.id;
        jp["name"] = pin.name;
        jp["type"] = pin.type;
        if (!pin.defaultValue.is_null())
          jp["default"] = pin.defaultValue;
        j["outputs"].push_back(jp);
      }

      std::ofstream out(f);
      if (!out.is_open())
        return false;
      out << j.dump(4);
      return true;
    } catch (...) {
      return false;
    }
  }

  void SaveTypes() {
    try {
      fs::create_directories(typesDir());
      for (const auto &t : g_TypesCache) {
        fs::path folder = typesDir() / t.id;
        fs::create_directories(folder);
        json j;
        j["id"] = t.id;
        j["name"] = t.name;
        j["description"] = t.description;
        j["color"] = t.colorHex;
        j["category"] = t.category;
        j["cpp_type"] = t.cpp_type;

        fs::path out = folder / "type.json";
        std::ofstream ofs(out);
        if (!ofs.is_open()) {
          std::cerr << "SaveTypes: failed to open " << out << std::endl;
          continue;
        }
        ofs << j.dump(4);
      }

      // Also write a global pin_setup.json mirror for quick import (optional)
      json global;
      global["types"] = json::array();
      for (const auto &t : g_TypesCache) {
        global["types"].push_back({{"id", t.id},
                                   {"name", t.name},
                                   {"description", t.description},
                                   {"color", t.colorHex},
                                   {"category", t.category},
                                   {"cpp_type", t.cpp_type}});
      }
      fs::create_directories(pinSetupDir());
      std::ofstream out(srcSetupPinFile());
      if (out.is_open())
        out << global.dump(4);
    } catch (const std::exception &e) {
      std::cerr << "SaveTypes exception: " << e.what() << std::endl;
    }
  }

  void SavePrimitives() {
    try {
      fs::create_directories(primitivesDir());
      for (const auto &s : g_SchemasCache) {
        if (s.kind != "primitive")
          continue;
        fs::path folder = primitivesDir() / s.id;
        fs::create_directories(folder);

        json j;
        j["id"] = s.id;
        j["name"] = s.name;
        j["name_secondary"] = s.name_secondary;
        j["proper_name"] = s.proper_name;
        j["proper_logo"] = s.proper_logo;
        j["description"] = s.description;
        j["kind"] = s.kind;
        j["nodetype"] = s.nodetype;
        j["logopath"] = s.logopath;
        j["hexcolheader"] = s.hexcolheader;
        j["hexcolbg"] = s.hexcolbg;
        j["hexcolborder"] = s.hexcolborder;
        j["hexcoltext"] = s.hexcoltext;
        j["hexcoltextsecondary"] = s.hexcoltextsecondary;

        j["inputs"] = json::array();
        for (const auto &p : s.inputs) {
          j["inputs"].push_back({{"id", p.id},
                                 {"name", p.name},
                                 {"type", p.type},
                                 {"default", p.defaultValue}});
        }
        j["outputs"] = json::array();
        for (const auto &p : s.outputs) {
          j["outputs"].push_back({{"id", p.id},
                                  {"name", p.name},
                                  {"type", p.type},
                                  {"default", p.defaultValue}});
        }

        fs::path out = folder / "config.json";
        std::ofstream ofs(out);
        if (!ofs.is_open()) {
          std::cerr << "SavePrimitives: failed to open " << out << std::endl;
          continue;
        }
        ofs << j.dump(4);

        fs::path cppSkeleton = folder / (s.id + ".cpp");
        if (!fs::exists(cppSkeleton)) {
          std::ofstream sk(cppSkeleton);
          if (sk.is_open()) {
            sk << "// Primitive skeleton for: " << s.id << "\n";
            sk << "// Description: " << s.description << "\n\n";
            sk << "// TODO: implement primitive runtime or transpiler "
                  "mapping\n";
            sk << "void primitive_" << s.id << "() {\n    // ...\n}\n";
          }
        }
      }
    } catch (const std::exception &e) {
      std::cerr << "SavePrimitives exception: " << e.what() << std::endl;
    }
  }

  void SaveFunctions() {
    try {
      fs::create_directories(functionsDir());
      for (const auto &s : g_SchemasCache) {
        if (s.kind != "function")
          continue;
        fs::path folder = functionsDir() / s.id;
        fs::create_directories(folder);

        json j;
        j["id"] = s.id;
        j["name"] = s.name;
        j["name_secondary"] = s.name_secondary;
        j["proper_name"] = s.proper_name;
        j["proper_logo"] = s.proper_logo;
        j["description"] = s.description;
        j["kind"] = s.kind;
        j["nodetype"] = s.nodetype;
        j["logopath"] = s.logopath;
        j["hexcolheader"] = s.hexcolheader;
        j["hexcolbg"] = s.hexcolbg;
        j["hexcolborder"] = s.hexcolborder;
        j["hexcoltext"] = s.hexcoltext;
        j["hexcoltextsecondary"] = s.hexcoltextsecondary;

        j["inputs"] = json::array();
        for (const auto &p : s.inputs) {
          j["inputs"].push_back({{"id", p.id},
                                 {"name", p.name},
                                 {"type", p.type},
                                 {"default", p.defaultValue}});
        }
        j["outputs"] = json::array();
        for (const auto &p : s.outputs) {
          j["outputs"].push_back({{"id", p.id},
                                  {"name", p.name},
                                  {"type", p.type},
                                  {"default", p.defaultValue}});
        }

        fs::path out = folder / "config.json";
        std::ofstream ofs(out);
        if (!ofs.is_open()) {
          std::cerr << "SaveFunctions: failed to open " << out << std::endl;
          continue;
        }
        ofs << j.dump(4);

        // skeleton
        fs::path cppSkeleton = folder / (s.id + ".cpp");
        if (!fs::exists(cppSkeleton)) {
          std::ofstream sk(cppSkeleton);
          if (sk.is_open()) {
            sk << "// Function skeleton for: " << s.id << "\n";
            sk << "// Description: " << s.description << "\n\n";
            sk << "// TODO: implement function logic\n";
            sk << "void function_" << s.id << "() {\n    // ...\n}\n";
          }
        }
      }
    } catch (const std::exception &e) {
      std::cerr << "SaveFunctions exception: " << e.what() << std::endl;
    }
  }

  void SaveMainNodeGraph() {
    try {
      fs::create_directories(srcMainDir());
      std::string graphFile = srcMainSketchFile().string();

      // use NodeGraph's existing API by setting file and calling
      // DumpGraphToJsonFile()
      m_Graph.SetGraphFile(graphFile);
      if (!m_Graph.DumpGraphToJsonFile(&m_NodeCtx)) {
        std::cerr << "SaveMainNodeGraph: failed to dump graph to " << graphFile
                  << std::endl;
      }
    } catch (const std::exception &e) {
      std::cerr << "SaveMainNodeGraph exception: " << e.what() << std::endl;
    }
  }

  // ---------------------- Fetch functions ------------------------

  static std::optional<PinTypeInfo> readTypeFromFolder(const fs::path &folder) {
    try {
      fs::path f = folder / "type.json";
      if (!fs::exists(f))
        return std::nullopt;
      std::ifstream in(f);
      if (!in.is_open())
        return std::nullopt;
      json j;
      in >> j;
      PinTypeInfo t;
      t.id = j.value("id", folder.filename().string());
      t.name = j.value("name", t.id);
      t.description = j.value("description", "");
      t.colorHex = j.value("color", "#FFFFFF");
      t.category = j.value("category", "custom");
      t.cpp_type = j.value("cpp_type", "");
      return t;
    } catch (...) {
      return std::nullopt;
    }
  }

  void FetchTypes() {
    g_TypesCache.clear();

    try {
      if (!fs::exists(typesDir())) {
        // nothing to fetch
        return;
      }

      for (auto &p : fs::directory_iterator(typesDir())) {
        if (!p.is_directory())
          continue;
        auto maybe = readTypeFromFolder(p.path());
        if (maybe) {
          g_TypesCache.push_back(*maybe);

          // register pin in NodeContext
          Cherry::NodeSystem::PinFormat pf(maybe->id, maybe->name,
                                           maybe->colorHex);

          Cherry::NodeSystem::PinShape shape =
              Cherry::NodeSystem::PinShape::Circle;

          if (maybe->category == "flow") {
            shape = Cherry::NodeSystem::PinShape::Flow;
          } else if (maybe->category == "primitive") {
            shape = Cherry::NodeSystem::PinShape::Circle;
          } else if (maybe->category == "event") {
            shape = Cherry::NodeSystem::PinShape::Square;
          }

          try {
            m_NodeCtx.SetupPinFormat(pf);
          } catch (...) {
            std::cerr << "FetchTypes: failed to SetupPinFormat for "
                      << maybe->id << std::endl;
          }
        }
      }

      // Also try to import global pin_setup.json if exists
      if (fs::exists(srcSetupPinFile())) {
        std::ifstream in(srcSetupPinFile());
        if (in.is_open()) {
          json j;
          in >> j;
          if (j.contains("types") && j["types"].is_array()) {
            for (auto &jt : j["types"]) {
              PinTypeInfo t;
              t.id = jt.value("id", "");
              if (t.id.empty())
                continue;
              t.name = jt.value("name", t.id);
              t.description = jt.value("description", "");
              t.colorHex = jt.value("color", "#FFFFFF");
              t.category = jt.value("category", "custom");
              t.cpp_type = jt.value("cpp_type", "");

              Cherry::NodeSystem::PinShape shape =
                  Cherry::NodeSystem::PinShape::Circle;

              if (t.category == "flow") {
                shape = Cherry::NodeSystem::PinShape::Flow;
              } else if (t.category == "primitive") {
                shape = Cherry::NodeSystem::PinShape::Circle;
              } else if (t.category == "event") {
                shape = Cherry::NodeSystem::PinShape::Square;
              }

              // avoid duplicates
              auto it = std::find_if(
                  g_TypesCache.begin(), g_TypesCache.end(),
                  [&](const PinTypeInfo &x) { return x.id == t.id; });
              if (it == g_TypesCache.end()) {
                g_TypesCache.push_back(t);
                m_NodeCtx.SetupPinFormat(Cherry::NodeSystem::PinFormat(
                    t.id, t.name, t.colorHex, shape));
              }
            }
          }
        }
      }
    } catch (const std::exception &e) {
      std::cerr << "FetchTypes exception: " << e.what() << std::endl;
    }
  }

  static std::optional<SchemaInfo>
  readSchemaFromFolder(const fs::path &folder) {
    try {
      fs::path f = folder / "config.json";
      if (!fs::exists(f))
        return std::nullopt;
      std::ifstream in(f);
      if (!in.is_open())
        return std::nullopt;

      json j;
      in >> j;

      SchemaInfo s;
      s.id = j.value("id", folder.filename().string());
      s.name = j.value("name", s.name);
      s.name_secondary = j.value("name_secondary", s.name_secondary);
      s.proper_name = j.value("proper_name", s.proper_name);
      s.proper_logo = j.value("proper_logo", s.proper_logo);
      s.description = j.value("description", "");
      s.kind = j.value("kind", "primitive");
      s.hexcolheader = j.value("hexcolheader", "#FFFFFF");
      s.hexcolbg = j.value("hexcolbg", "#FFFFFF");
      s.hexcolborder = j.value("hexcolborder", "#FFFFFF");
      s.hexcoltext = j.value("hexcoltext", "#FFFFFF");
      s.hexcoltextsecondary = j.value("hexcoltextsecondary", "#FFFFFF");
      s.nodetype = j.value("nodetype", "default");
      s.logopath = j.value("logopath", "");

      if (j.contains("inputs") && j["inputs"].is_array()) {
        for (auto &ji : j["inputs"]) {
          PinDef p;
          p.id = ji.value("id", "");
          p.name = ji.value("name", p.id);
          p.type = ji.value("type", "");
          if (ji.contains("default"))
            p.defaultValue = ji["default"];
          s.inputs.push_back(p);
        }
      }

      if (j.contains("outputs") && j["outputs"].is_array()) {
        for (auto &jo : j["outputs"]) {
          PinDef p;
          p.id = jo.value("id", "");
          p.name = jo.value("name", p.id);
          p.type = jo.value("type", "");
          if (jo.contains("default"))
            p.defaultValue = jo["default"];
          s.outputs.push_back(p);
        }
      }

      return s;
    } catch (...) {
      return std::nullopt;
    }
  }

  void FetchPrimitives() {
    // load primitives into cache and register schemas
    try {
      if (!fs::exists(primitivesDir()))
        return;
      for (auto &p : fs::directory_iterator(primitivesDir())) {
        if (!p.is_directory())
          continue;
        auto maybe = readSchemaFromFolder(p.path());
        if (!maybe)
          continue;
        maybe->kind = maybe->kind.empty() ? "primitive" : maybe->kind;
        g_SchemasCache.push_back(*maybe);

        // create schema in context
        try {
          m_NodeCtx.CreateSchema(maybe->id);
          auto schema = m_NodeCtx.GetSchema(maybe->id);
          if (schema) {
            for (const auto &pin : maybe->inputs)
              schema->AddInputPin(pin.name, pin.type);
            for (const auto &pin : maybe->outputs)
              schema->AddOutputPin(pin.name, pin.type);

            if (maybe->nodetype == "blueprint") {
              schema->SetType(Cherry::NodeSystem::NodeType::Blueprint);
            }

            if (!maybe->name.empty()) {
              schema->SetLabel(maybe->name);
            }

            if (!maybe->name_secondary.empty()) {
              schema->SetSecondLabel(maybe->name_secondary);
            }

            if (!maybe->hexcolheader.empty()) {
              schema->SetHexHeaderColor(maybe->hexcolheader);
            }

            if (!maybe->hexcoltext.empty()) {
              schema->SetLabelHexColor(maybe->hexcoltext);
            }

            if (!maybe->hexcoltextsecondary.empty()) {
              schema->SetSecondLabelHexColor(maybe->hexcoltextsecondary);
            }

            if (!maybe->hexcolbg.empty()) {
              if (maybe->hexcolbg == "def") {
              } else {
                schema->SetHexBackgroundColor(maybe->hexcolbg);
              }
            }

            if (!maybe->hexcolborder.empty()) {
              if (maybe->hexcolborder == "def") {
              } else {
                schema->SetHexBorderColor(maybe->hexcolborder);
              }
            }

            if (!maybe->logopath.empty()) {
              schema->SetLogoPath(maybe->logopath);
            }
          } else {
            std::cerr << "FetchPrimitives: GetSchema returned nullptr for "
                      << maybe->id << std::endl;
          }
        } catch (...) {
          std::cerr << "FetchPrimitives: failed to register schema "
                    << maybe->id << std::endl;
        }

        // ensure skeleton if missing
        fs::path skeleton = p.path() / (maybe->id + ".cpp");
        if (!fs::exists(skeleton)) {
          std::ofstream sk(skeleton);
          if (sk.is_open()) {
            sk << "// Auto-generated primitive skeleton for " << maybe->id
               << "\n";
            sk << "// TODO: implement\n";
          }
        }
      }
    } catch (const std::exception &e) {
      std::cerr << "FetchPrimitives exception: " << e.what() << std::endl;
    }
  }

  void FetchFunctions() {
    try {
      if (!fs::exists(functionsDir()))
        return;
      for (auto &p : fs::directory_iterator(functionsDir())) {
        if (!p.is_directory())
          continue;
        auto maybe = readSchemaFromFolder(p.path());
        if (!maybe)
          continue;
        maybe->kind = maybe->kind.empty() ? "function" : maybe->kind;
        g_SchemasCache.push_back(*maybe);
        g_FunctionsCache.push_back(*maybe);

        // register schema into context
        try {
          m_NodeCtx.CreateSchema(maybe->id);
          auto schema = m_NodeCtx.GetSchema(maybe->id);
          if (schema) {
            for (const auto &pin : maybe->inputs)
              schema->AddInputPin(pin.name, pin.type);
            for (const auto &pin : maybe->outputs)
              schema->AddOutputPin(pin.name, pin.type);

            if (maybe->nodetype == "blueprint") {
              schema->SetType(Cherry::NodeSystem::NodeType::Blueprint);
            }
            if (!maybe->name.empty()) {
              schema->SetLabel(maybe->name);
            }

            if (!maybe->name_secondary.empty()) {
              schema->SetSecondLabel(maybe->name_secondary);
            }
            if (!maybe->hexcolheader.empty()) {
              schema->SetHexHeaderColor(maybe->hexcolheader);
            }

            if (!maybe->hexcoltext.empty()) {
              schema->SetLabelHexColor(maybe->hexcoltext);
            }

            if (!maybe->hexcoltextsecondary.empty()) {
              schema->SetSecondLabelHexColor(maybe->hexcoltextsecondary);
            }

            if (!maybe->hexcolbg.empty()) {
              if (maybe->hexcolbg == "def") {
              } else {
                schema->SetHexBackgroundColor(maybe->hexcolbg);
              }
            }

            if (!maybe->hexcolborder.empty()) {
              if (maybe->hexcolborder == "def") {
              } else {
                schema->SetHexBorderColor(maybe->hexcolborder);
              }
            }

            if (!maybe->logopath.empty()) {
              schema->SetLogoPath(maybe->logopath);
            }
          }
        } catch (...) {
          std::cerr << "FetchFunctions: failed to register schema " << maybe->id
                    << std::endl;
        }

        // ensure skeleton if missing
        fs::path skeleton = p.path() / (maybe->id + ".cpp");
        if (!fs::exists(skeleton)) {
          std::ofstream sk(skeleton);
          if (sk.is_open()) {
            sk << "// Auto-generated function skeleton for " << maybe->id
               << "\n";
            sk << "// TODO: implement\n";
          }
        }
      }
    } catch (const std::exception &e) {
      std::cerr << "FetchFunctions exception: " << e.what() << std::endl;
    }
  }

  void Transpilation();
  void FetchMainNodeGraph() {
    try {
      std::string graphFile = srcMainSketchFile().string();
      m_Graph.SetGraphFile(graphFile);
      bool ok = m_Graph.PopulateGraphFromJsonFile(&m_NodeCtx);
      if (!ok) {
        std::cerr << "FetchMainNodeGraph: unable to load " << graphFile
                  << " (file missing or invalid). Creating empty graph."
                  << std::endl;
        json j;
        j["nodes"] = json::array();
        j["connections"] = json::array();
        fs::create_directories(srcMainDir());
        std::ofstream out(graphFile);
        if (out.is_open()) {
          out << j.dump(4);
          out.close();
          m_Graph.PopulateGraphFromJsonFile(&m_NodeCtx);
        }
        return;
      }

      for (const auto &ni : m_Graph.m_InstanciatedNodes) {
        std::string typeId = ni.TypeID;
        auto schema = m_NodeCtx.GetSchema(typeId);
        if (schema == nullptr) {
          std::string fallbackId = std::string("fallback_") + typeId;
          m_NodeCtx.CreateSchema(fallbackId);
          auto fb = m_NodeCtx.GetSchema(fallbackId);
          if (fb) {
            fb->AddInputPin("InExec", "exec");
            fb->AddOutputPin("OutExec", "exec");

            fb->AddInputPin("InValue", "variant");
            fb->AddOutputPin("OutValue", "variant");
          }
        }
      }

    } catch (const std::exception &e) {
      std::cerr << "FetchMainNodeGraph exception: " << e.what() << std::endl;
    }
  }
  void RegisterBoolVarNode() {
    m_Graph.AddNodeDataType(
        "bool_input",
        // Serializer
        [](const Cherry::NodeSystem::NodeInstance &node) -> json {
          bool val = false;
          try {
            if (node.Datas.is_object() && node.Datas.contains("value")) {
              if (node.Datas["value"].is_boolean()) {
                val = node.Datas["value"].get<bool>();
              } else if (node.Datas["value"].is_string()) {
                std::string s = node.Datas["value"].get<std::string>();
                val = (s == "true" || s == "1");
              }
            }
          } catch (...) {
            val = false;
          }
          return {{"value", val}};
        },
        // Deserializer
        [](Cherry::NodeSystem::NodeInstance &node, const json &j) {
          bool val = false;
          if (j.contains("value")) {
            try {
              if (j["value"].is_boolean()) {
                val = j["value"].get<bool>();
              } else if (j["value"].is_string()) {
                std::string s = j["value"].get<std::string>();
                val = (s == "true" || s == "1");
              }
            } catch (...) {
              val = false;
            }
          }

          if (!node.Datas.is_object())
            node.Datas = json::object();

          node.Datas["value"] = val;
        });

    // Render callback
    m_Graph.SetRenderCallbackForNodeData(
        "bool_input", [this](Cherry::NodeSystem::NodeInstance &node) {
          bool val = false;
          try {
            if (node.Datas.is_object() && node.Datas.contains("value")) {
              if (node.Datas["value"].is_boolean())
                val = node.Datas["value"].get<bool>();
              else if (node.Datas["value"].is_string()) {
                std::string s = node.Datas["value"].get<std::string>();
                val = (s == "true" || s == "1");
              }
            }
          } catch (...) {
            val = false;
          }

          if (ImGui::Checkbox("##bool", &val)) {
            if (!m_Graph.SetNodeData(node.InstanceID, "value", json(val))) {
              if (!node.Datas.is_object())
                node.Datas = json::object();
              node.Datas["value"] = val;
            }
          }
        });
  }

  void SpawnNode(const std::string &schema_id, float x, float y,
                 const std::string &link);

  // Minimum builtins to always provide
  void PopulateMinimum();
  void SpawnMinimal();

  void DrawMainMenu();
  void DrawNodeExplorer();

  // Transpilation :
  // Helpers (place near top of file)
  std::string SanitizeIdentifier(const std::string &s) {
    std::string out;
    for (char c : s) {
      if (isalnum((unsigned char)c) || c == '_')
        out.push_back(c);
      else
        out.push_back('_');
    }
    // avoid starting with digit
    if (!out.empty() && isdigit((unsigned char)out.front()))
      out = std::string("_") + out;
    return out;
  }

  std::string GetCppTypeForPinType(const std::string &pinTypeId) {
    static const std::unordered_map<std::string, std::string> base = {
        {"bool", "bool"},     {"int", "int"},   {"float", "float"},
        {"double", "double"}, {"char", "char"}, {"string", "std::string"},
        {"exec", "void"}};

    auto it = base.find(pinTypeId);
    if (it != base.end())
      return it->second;

    if (!pinTypeId.empty())
      return pinTypeId;
    return "auto";
  }

  std::optional<SchemaInfo> FindSchemaInfoById(const std::string &id) {
    for (const auto &s : g_SchemasCache)
      if (s.id == id)
        return s;
    return std::nullopt;
  }

  std::string VarNameForPin(const Cherry::NodeSystem::NodeInstance &ni,
                            const std::string &pinName) {
    return SanitizeIdentifier(ni.InstanceID + "_" + pinName);
  }
  void PopulatePrimitiveBranch(const SchemaInfo &schema,
                               const Cherry::NodeSystem::NodeInstance &ni,
                               std::ostringstream &outBody,
                               std::set<std::string> &declaredVars) {
    // schema corresponds to "branch" primitive
    // We expect input pin named "cond" (or schema.inputs containing type bool)
    std::string instance = SanitizeIdentifier(ni.InstanceID);

    // find condition pin name in schema inputs (fall back to "cond")
    std::string condPinName = "cond";
    for (const auto &p : schema.inputs) {
      if (p.type == "bool" || p.id == "cond" || p.name == "Condition") {
        condPinName = p.id.empty() ? p.name : p.id;
        break;
      }
    }

    // variable name for condition
    std::string condVar = VarNameForPin(ni, condPinName);
    if (declaredVars.find(condVar) == declaredVars.end()) {
      // declare condition variable (bool) at top-level; caller will collect
      // these
      declaredVars.insert(condVar);
    }

    // Function for this node
    outBody << "// --- branch node: " << ni.InstanceID
            << " (schema: " << schema.id << ") ---\n";
    outBody << "void node_" << instance << "() {\n";
    outBody << "    // evaluate condition (assumed stored in variable: "
            << condVar << ")\n";
    outBody << "    if (" << condVar << ") {\n";

    // follow true output connections
    std::vector<std::string> trueTargets =
        m_Graph.GetAllNodesLinkedToOutputInstanceID(ni.InstanceID, "true");
    if (!trueTargets.empty()) {
      // call first target (if multiple, call them in sequence)
      for (auto &t : trueTargets) {
        std::string tname = SanitizeIdentifier(t);
        outBody << "        node_" << tname << "();\n";
      }
    } else {
      outBody << "        // no target on True\n";
    }

    outBody << "    } else {\n";

    // follow false output connections
    std::vector<std::string> falseTargets =
        m_Graph.GetAllNodesLinkedToOutputInstanceID(ni.InstanceID, "false");
    if (!falseTargets.empty()) {
      for (auto &t : falseTargets) {
        std::string tname = SanitizeIdentifier(t);
        outBody << "        node_" << tname << "();\n";
      }
    } else {
      outBody << "        // no target on False\n";
    }

    outBody << "    }\n";
    outBody << "}\n\n";
  }

private:
  VxContext *ctx;
  bool opened;

  Cherry::NodeSystem::NodeContext m_NodeCtx;
  Cherry::NodeSystem::NodeGraph m_Graph;
  Cherry::NodeEngine m_NodeEngine;

  std::string m_Path;

  // Cherry
  std::shared_ptr<Cherry::AppWindow> m_AppWindow;
  ComponentsPool m_ComponentPool;
};

}; // namespace ModuleUI

#endif // LOGUTILITY_H
