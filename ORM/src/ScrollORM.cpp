#include "../include/ScrollORM.hpp"

const std::unordered_map<std::string, std::pair<std::string, std::string>>
    dataTypes = {{"String", {"std::string", "TEXT"}},
                 {"Int", {"int", "INTEGER"}},
                 {"Float", {"float", "REAL"}},
                 {"Bool", {"bool", "BOOLEAN"}},
                 {"Date", {"std::tm", "DATE"}},
                 {"DateTime", {"std::tm", "TIMESTAMP"}},
                 {"Time", {"std::tm", "TIME"}},
                 {"Binary", {"std::string", "BYTEA"}},
                 {"UUID", {"std::string", "UUID"}}};

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

      std::vector<std::pair<std::string, std::string>> tableFields;

      Select +=
          "static std::vector<" + modelName +
          "> SelectAll() {\n"
          "\t\tsoci::session* sql = Database::GetInstance()->GetSession();\n"
          "\t\tsoci::rowset<" +
          modelName + "> modelsRS = (sql->prepare << \"SELECT * FROM " +
          tableName +
          "\");\n"
          "\t\tstd::vector<" +
          modelName +
          "> models;\n"
          "\t\tstd::move(modelsRS.begin(), modelsRS.end(), "
          "std::back_inserter(models));\n"
          "\t\treturn models;\n"
          "\t}\n";

      Insert +=
          "\tstatic " + modelName + " Insert(const " + modelName + "& " +
          loweredModelName +
          ") {\n"
          "\t\tsoci::session* sql = Database::GetInstance()->GetSession();\n"
          "\t\t" +
          modelName +
          " model;\n"
          "\t\t*sql << \"INSERT INTO " +
          tableName + " (";

      for (auto [fieldName, fieldType] : modelFields) {
        std::string capitalizedFieldName = fieldName;
        capitalizedFieldName[0] = std::toupper(capitalizedFieldName[0]);

        std::pair<std::string, std::string> temp =
            std::make_pair(fieldName, dataTypes.at(fieldType).second);
        tableFields.push_back(temp);

        fromSetters += "\t\tp." + fieldName + " = v.get<" +
                       dataTypes.at(fieldType).first + ">(\"" + fieldName +
                       "\");\n";
        toSetters += "\t\tv.set(\"" + fieldName + "\", p." + fieldName + ");\n";
        fields +=
            "\t" + dataTypes.at(fieldType).first + " " + fieldName + ";\n";
        sociUpdateFields += fieldName + "=:" + fieldName + ", ";
        sociUpdateUses +=
            "soci::use(" + loweredModelName + "." + fieldName + "), ";
        sociInsertFields += ":" + fieldName + ", ";
        Insert += fieldName + ", ";

        Select +=
            "\tstatic std::vector<" + modelName + "> SelectBy" +
            capitalizedFieldName + "(const " + dataTypes.at(fieldType).first +
            "& " + fieldName +
            ") {\n"
            "\t\tsoci::session* sql = Database::GetInstance()->GetSession();\n"
            "\t\tsoci::rowset<" +
            modelName + "> modelsRS = (sql->prepare << \"SELECT * FROM " +
            tableName + " WHERE " + fieldName + " = :" + fieldName +
            "\", soci::use(" + fieldName +
            "));\n"
            "\t\tstd::vector<" +
            modelName +
            "> models;\n"
            "\t\tstd::move(modelsRS.begin(), modelsRS.end(), "
            "std::back_inserter(models));\n"
            "\t\treturn models;\n"
            "\t}\n";

        DeleteBy +=
            "\tstatic std::vector<" + modelName + "> DeleteBy" +
            capitalizedFieldName + "(const " + dataTypes.at(fieldType).first +
            "& " + fieldName +
            ") {\n"
            "\t\tsoci::session* sql = Database::GetInstance()->GetSession();\n"
            "\t\tstd::vector<" +
            modelName +
            "> models;\n"
            "\t\tsoci::rowset<" +
            modelName + "> modelRS = (sql->prepare << \"DELETE FROM " +
            tableName + " WHERE " + fieldName + " = :" + fieldName +
            " RETURNING *\", soci::use(" + fieldName +
            "));\n"
            "\t\tstd::move(modelRS.begin(), modelRS.end(), "
            "std::back_inserter(models));\n"
            "\t\treturn models;\n"
            "\t}\n";
      }

      Insert.replace(Insert.find_last_of(", ") - 1, 2,
                     ") VALUES (" +
                         sociInsertFields.replace(
                             sociInsertFields.find_last_of(", ") - 1, 2,
                             ") RETURNING * \", soci::use(" + loweredModelName +
                                 "), soci::into(model);\n"));

      sociUpdateFields.replace(sociUpdateFields.find_last_of(", ") - 1, 2, " ");

      for (auto [fieldName, fieldType] : modelFields) {
        std::string capitalizedFieldName = fieldName;
        capitalizedFieldName[0] = std::toupper(capitalizedFieldName[0]);

        UpdateBy +=
            "\tstatic std::vector<" + modelName + "> UpdateBy" +
            capitalizedFieldName + "(const " + dataTypes.at(fieldType).first +
            "& " + fieldName + ", const " + modelName + "& " +
            loweredModelName +
            ") {"
            "\t\tsoci::session* sql = Database::GetInstance()->GetSession();\n"
            "\t\tstd::vector<" +
            modelName +
            "> models;\n"
            "\t\tsoci::rowset<" +
            modelName + "> modelRS = (sql->prepare << \"UPDATE " + tableName +
            " SET " + sociUpdateFields + "WHERE " + fieldName +
            "=:" + capitalizedFieldName + "_" + " RETURNING *\", " +
            sociUpdateUses + "soci::use(" + fieldName +
            "));\n"
            "\t\tstd::move(modelRS.begin(), modelRS.end(), "
            "std::back_inserter(models));\n"
            "\t\treturn models;\n"
            "\t}\n";
      }

      Insert += "\t\treturn model;\n"
                "\t}\n";

      ORMMethods += Select + Insert + DeleteBy + UpdateBy;

      tables.push_back(std::make_pair(tableName, tableFields));

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

  std::ofstream modelsFileOut(config["server_location"].get<std::string>() +
                              "/models.hpp");

  if (!modelsFileOut.is_open()) {
    std::cerr << "Error opening file: "
              << config["server_location"].get<std::string>() + "/models.hpp"
              << std::endl;
    return;
  }

  modelsFileOut << modelsFile;
  modelsFileOut.close();
}

