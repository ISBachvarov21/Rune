
#include <string>
#include <vector>
#include <ctime>
#include "Database.hpp"


class Post {
public:
  Post() = default;
  ~Post() = default;

  std::tm date;
  std::string description;
  std::string title;


  
static std::vector<Post> SelectAll() {
  soci::session* sql = Database::GetInstance()->GetSession();
  soci::rowset<Post> modelsRS = (sql->prepare << "SELECT * FROM Posts");
  std::vector<Post> models;
  std::move(modelsRS.begin(), modelsRS.end(), std::back_inserter(models));
  return models;
}
static std::vector<Post> SelectByDate(const std::tm& date) { 
  soci::session* sql = Database::GetInstance()->GetSession();
  soci::rowset<Post> modelsRS = (sql->prepare << "SELECT * FROM Posts WHERE date = :date", soci::use(date));
  std::vector<Post> models;
  std::move(modelsRS.begin(), modelsRS.end(), std::back_inserter(models));
  return models;
}
static std::vector<Post> SelectByDescription(const std::string& description) { 
  soci::session* sql = Database::GetInstance()->GetSession();
  soci::rowset<Post> modelsRS = (sql->prepare << "SELECT * FROM Posts WHERE description = :description", soci::use(description));
  std::vector<Post> models;
  std::move(modelsRS.begin(), modelsRS.end(), std::back_inserter(models));
  return models;
}
static std::vector<Post> SelectByTitle(const std::string& title) { 
  soci::session* sql = Database::GetInstance()->GetSession();
  soci::rowset<Post> modelsRS = (sql->prepare << "SELECT * FROM Posts WHERE title = :title", soci::use(title));
  std::vector<Post> models;
  std::move(modelsRS.begin(), modelsRS.end(), std::back_inserter(models));
  return models;
}
static Post Insert(const Post& post) {
  soci::session* sql = Database::GetInstance()->GetSession();
  Post model;
  *sql << "INSERT INTO Posts (date, description, title) VALUES (:date, :description, :title) RETURNING * ", soci::use(post), soci::into(model);
  return model;
}
static std::vector<Post> DeleteByDate(const std::tm& date) {
  soci::session* sql = Database::GetInstance()->GetSession();
  std::vector<Post> models;
  soci::rowset<Post> modelRS = (sql->prepare << "DELETE FROM Posts WHERE date = :date RETURNING *", soci::use(date));
  std::move(modelRS.begin(), modelRS.end(), std::back_inserter(models));
  return models;
}
static std::vector<Post> DeleteByDescription(const std::string& description) {
  soci::session* sql = Database::GetInstance()->GetSession();
  std::vector<Post> models;
  soci::rowset<Post> modelRS = (sql->prepare << "DELETE FROM Posts WHERE description = :description RETURNING *", soci::use(description));
  std::move(modelRS.begin(), modelRS.end(), std::back_inserter(models));
  return models;
}
static std::vector<Post> DeleteByTitle(const std::string& title) {
  soci::session* sql = Database::GetInstance()->GetSession();
  std::vector<Post> models;
  soci::rowset<Post> modelRS = (sql->prepare << "DELETE FROM Posts WHERE title = :title RETURNING *", soci::use(title));
  std::move(modelRS.begin(), modelRS.end(), std::back_inserter(models));
  return models;
}
static std::vector<Post> UpdateByDate(const std::tm& date, const Post& post) {
  soci::session* sql = Database::GetInstance()->GetSession();
  std::vector<Post> models;
  soci::rowset<Post> modelRS = (sql->prepare << "UPDATE Posts SET date = :date, description = :description, title = :title WHERE date=:date RETURNING *", soci::use(post.date), soci::use(post.description), soci::use(post.title), soci::use(date));
  std::move(modelRS.begin(), modelRS.end(), std::back_inserter(models));
  return models;
}
static std::vector<Post> UpdateByDescription(const std::string& description, const Post& post) {
  soci::session* sql = Database::GetInstance()->GetSession();
  std::vector<Post> models;
  soci::rowset<Post> modelRS = (sql->prepare << "UPDATE Posts SET date = :date, description = :description, title = :title WHERE description=:description RETURNING *", soci::use(post.date), soci::use(post.description), soci::use(post.title), soci::use(description));
  std::move(modelRS.begin(), modelRS.end(), std::back_inserter(models));
  return models;
}
static std::vector<Post> UpdateByTitle(const std::string& title, const Post& post) {
  soci::session* sql = Database::GetInstance()->GetSession();
  std::vector<Post> models;
  soci::rowset<Post> modelRS = (sql->prepare << "UPDATE Posts SET date = :date, description = :description, title = :title WHERE title=:title RETURNING *", soci::use(post.date), soci::use(post.description), soci::use(post.title), soci::use(title));
  std::move(modelRS.begin(), modelRS.end(), std::back_inserter(models));
  return models;
}
};


