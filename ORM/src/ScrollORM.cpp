#include "../include/ScrollORM.hpp"

const std::unordered_map<std::string, std::pair<std::string, std::string>> dataTypes = {
    {"String", {"std::string", "TEXT"}},
    {"Int", {"int", "INTEGER"}},
    {"Float", {"float", "REAL"}},
    {"Bool", {"bool", "BOOLEAN"}},
    {"Date", {"std::tm", "DATE"}},
    {"DateTime", {"std::tm", "TIMESTAMP"}},
    {"Time", {"std::tm", "TIME"}},
    {"Binary", {"std::string", "BYTEA"}},
    {"UUID", {"std::string", "UUID"}}
};

const std::string modelsFileTemplate = R"(
#include <string>
#include <vector>
#include <ctime>
#include "Database.hpp"

{{models}}
)";

const std::string classTemplate = R"(
class {{model name}} {
public:
  {{model name}}() = default;
  ~{{model name}}() = default;

{{fields}}

  {{ORM methods}}
};
)";

const std::string conversionTemplate = R"(
template<>
struct soci::type_conversion<{{model name}}> {
  typedef soci::values base_type;

  static void from_base(soci::values const& v, soci::indicator ind, {{model name}}& p) {
{{from setters}}
  }

  static void to_base(const {{model name}}& p, soci::values& v, soci::indicator& ind) {
{{to setters}}
    ind = soci::i_ok;
  }
};
)";

void reflectModels(json config) {
  if (!config.contains("database")) {
    return;
  }

  // find all header files inside config["database"]["models_location"]

  if (!config["database"].contains("models_location")) {
    std::cerr << "\033[1;31m[-] Models location not found\033[0m" << std::endl;
    return;
  }

  std::string modelsLocation =
      config["database"]["models_location"].get<std::string>();
  std::vector<std::string> modelFiles;

  std::string modelsFile = modelsFileTemplate;
  std::string models;

#if defined(__linux__) || defined(__APPLE__)
  DIR *dir;
  dirent *ent;

  if ((dir = opendir(modelsLocation.c_str())) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      if (ent->d_type == DT_REG &&
          ((std::string)ent->d_name).find(".scrl") != std::string::npos) {
        modelFiles.push_back(ent->d_name);
      }
    }
    closedir(dir);
  }

#elif defined(_WIN32)
  std::cout << "\033[1;31mLINE 329 MODEL REFLECTION\033[0m" << std::endl;