std::string checkConfig(json& config) {
  if (!config.contains("database")) {
    return "\033[1;31m[-] Database configuration not found\033[0m";
  }

  if (!config["database"].contains("models_location")) {
    return "\033[1;31m[-] Models location not found\033[0m";
  }

  if (!config["database"].contains("provider")) {
    return "\033[1;31m[-] Database provider not found\033[0m";
  }

  if (!config["database"].contains("host")) {
    return "\033[1;31m[-] Database host not found\033[0m";
  }

  if (!config["database"].contains("port")) {
    return "\033[1;31m[-] Database port not found\033[0m";
  }

  if (!config["database"].contains("database")) {
    return "\033[1;31m[-] Database name not found\033[0m";
  }

  if (!config["database"].contains("user")) {
    return "\033[1;31m[-] Database user not found\033[0m";
  }

  if (!config["database"].contains("password")) {
    return "\033[1;31m[-] Database password not found\033[0m";
  }

  return "";
}

json reflectModels(std::vector<std::string> modelFiles, std::string modelsLocation) {
  json models;
  for (auto modelFile : modelFiles) {
    std::string path = modelsLocation + "/" + modelFile;
    std::ifstream file(path);
    std::string line;

    if (!file.is_open()) {
      std::cerr << "Error opening file: " << path << std::endl;
      return json();
    }

    unsigned int lineCount = 0;
    int inModel = -1;
    std::string currentModel = "";

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
              return json();
            }
          }

          if (modelName.empty() || modelName == "model") {
            std::cerr << "Invalid or empty model name at " << path << ":"
                      << lineCount << std::endl;
            return json();
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
            return json();
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
              return json();
            }
          }

          if (tableName.empty() || tableName == ":" || tableName == "model") {
            std::cerr << "Invalid or empty table name at " << path << ":"
                      << lineCount << std::endl;
            return json();
          }

          currentModel = modelName;  
          models[modelName]["table"] = tableName;
          inModel = lineCount;
        }
      }

      if (inModel != -1 && lineCount > inModel) {
        if (line.find("}") != std::string::npos) {
          inModel = -1;
        } 
        else {
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
            return json();
          } else if (fieldName.empty() && foundColon) {
            std::cerr << "Expected field name before colon at " << path << ":"
                      << lineCount << std::endl;
            return json();
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
            return json();
          } else if (!fieldName.empty() && !fieldType.empty() && foundColon) {
            models[currentModel]["fields"][fieldName]["type"] = fieldType;
          }

          // model : table {
          //  field : type @attribute value @attribute value
          // }
          size_t attributeIndex;
          while ((attributeIndex = line.find("@", attributeIndex)) != std::string::npos) {
            std::string attribute = "";
            std::string value = "";
            
            for (auto token : line.substr(attributeIndex)) {
              if (token == '@') {
                continue;
              }
              else if (std::isspace(token) && attribute.empty()) {
                continue;
              }
              else if (std::isspace(token) && !attribute.empty()) {
                break;
              }
              else {
                attribute += token;
              }
            }

            for (auto token : line.substr(attributeIndex + attribute.size())) {
              if (token == '@') {
                continue;
              }
              else if (std::isspace(token) && value.empty()) {
                continue;
              }
              else if (std::isspace(token) && !value.empty()) {
                break;
              }
              else {
                value += token;
              }
            }
            
            // Current implementation supports default, pk, unique, nullable,
            // TODO: add support for fk, check, index, autoincrement

            if (attribute.empty()) {
              std::cerr << "Invalid attribute at " << path << ":" << lineCount
                        << std::endl;
              return json();
            }
            if (value.empty()) {
              if (attribute == "default") {
                std::cerr << "Expected value for default attribute at " << path
                          << ":" << lineCount << std::endl;
                return json();
              }
            }

            models[currentModel]["fields"][fieldName]["attributes"][attribute] = value;
          }
        }
      }
    }
  }

  return models;
}

