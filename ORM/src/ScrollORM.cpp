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

const std::unordered_map<std::string, std::string> attributeConversions = {
    {"default", "DEFAULT"},
    {"pk", "PRIMARY KEY"},
    {"unique", "UNIQUE"},
    {"nullable", "NOT NULL"},
    {"fk", "FOREIGN KEY REFERENCES {{table}}({{field}})"},
    {"check", "CHECK ({{condition}})"},
    {"index", "INDEX"},
    {"autoincrement", "AUTOINCREMENT"}};

const std::unordered_map<std::string, std::string> keyOnlyAttributes = {
    {"pk", "PRIMARY KEY"},
    {"unique", "UNIQUE"},
    {"nullable", "NOT NULL"},
    {"autoincrement", "AUTOINCREMENT"}};

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

void findAndReplace(std::string &str, const std::string &toReplace,
                           const std::string &replaceWith) {
  size_t pos = 0;
  while ((pos = str.find(toReplace, pos)) != std::string::npos) {
    str.replace(pos, toReplace.length(), replaceWith);
  }
}

json reflectModels(std::vector<std::string> modelFiles,
                   std::string modelsLocation) {
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
        } else {
          std::string fieldName = "";
          std::string fieldType = "";

          bool foundColon = false;
          int pos = -1;

          if (line.size() == 0) {
            continue;
          }

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
          } else if (!fieldType.empty() && !dataTypes.contains(fieldType) &&
                     !models.contains(fieldType)) {
            std::cerr << "Invalid field type '" << fieldType << "' at " << path
                      << ":" << lineCount << std::endl;
            return json();
          } else if (!fieldName.empty() && !fieldType.empty() && foundColon) {
            models[currentModel]["fields"][fieldName]["type"] = fieldType;
          }

          // model : table {
          //  field : type @attribute value @attribute value
          // }
          //
          // Current implementation supports default, pk, unique, nullable, fk,
          // TODO: add support for check, index
          // TODO: add table-wide attributes (constraints, indexes, etc.)
          while ((pos = line.find('@', pos)) != std::string::npos) {
            std::string attribute = "";
            std::string value = "";
            std::vector<std::string> tupleValues;

            // attribute
            for (auto token : line.substr(pos)) {
              ++pos;
              if (token == '@' && attribute.empty()) {
                continue;
              } else if (std::isspace(token) && attribute.empty()) {
                continue;
              } else if (std::isspace(token) && !attribute.empty()) {
                break;
              } else {
                attribute += token;
              }
            }

            bool inString = false;
            bool inTuple = false;

            // value
            for (auto token : line.substr(pos, line.find('@', pos) - pos)) {
              ++pos;
              if (token == '@' && !keyOnlyAttributes.contains(attribute)) {
                std::cerr << "Unexpected '@' instead of value for '"
                          << attribute << "' at " << path << ":" << lineCount
                          << std::endl;
                return json();
              } else if (token == '@' &&
                         keyOnlyAttributes.contains(attribute)) {
                break;
              } else if (token == '\'' && !inString) {
                inString = true;
                value += token;
                continue;
              } else if (token == '\'' && inString) {
                inString = false;
                value += token;
                continue;
              } else if (token == '(' && !inTuple) {
                inTuple = true;
                continue;
              } else if (token == ')' && inTuple) {
                inTuple = false;
                tupleValues.push_back(value);
                break;
              } else if (std::isspace(token) && value.empty()) {
                continue;
              } else if (std::isspace(token) && !value.empty() && !inString &&
                         !inTuple) {
                break;
              } else if (token == ',' && !value.empty() && inTuple) {
                std::cout << "Tuple value: " << value << std::endl;
                tupleValues.push_back(value);
                value = "";
                continue;
              } else {
                value += token;
              }
            }

            if (!attribute.empty() &&
                !attributeConversions.contains(attribute)) {
              std::cerr << "Invalid attribute '" << attribute << "' at " << path
                        << ":" << lineCount << std::endl;
              return json();
            }

            if (value.empty()) {
              if (attribute == "default") {
                std::cerr << "Expected value for '" << attribute << "' at "
                          << path << ":" << lineCount << std::endl;
                return json();
              }
            }

            if (!attribute.empty()) {
              if (!tupleValues.empty()) {
                std::cout << tupleValues.size() << std::endl;
                for (auto tupleValue : tupleValues) {
                  models[currentModel]["fields"][fieldName]["attributes"]
                        [attribute]
                            .push_back(tupleValue);
                }
              } else {
                models[currentModel]["fields"][fieldName]["attributes"]
                      [attribute] = value;
              }
            }
          }
        }
      }
    }
  }

  return models;
}

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

  for (const auto &entry :
       std::filesystem::directory_iterator(modelsLocation)) {
    if (entry.path().extension() == ".scrl") {
      modelFiles.push_back(entry.path().filename());
    }
  }
  
  json modelsJson = reflectModels(modelFiles, modelsLocation);

  /*
   *{
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
   *}
  */
  for (auto &[modelName, model] : modelsJson.items()) {
    std::string tableName = model["table"].get<std::string>();
    std::string loweredModelName = modelName;
    loweredModelName[0] = std::tolower(loweredModelName[0]);

    std::string conversion = conversionTemplate;
    std::string modelStr = classTemplate;
    std::string fields;
    std::string selects;
    std::string deletes;
    std::string updates;
    std::string insert;
    std::string selectByTemplate;
    std::string updateByTemplate;
    std::string deleteByTemplate;
    std::string ORMMethods;
    std::string sociUpdateFields;
    std::string sociUpdateUses;
    std::string sociInsertFields;
    std::string sociInsertIntos;
    std::string fromSetters;
    std::string toSetters;
    std::string fromSetterTemplate;
    std::string toSetterTemplate;

    std::vector<std::pair<std::string, std::string>> tableFields;

    fromSetterTemplate = "\t\tp.{{field name}} = v.get<{{field type}}>(\"{{field name}}\");\n";
    toSetterTemplate = "\t\tv.set(\"{{field name}}\", p.{{field name}});\n";

    selects = R"(
static std::vector<{{model name}}> SelectAll() {
  soci::session* sql = Database::GetInstance()->GetSession();
  soci::rowset<{{model name}}> modelsRS = (sql->prepare << "SELECT * FROM {{table name}}");
  std::vector<{{model name}}> models;
  std::move(modelsRS.begin(), modelsRS.end(), std::back_inserter(models));
  return models;
})";
    selectByTemplate = R"(
