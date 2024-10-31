
#include <string>
#include <vector>
#include <ctime>
#include "Database.hpp"


class User {
public:
  User() = default;
  ~User() = default;

	std::string name;
	int age;


  static std::vector<User> SelectAll() {
		soci::session* sql = Database::GetInstance()->GetSession();
		soci::rowset<User> modelsRS = (sql->prepare << "SELECT * FROM Users");
		std::vector<User> models;
		std::move(modelsRS.begin(), modelsRS.end(), std::back_inserter(models));
		return models;
	}
	static std::vector<User> SelectByName(const std::string& name) {
		soci::session* sql = Database::GetInstance()->GetSession();
		soci::rowset<User> modelsRS = (sql->prepare << "SELECT * FROM Users WHERE name = :name", soci::use(name));
		std::vector<User> models;
		std::move(modelsRS.begin(), modelsRS.end(), std::back_inserter(models));
		return models;
	}
	static std::vector<User> SelectByAge(const int& age) {
		soci::session* sql = Database::GetInstance()->GetSession();
		soci::rowset<User> modelsRS = (sql->prepare << "SELECT * FROM Users WHERE age = :age", soci::use(age));
		std::vector<User> models;
		std::move(modelsRS.begin(), modelsRS.end(), std::back_inserter(models));
		return models;
	}
	static User Insert(const User& user) {
		soci::session* sql = Database::GetInstance()->GetSession();
		User model;
		*sql << "INSERT INTO Users (name, age) VALUES (:name, :age) RETURNING * ", soci::use(user), soci::into(model);
		return model;
	}
	static std::vector<User> DeleteByName(const std::string& name) {
		soci::session* sql = Database::GetInstance()->GetSession();
		std::vector<User> models;
		soci::rowset<User> modelRS = (sql->prepare << "DELETE FROM Users WHERE name = :name RETURNING *", soci::use(name));
		std::move(modelRS.begin(), modelRS.end(), std::back_inserter(models));
		return models;
	}
	static std::vector<User> DeleteByAge(const int& age) {
		soci::session* sql = Database::GetInstance()->GetSession();
		std::vector<User> models;
		soci::rowset<User> modelRS = (sql->prepare << "DELETE FROM Users WHERE age = :age RETURNING *", soci::use(age));
		std::move(modelRS.begin(), modelRS.end(), std::back_inserter(models));
		return models;
	}
	static std::vector<User> UpdateByName(const std::string& name, const User& user) {		soci::session* sql = Database::GetInstance()->GetSession();
		std::vector<User> models;
		soci::rowset<User> modelRS = (sql->prepare << "UPDATE Users SET name=:name, age=:age WHERE name=:Name_ RETURNING *", soci::use(user.name), soci::use(user.age), soci::use(name));
		std::move(modelRS.begin(), modelRS.end(), std::back_inserter(models));
		return models;
	}
	static std::vector<User> UpdateByAge(const int& age, const User& user) {		soci::session* sql = Database::GetInstance()->GetSession();
		std::vector<User> models;
		soci::rowset<User> modelRS = (sql->prepare << "UPDATE Users SET name=:name, age=:age WHERE age=:Age_ RETURNING *", soci::use(user.name), soci::use(user.age), soci::use(age));
		std::move(modelRS.begin(), modelRS.end(), std::back_inserter(models));
		return models;
	}

};


template<>
struct soci::type_conversion<User> {
  typedef soci::values base_type;

  static void from_base(soci::values const& v, soci::indicator ind, User& p) {
		p.name = v.get<std::string>("name");
		p.age = v.get<int>("age");

  }

  static void to_base(const User& p, soci::values& v, soci::indicator& ind) {
		v.set("name", p.name);
		v.set("age", p.age);

    ind = soci::i_ok;
  }
};