#endif

  std::vector<std::pair<std::string, std::string>> modelDefinitions;
  std::vector<std::vector<std::pair<std::string, std::string>>>
      fieldDefinitions;

  for (auto model : modelFiles) {
    std::string path = modelsLocation + "/" + model;
    std::ifstream file(path);
    std::string line;

    if (!file.is_open()) {
      std::cerr << "Error opening file: " << path << std::endl;
      return;
    }

    unsigned int lineCount = 0;
    int inModel = -1;
    std::vector<std::pair<std::string, std::string>> fields;

    while (std::getline(file, line)) {
      ++lineCount;
      if (inModel == -1) {
        size_t modelDefinitionIndex = line.find("model");

        if (modelDefinitionIndex != std::string::npos) {
          std::string modelName;
          size_t pos = modelDefinitionIndex + 5;
          bool foundName = false;

          for (auto token : line.substr(pos)) {
            ++pos;
            if (isalnum(token) || token == '_' || token == '-') {
              modelName += token;
              foundName = true;
            } else if (std::isspace(token) && !foundName) {
              continue;
            } else if (std::isspace(token) && foundName) {
              break;
            } else {
              std::cerr << "Expected name for model at " << path << ":"
                        << lineCount << ", got: '" << token << "'" << std::endl;
              return;
            }
          }

          if (modelName.empty() || modelName == "model") {
            std::cerr << "Invalid or empty model name at " << path << ":"
                      << lineCount << std::endl;
            return;
          }

          bool foundColon = false;
          for (auto token : line.substr(pos)) {
            ++pos;

            if (std::isspace(token) && !foundColon) {
              continue;
            } else if (token == ':') {
              foundColon = true;
              break;
            }
          }

          if (!foundColon) {
            std::cerr << "Expected colon after model name at " << path << ":"
                      << lineCount << std::endl;
            return;
          }

          std::string tableName;
          foundName = false;
          for (auto token : line.substr(pos)) {
            ++pos;

            if (isalnum(token) || token == '_' || token == '-') {
              tableName += token;
              foundName = true;
            } else if (std::isspace(token) && !foundName) {
              continue;
            } else if (std::isspace(token) && foundName) {
              break;
            } else {
              std::cerr << "Expected table name for model at " << path << ":"
                        << lineCount << ", got: '" << token << "'" << std::endl;
              return;
            }
          }

          if (tableName.empty() || tableName == ":" || tableName == "model") {
            std::cerr << "Invalid or empty table name at " << path << ":"
                      << lineCount << std::endl;
            return;
          }
          modelDefinitions.push_back(std::make_pair(modelName, tableName));
          inModel = lineCount;
          fields.clear();
        }
      }

      if (inModel != -1 && lineCount > inModel) {
        if (line.find("}") != std::string::npos) {
          inModel = -1;
          fieldDefinitions.push_back(fields);
        } else {
          std::string fieldName = "";
          std::string fieldType = "";
          bool foundColon = false;
          int pos = -1;

          for (auto token : line) {
            ++pos;
            if (std::isspace(token) && fieldName.empty()) {
              continue;
            } else if (std::isspace(token) && !fieldName.empty()) {
              break;
            } else if ((isalnum(token) || token == '_' || token == '-') &&
                       !foundColon) {
              fieldName += token;
            }
          }

          for (auto token : line.substr(pos)) {
            ++pos;
            if (token == ':') {
              foundColon = true;
              break;
            }
          }

          if (!fieldName.empty() && !foundColon) {
            std::cerr << "Expected colon after field name at " << path << ":"
                      << lineCount << std::endl;
            return;
          } else if (fieldName.empty() && foundColon) {
            std::cerr << "Expected field name before colon at " << path << ":"
                      << lineCount << std::endl;
            return;
          }

          for (auto token : line.substr(pos)) {
            ++pos;
            if (std::isspace(token) && fieldType.empty()) {
              continue;
            } else if (std::isspace(token) && !fieldType.empty()) {
              break;
            } else {
              fieldType += token;
            }
          }

          if (fieldType.empty() && foundColon) {
            std::cerr << "Expected field type after colon at " << path << ":"
                      << lineCount << std::endl;
            return;
          } else if (!fieldName.empty() && !fieldType.empty() && foundColon) {
            fields.push_back(std::make_pair(fieldName, fieldType));
          }
        }
      }
    }

    for (int i = 0; i < modelDefinitions.size(); ++i) {
      auto [modelName, tableName] = modelDefinitions[i];
      auto modelFields = fieldDefinitions[i];
      
      std::string loweredModelName = modelName;
      loweredModelName[0] = std::tolower(loweredModelName[0]);
      
      std::string conversion = conversionTemplate;
      std::string model = classTemplate;
      std::string fields;
      std::string Select;
      std::string UpdateBy;
      std::string DeleteBy;
      std::string Insert;
      std::string ORMMethods;
      std::string sociUpdateFields;
      std::string sociUpdateUses;
      std::string sociInsertFields;
      std::string fromSetters;
      std::string toSetters;

      Select += "static std::vector<" + modelName + "> SelectAll() {\n"
                  "\t\tsoci::session* sql = Database::GetInstance()->GetSession();\n"
                  "\t\tsoci::rowset<" + modelName + "> modelsRS = (sql->prepare << \"SELECT * FROM " + tableName + "\");\n"
                  "\t\tstd::vector<" + modelName + "> models;\n"
                  "\t\tstd::move(modelsRS.begin(), modelsRS.end(), std::back_inserter(models));\n"
                  "\t\treturn models;\n"
                  "\t}\n";

      Insert += "\tstatic " + modelName + " Insert(const " + modelName + "& " + loweredModelName + ") {\n"
                "\t\tsoci::session* sql = Database::GetInstance()->GetSession();\n"
                "\t\t" + modelName + " model;\n"
                "\t\t*sql << \"INSERT INTO " + tableName + " (";
      
      for (auto [fieldName, fieldType] : modelFields) {
        std::string capitalizedFieldName = fieldName;
        capitalizedFieldName[0] = std::toupper(capitalizedFieldName[0]);        

        fromSetters += "\t\tp." + fieldName + " = v.get<" + dataTypes.at(fieldType).first + ">(\"" + fieldName + "\");\n";
        toSetters += "\t\tv.set(\"" + fieldName + "\", p." + fieldName + ");\n";
        fields += "\t" + dataTypes.at(fieldType).first + " " + fieldName + ";\n";
        sociUpdateFields += fieldName + "=:" + fieldName + ", ";
        sociUpdateUses += "soci::use(" + loweredModelName + "." + fieldName + "), ";
        sociInsertFields += ":" + fieldName + ", ";
        Insert += fieldName + ", ";
       
        Select += "\tstatic std::vector<" + modelName + "> SelectBy" + capitalizedFieldName + "(const " + dataTypes.at(fieldType).first + "& " + fieldName + ") {\n"
                  "\t\tsoci::session* sql = Database::GetInstance()->GetSession();\n"
                  "\t\tsoci::rowset<" + modelName + "> modelsRS = (sql->prepare << \"SELECT * FROM " + tableName + " WHERE " + fieldName + " = :" + fieldName + "\", soci::use(" + fieldName + "));\n"
                  "\t\tstd::vector<" + modelName + "> models;\n"
                  "\t\tstd::move(modelsRS.begin(), modelsRS.end(), std::back_inserter(models));\n"
                  "\t\treturn models;\n"
                  "\t}\n";

        DeleteBy += "\tstatic std::vector<" + modelName + "> DeleteBy" + capitalizedFieldName + "(const " + dataTypes.at(fieldType).first + "& " + fieldName + ") {\n"
                    "\t\tsoci::session* sql = Database::GetInstance()->GetSession();\n"
                    "\t\tstd::vector<" + modelName + "> models;\n"
                    "\t\tsoci::rowset<" + modelName + "> modelRS = (sql->prepare << \"DELETE FROM " + tableName + " WHERE " + fieldName + " = :" + fieldName + " RETURNING *\", soci::use(" + fieldName + "));\n"
                    "\t\tstd::move(modelRS.begin(), modelRS.end(), std::back_inserter(models));\n"
                    "\t\treturn models;\n"
                    "\t}\n";
      }

      Insert.replace(Insert.find_last_of(", ") - 1, 2, ") VALUES (" + sociInsertFields.replace(sociInsertFields.find_last_of(", ") - 1, 2, ") RETURNING * \", soci::use(" + loweredModelName + "), soci::into(model);\n"));
      
      sociUpdateFields.replace(sociUpdateFields.find_last_of(", ") - 1, 2, " ");
       
      for (auto [fieldName, fieldType] : modelFields) {
        std::string capitalizedFieldName = fieldName;
        capitalizedFieldName[0] = std::toupper(capitalizedFieldName[0]);
        

        UpdateBy += "\tstatic std::vector<"+modelName+"> UpdateBy" + capitalizedFieldName + "(const " + dataTypes.at(fieldType).first + "& " + fieldName + ", const " + modelName + "& " + loweredModelName + ") {"
                    "\t\tsoci::session* sql = Database::GetInstance()->GetSession();\n"
                    "\t\tstd::vector<" + modelName + "> models;\n"
                    "\t\tsoci::rowset<" + modelName + "> modelRS = (sql->prepare << \"UPDATE " + tableName + " SET " + sociUpdateFields + "WHERE " + fieldName + "=:" + capitalizedFieldName + "_" + " RETURNING *\", " + sociUpdateUses + "soci::use(" + fieldName + "));\n"
                    "\t\tstd::move(modelRS.begin(), modelRS.end(), std::back_inserter(models));\n"
                    "\t\treturn models;\n" 
                    "\t}\n";
      }

      Insert += "\t\treturn model;\n" 
                "\t}\n";

      ORMMethods += Select + Insert + DeleteBy + UpdateBy;

      model.replace(model.find("{{model name}}"), 14, modelName);
      model.replace(model.find("{{model name}}"), 14, modelName);
      model.replace(model.find("{{model name}}"), 14, modelName);
      model.replace(model.find("{{fields}}"), 10, fields);
      model.replace(model.find("{{ORM methods}}"), 15, ORMMethods);
    
      conversion.replace(conversion.find("{{model name}}"), 14, modelName);
      conversion.replace(conversion.find("{{model name}}"), 14, modelName);
      conversion.replace(conversion.find("{{model name}}"), 14, modelName);
      conversion.replace(conversion.find("{{from setters}}"), 16, fromSetters);
      conversion.replace(conversion.find("{{to setters}}"), 14, toSetters);

      models += model;
      models += "\n";
      models += conversion;
    }

    file.close();
  } 

  modelsFile.replace(modelsFile.find("{{models}}"), 10, models);

  std::ofstream modelsFileOut(config["server_location"].get<std::string>() + "/models.hpp");

  if (!modelsFileOut.is_open()) {
    std::cerr << "Error opening file: " << config["server_location"].get<std::string>() + "/models.hpp" << std::endl;
    return;
  }

  modelsFileOut << modelsFile;
  modelsFileOut.close();
}