void migrateDB(json config) {
  std::string configCheck = checkConfig(config);

  if (!configCheck.empty()) {
    std::cerr << configCheck << std::endl;
    return;
  }

  std::string modelsLocation =
      config["database"]["models_location"].get<std::string>();

  std::vector<std::string> modelFiles;

  for (const auto& entry : std::filesystem::directory_iterator(modelsLocation)) {
    if (entry.path().extension() == ".scrl") {
      modelFiles.push_back(entry.path().filename());
    }
  }

  std::string provider = config["database"]["provider"].get<std::string>();
  std::string host = config["database"]["host"].get<std::string>();
  std::string port = std::to_string(config["database"]["port"].get<int>());
  std::string database = config["database"]["database"].get<std::string>();
  std::string user = config["database"]["user"].get<std::string>();
  std::string password = config["database"]["password"].get<std::string>();

  if (provider != "postgresql") {
    std::cerr << "\033[1;31m[-] Database provider not supported\033[0m"
              << std::endl;
    return;
  }

  std::string connectionString = "dbname=" + database + " user=" + user +
                                 " password=" + password + " host=" + host +
                                 " port=" + port;

  /*
   *[
   *  "<model name>": {
   *    "table": "<table name>",
   *    "fields": [
   *      "<field name>": {
   *        "type": "<field type>",
   *        "attributes": [
   *          "<attribute name>": "<attribute value>"
   *        ]
   *      },
   *      ...
   *    ]
   *  },
   *  ...
   *]
  */
  json models = reflectModels(modelFiles, modelsLocation);

  if (models.empty()) {
    return;
  }

  std::string migrationFile =
      config["database"]["models_location"].get<std::string>() +
      "/migrations.sql";
  std::ofstream migrationFileOut(migrationFile);

  if (!migrationFileOut.is_open()) {
    std::cerr << "Error opening file: " << migrationFile << std::endl;
    return;
  }

  soci::session sql;
  sql.open(soci::postgresql, connectionString);

  std::vector<std::string> tables(100);
 
  sql.get_table_names(), soci::into(tables);

  for (auto& [modelName, model] : models.items()) {
    std::string tableName = model["table"].get<std::string>();    
    std::cout << "Migrating table: " << tableName << std::endl;

    std::transform(tableName.begin(), tableName.end(), tableName.begin(), ::tolower);

    std::string migration = "";
    
    if (std::find(tables.begin(), tables.end(), tableName) == tables.end()) {
      migration = "CREATE TABLE " + tableName + " (\n";

      for (auto [fieldName, field] : model["fields"].items()) {
        std::string fieldType = field["type"].get<std::string>();
        migration +=
            "\t" + fieldName + " " + dataTypes.at(fieldType).second + ",\n";
      }

      migration.replace(migration.find_last_of(","), 1, "\n);\n");
    }
    else {
      // check if the table has all the fields
      // if not, add the missing fields
      // if the table has extra fields, remove them

      soci::rowset<soci::row> tableFieldsRS = (sql.prepare << "SELECT column_name, data_type FROM information_schema.columns WHERE table_name = :table_name", soci::use(tableName));

      std::unordered_map<std::string, std::string> tableFields;

      for (soci::rowset<soci::row>::const_iterator field = tableFieldsRS.begin(); field != tableFieldsRS.end(); ++field) {
        tableFields[field->get<std::string>(0)] = field->get<std::string>(1);
      }

      std::vector<std::pair<std::string, std::string>> missingFields;
      std::vector<std::pair<std::string, std::string>> extraFields;

      // TODO: fields whose types have changed should be updated
      // TODO: fields whose attributes have changed should be updated
      for (auto [fieldName, field] : model["fields"].items()) {
        std::string fieldType = field["type"].get<std::string>();
        if (!tableFields.contains(fieldName)) {
          missingFields.push_back(std::make_pair(fieldName, fieldType));
        }
      }

      for (auto [fieldName, fieldType] : tableFields) {
        if (!model["fields"].contains(fieldName)) {
          extraFields.push_back(std::make_pair(fieldName, fieldType));
        }
      }
      
      if (!extraFields.empty()) {
        migration += "ALTER TABLE " + tableName;
        
        for (auto [fieldName, fieldType] : extraFields) {
          migration += " DROP COLUMN " + fieldName + ", ";
        }
        
        migration.replace(migration.find_last_of(","), 2, ";\n");
      }

      if (!missingFields.empty()) {
        migration += "ALTER TABLE " + tableName;
        
        for (auto [fieldName, fieldType] : missingFields) {
          migration += " ADD COLUMN " + fieldName + " " + dataTypes.at(fieldType).second + ", ";
        }
        
        migration.replace(migration.find_last_of(","), 2, ";\n");
      }
    }

    migrationFileOut << migration;
  }

  migrationFileOut.close();
}