static std::vector<{{model name}}> SelectBy{{Field name}}(const {{field type}}& {{field name}}) { 
  soci::session* sql = Database::GetInstance()->GetSession();
  soci::rowset<{{model name}}> modelsRS = (sql->prepare << "SELECT * FROM {{table name}} WHERE {{field name}} = :{{field name}}", soci::use({{field name}}));
  std::vector<{{model name}}> models;
  std::move(modelsRS.begin(), modelsRS.end(), std::back_inserter(models));
  return models;
})";

    insert = R"(
static {{model name}} Insert(const {{model name}}& {{lowered model name}}) {
  soci::session* sql = Database::GetInstance()->GetSession();
  {{model name}} model;
  *sql << "INSERT INTO {{table name}} ({{fields}}) VALUES ({{values}}) RETURNING * ", soci::use({{lowered model name}}), soci::into(model);
  return model;
})";

    deleteByTemplate = R"(
static std::vector<{{model name}}> DeleteBy{{Field name}}(const {{field type}}& {{field name}}) {
  soci::session* sql = Database::GetInstance()->GetSession();
  std::vector<{{model name}}> models;
  soci::rowset<{{model name}}> modelRS = (sql->prepare << "DELETE FROM {{table name}} WHERE {{field name}} = :{{field name}} RETURNING *", soci::use({{field name}}));
  std::move(modelRS.begin(), modelRS.end(), std::back_inserter(models));
  return models;
})";

    updateByTemplate = R"(