template<>
struct soci::type_conversion<Post> {
  typedef soci::values base_type;

  static void from_base(soci::values const& v, soci::indicator ind, Post& p) {
		p.date = v.get<std::tm>("date");
		p.description = v.get<std::string>("description");
		p.title = v.get<std::string>("title");

  }

  static void to_base(const Post& p, soci::values& v, soci::indicator& ind) {
		v.set("date", p.date);
		v.set("description", p.description);
		v.set("title", p.title);

    ind = soci::i_ok;
  }
};

class User {
public:
  User() = default;
  ~User() = default;

  int age;
  std::string name;


  
static std::vector<User> SelectAll() {
  soci::session* sql = Database::GetInstance()->GetSession();
  soci::rowset<User> modelsRS = (sql->prepare << "SELECT * FROM Users");
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
static std::vector<User> SelectByName(const std::string& name) { 
  soci::session* sql = Database::GetInstance()->GetSession();
  soci::rowset<User> modelsRS = (sql->prepare << "SELECT * FROM Users WHERE name = :name", soci::use(name));
  std::vector<User> models;
  std::move(modelsRS.begin(), modelsRS.end(), std::back_inserter(models));
  return models;
}
static User Insert(const User& user) {
  soci::session* sql = Database::GetInstance()->GetSession();
  User model;
  *sql << "INSERT INTO Users (age, name) VALUES (:age, :name) RETURNING * ", soci::use(user), soci::into(model);
  return model;
}
static std::vector<User> DeleteByAge(const int& age) {
  soci::session* sql = Database::GetInstance()->GetSession();
  std::vector<User> models;
  soci::rowset<User> modelRS = (sql->prepare << "DELETE FROM Users WHERE age = :age RETURNING *", soci::use(age));
  std::move(modelRS.begin(), modelRS.end(), std::back_inserter(models));
  return models;
}
static std::vector<User> DeleteByName(const std::string& name) {
  soci::session* sql = Database::GetInstance()->GetSession();
  std::vector<User> models;
  soci::rowset<User> modelRS = (sql->prepare << "DELETE FROM Users WHERE name = :name RETURNING *", soci::use(name));
  std::move(modelRS.begin(), modelRS.end(), std::back_inserter(models));
  return models;
}
static std::vector<User> UpdateByAge(const int& age, const User& user) {
  soci::session* sql = Database::GetInstance()->GetSession();
  std::vector<User> models;
  soci::rowset<User> modelRS = (sql->prepare << "UPDATE Users SET age = :age, name = :name WHERE age=:age RETURNING *", soci::use(user.age), soci::use(user.name), soci::use(age));
  std::move(modelRS.begin(), modelRS.end(), std::back_inserter(models));
  return models;
}
static std::vector<User> UpdateByName(const std::string& name, const User& user) {
  soci::session* sql = Database::GetInstance()->GetSession();
  std::vector<User> models;
  soci::rowset<User> modelRS = (sql->prepare << "UPDATE Users SET age = :age, name = :name WHERE name=:name RETURNING *", soci::use(user.age), soci::use(user.name), soci::use(name));
  std::move(modelRS.begin(), modelRS.end(), std::back_inserter(models));
  return models;
}
};


template<>
struct soci::type_conversion<User> {
  typedef soci::values base_type;

  static void from_base(soci::values const& v, soci::indicator ind, User& p) {
		p.age = v.get<int>("age");
		p.name = v.get<std::string>("name");

  }

  static void to_base(const User& p, soci::values& v, soci::indicator& ind) {
		v.set("age", p.age);
		v.set("name", p.name);

    ind = soci::i_ok;
  }
};