static std::vector<{{model name}}> UpdateBy{{Field name}}(const {{field type}}& {{field name}}, const {{model name}}& {{lowered model name}}) {
  soci::session* sql = Database::GetInstance()->GetSession();
  std::vector<{{model name}}> models;
  soci::rowset<{{model name}}> modelRS = (sql->prepare << "UPDATE {{table name}} SET {{fields}} WHERE {{field name}}=:{{field name}} RETURNING *", {{soci uses}}, soci::use({{field name}}));
  std::move(modelRS.begin(), modelRS.end(), std::back_inserter(models));
  return models;
})";

    findAndReplace(selects, "{{model name}}", modelName);
    findAndReplace(selects, "{{table name}}", tableName);

    findAndReplace(insert, "{{model name}}", modelName);
    findAndReplace(insert, "{{lowered model name}}", loweredModelName);
    findAndReplace(insert, "{{table name}}", tableName);

    findAndReplace(selectByTemplate, "{{model name}}", modelName);
    findAndReplace(selectByTemplate, "{{table name}}", tableName);

    findAndReplace(deleteByTemplate, "{{model name}}", modelName);
    findAndReplace(deleteByTemplate, "{{table name}}", tableName);

    findAndReplace(updateByTemplate, "{{model name}}", modelName);
    findAndReplace(updateByTemplate, "{{table name}}", tableName);
    findAndReplace(updateByTemplate, "{{lowered model name}}", loweredModelName);
    
    for (auto [fieldName, field] : model["fields"].items()) {
      std::string fieldType = field["type"].get<std::string>();
      fields +=
          "  " + dataTypes.at(fieldType).first + " " + fieldName + ";\n";

      sociUpdateFields += fieldName + " = :" + fieldName + ", ";
      sociUpdateUses += "soci::use(" + loweredModelName + "." + fieldName +
                        "), ";

      sociInsertIntos += ":" + fieldName + ", ";
      sociInsertFields += fieldName + ", ";

      std::string fromSetter = fromSetterTemplate;
      std::string toSetter = toSetterTemplate;

      findAndReplace(fromSetter, "{{field name}}", fieldName);
      findAndReplace(fromSetter, "{{field type}}", dataTypes.at(fieldType).first);
      findAndReplace(toSetter, "{{field name}}", fieldName);

      fromSetters += fromSetter; 
      toSetters += toSetter;
    }

    sociInsertFields.replace(sociInsertFields.find_last_of(","), 2, "");
    sociInsertIntos.replace(sociInsertIntos.find_last_of(","), 2, "");
    sociUpdateFields.replace(sociUpdateFields.find_last_of(","), 2, "");
    sociUpdateUses.replace(sociUpdateUses.find_last_of(","), 2, "");

    for (auto [fieldName, field] : model["fields"].items()) {
      std::string fieldType = field["type"].get<std::string>();
      std::string capitalizedFieldName = fieldName;
      capitalizedFieldName[0] = std::toupper(capitalizedFieldName[0]);

      std::string selectBy = selectByTemplate;
      std::string updateBy = updateByTemplate;
      std::string deleteBy = deleteByTemplate;
      
      std::pair<std::string, std::string> temp =
          std::make_pair(fieldName, dataTypes.at(fieldType).second);
      tableFields.push_back(temp);

      findAndReplace(selectBy, "{{Field name}}", capitalizedFieldName);
      findAndReplace(selectBy, "{{field name}}", fieldName);
      findAndReplace(selectBy, "{{field type}}", dataTypes.at(fieldType).first);

      findAndReplace(updateBy, "{{Field name}}", capitalizedFieldName);
      findAndReplace(updateBy, "{{field name}}", fieldName);
      findAndReplace(updateBy, "{{field type}}", dataTypes.at(fieldType).first);
      findAndReplace(updateBy, "{{fields}}", sociUpdateFields);
      findAndReplace(updateBy, "{{soci uses}}", sociUpdateUses);

      findAndReplace(deleteBy, "{{Field name}}", capitalizedFieldName);
      findAndReplace(deleteBy, "{{field name}}", fieldName);
      findAndReplace(deleteBy, "{{field type}}", dataTypes.at(fieldType).first);

      selects += selectBy;
      deletes += deleteBy;
      updates += updateBy;
    }
    
    findAndReplace(insert, "{{fields}}", sociInsertFields);
    findAndReplace(insert, "{{values}}", sociInsertIntos);
    sociUpdateFields.replace(sociUpdateFields.find_last_of(", ") - 1, 2, " ");

    ORMMethods += selects + insert + deletes + updates;

    tables.push_back(std::make_pair(tableName, tableFields));

    findAndReplace(modelStr, "{{model name}}", modelName);
    findAndReplace(modelStr, "{{fields}}", fields);
    findAndReplace(modelStr, "{{ORM methods}}", ORMMethods);

    findAndReplace(conversion, "{{model name}}", modelName);
    findAndReplace(conversion, "{{from setters}}", fromSetters);
    findAndReplace(conversion, "{{to setters}}", toSetters);

    models += modelStr;
    models += "\n";
    models += conversion;
    std::cout << "success 123" << std::endl;
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

std::string checkConfig(json &config) {
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


void migrateDB(json config, std::string migrationName) {
  std::string configCheck = checkConfig(config);

  if (!configCheck.empty()) {
    std::cerr << configCheck << std::endl;
    return;
  }

  std::string modelsLocation =
      config["database"]["models_location"].get<std::string>();

  std::vector<std::string> modelFiles;

  for (const auto &entry :
       std::filesystem::directory_iterator(modelsLocation)) {
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
   *{
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
   *}
   */
  json models = reflectModels(modelFiles, modelsLocation);

  if (models.empty()) {
    return;
  }

  std::string migrationPrefix = "/migration_";

  if (migrationName == "") {
    migrationPrefix.pop_back();
  }

  std::cout << config["database"]["models_location"].get<std::string>() +
                   migrationPrefix + migrationName + ".sql"
            << std::endl;

  std::string migrationFile =
      config["database"]["models_location"].get<std::string>() +
      migrationPrefix + migrationName + ".sql";
  std::ofstream migrationFileOut(migrationFile);

  if (!migrationFileOut.is_open()) {
    std::cerr << "Error opening file: " << migrationFile << std::endl;
    return;
  }

  soci::session sql;
  sql.open(soci::postgresql, connectionString);

  std::vector<std::string> tables(100);

  sql.get_table_names(), soci::into(tables);

  for (auto &[modelName, model] : models.items()) {
    std::string tableName = model["table"].get<std::string>();
    std::string loweredTableName = tableName;
    std::cout << "Migrating table: " << tableName << std::endl;

    std::cout << model.dump(4) << std::endl;
    std::transform(loweredTableName.begin(), loweredTableName.end(),
                   loweredTableName.begin(), ::tolower);

    std::string migration = "";

    if (std::find(tables.begin(), tables.end(), loweredTableName) ==
        tables.end()) {
      migration = "CREATE TABLE " + tableName + " (\n";

      for (auto [fieldName, field] : model["fields"].items()) {
        std::string fieldType = field["type"].get<std::string>();

        json attributes = field["attributes"];
        std::string attributesStr = "";

        if (!attributes.empty()) {
          for (auto [attribute, value] : attributes.items()) {
            if (attribute == "fk") {
              if (value.type() != json::value_t::array) {
                std::cerr << "Expected object for 'fk' attribute at "
                          << modelName << ":" << fieldName << std::endl;
                sql.close();
                return;
              }

              if (value.size() != 2) {
                std::cerr
                    << "Expected 2 values inside tuple for 'fk' attribute at "
                    << modelName << ":" << fieldName << std::endl;
                sql.close();
                return;
              }

              std::string fkTable = value[0].get<std::string>();
              std::string fkField = value[1].get<std::string>();
              std::string fk = attributeConversions.at(attribute);
              fk.replace(fk.find("{{table}}"), 9, fkTable);
              fk.replace(fk.find("{{field}}"), 9, fkField);

              attributesStr += fk + " ";
            } else if (value.get<std::string>().empty()) {
              attributesStr += attributeConversions.at(attribute) + " ";
            } else {
              attributesStr += attributeConversions.at(attribute) + " " +
                               value.get<std::string>() + " ";
            }
          }
          attributesStr.pop_back();
        }

        if (dataTypes.contains(fieldType)) {
          migration += "\t" + fieldName + " " + dataTypes.at(fieldType).second +
                       " " + attributesStr + ",\n";
        }
      }

      if (migration.find_last_of(",") != std::string::npos) {
        migration.replace(migration.find_last_of(","), 1, "\n);\n");
      }
    } else {
      // check if the table has all the fields
      // if not, add the missing fields
      // if the table has extra fields, remove them

      soci::rowset<soci::row> tableFieldsRS =
          (sql.prepare
               << "SELECT column_name, data_type FROM "
                  "information_schema.columns WHERE table_name = :table_name",
           soci::use(loweredTableName));

      std::unordered_map<std::string, std::string> tableFields;

      for (soci::rowset<soci::row>::const_iterator field =
               tableFieldsRS.begin();
           field != tableFieldsRS.end(); ++field) {
        tableFields[field->get<std::string>(0)] = field->get<std::string>(1);
      }

      std::vector<std::pair<std::string, std::string>> missingFields;
      std::vector<std::pair<std::string, std::string>> extraFields;

      // TODO: fields whose attributes have changed should be updated
      for (auto [fieldName, field] : model["fields"].items()) {
        std::string fieldType = field["type"].get<std::string>();
        if (!dataTypes.contains(fieldType) && models.contains(fieldType)) {
          continue;
        }
        if (!tableFields.contains(fieldName)) {
          missingFields.push_back(std::make_pair(fieldName, fieldType));
        } else {
          std::string tableFieldType = tableFields[fieldName];
          std::transform(tableFieldType.begin(), tableFieldType.end(),
                         tableFieldType.begin(), ::toupper);

          if (tableFieldType != dataTypes.at(fieldType).second) {
            missingFields.push_back(std::make_pair(fieldName, fieldType));
            extraFields.push_back(
                std::make_pair(fieldName, tableFields[fieldName]));
          }
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
          migration += " DROP COLUMN " + fieldName + ",";
        }

        migration.replace(migration.find_last_of(","), 1, ";\n");
      }

      if (!missingFields.empty()) {
        migration += "ALTER TABLE " + tableName;

        for (auto [fieldName, fieldType] : missingFields) {
          std::string attributes = "";
          if (model["fields"][fieldName].contains("attributes")) {
            for (auto [attribute, value] :
                 model["fields"][fieldName]["attributes"].items()) {
              attributes += attributeConversions.at(attribute) + " " +
                            value.get<std::string>() + " ";
              attributes.pop_back();
            }
          }
          migration += " ADD COLUMN " + fieldName + " " +
                       dataTypes.at(fieldType).second + attributes + ",";
        }

        migration.replace(migration.find_last_of(","), 1, ";\n");
      }
    }

    migrationFileOut << migration;
  }

  migrationFileOut.close();
}
