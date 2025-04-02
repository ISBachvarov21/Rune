
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
	int id;
	std::string title;
	int user_id;


  
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
  static std::vector<Post> SelectById(const int& id) { 
    soci::session* sql = Database::GetInstance()->GetSession();
    soci::rowset<Post> modelsRS = (sql->prepare << "SELECT * FROM Posts WHERE id = :id", soci::use(id));
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
  static std::vector<Post> SelectByUser_id(const int& user_id) { 
    soci::session* sql = Database::GetInstance()->GetSession();
    soci::rowset<Post> modelsRS = (sql->prepare << "SELECT * FROM Posts WHERE user_id = :user_id", soci::use(user_id));
    std::vector<Post> models;
    std::move(modelsRS.begin(), modelsRS.end(), std::back_inserter(models));
    return models;
  }
  static Post Insert(const Post& post) {
    soci::session* sql = Database::GetInstance()->GetSession();
    Post model;
    *sql << "INSERT INTO Posts (date, description, id, title, user_id) VALUES (:date, :description, :id, :title, :user_id) RETURNING * ", soci::use(post), soci::into(model);
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
  static std::vector<Post> DeleteById(const int& id) {
    soci::session* sql = Database::GetInstance()->GetSession();
    std::vector<Post> models;
    soci::rowset<Post> modelRS = (sql->prepare << "DELETE FROM Posts WHERE id = :id RETURNING *", soci::use(id));
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
  static std::vector<Post> DeleteByUser_id(const int& user_id) {
    soci::session* sql = Database::GetInstance()->GetSession();
    std::vector<Post> models;
    soci::rowset<Post> modelRS = (sql->prepare << "DELETE FROM Posts WHERE user_id = :user_id RETURNING *", soci::use(user_id));
    std::move(modelRS.begin(), modelRS.end(), std::back_inserter(models));
    return models;
  }
  static std::vector<Post> UpdateByDate(const std::tm& date, const Post& post) {
    soci::session* sql = Database::GetInstance()->GetSession();
    std::vector<Post> models;
    soci::rowset<Post> modelRS = (sql->prepare << "UPDATE Posts SET date = :date, description = :description, id = :id, title = :title, user_id = :user_id WHERE date=:date RETURNING *", soci::use(post.date), soci::use(post.description), soci::use(post.id), soci::use(post.title), soci::use(post.user_id), soci::use(date));
    std::move(modelRS.begin(), modelRS.end(), std::back_inserter(models));
    return models;
  }
  static std::vector<Post> UpdateByDescription(const std::string& description, const Post& post) {
    soci::session* sql = Database::GetInstance()->GetSession();
    std::vector<Post> models;
    soci::rowset<Post> modelRS = (sql->prepare << "UPDATE Posts SET date = :date, description = :description, id = :id, title = :title, user_id = :user_id WHERE description=:description RETURNING *", soci::use(post.date), soci::use(post.description), soci::use(post.id), soci::use(post.title), soci::use(post.user_id), soci::use(description));
    std::move(modelRS.begin(), modelRS.end(), std::back_inserter(models));
    return models;
  }
  static std::vector<Post> UpdateById(const int& id, const Post& post) {
    soci::session* sql = Database::GetInstance()->GetSession();
    std::vector<Post> models;
    soci::rowset<Post> modelRS = (sql->prepare << "UPDATE Posts SET date = :date, description = :description, id = :id, title = :title, user_id = :user_id WHERE id=:id RETURNING *", soci::use(post.date), soci::use(post.description), soci::use(post.id), soci::use(post.title), soci::use(post.user_id), soci::use(id));
    std::move(modelRS.begin(), modelRS.end(), std::back_inserter(models));
    return models;
  }
  static std::vector<Post> UpdateByTitle(const std::string& title, const Post& post) {
    soci::session* sql = Database::GetInstance()->GetSession();
    std::vector<Post> models;
    soci::rowset<Post> modelRS = (sql->prepare << "UPDATE Posts SET date = :date, description = :description, id = :id, title = :title, user_id = :user_id WHERE title=:title RETURNING *", soci::use(post.date), soci::use(post.description), soci::use(post.id), soci::use(post.title), soci::use(post.user_id), soci::use(title));
    std::move(modelRS.begin(), modelRS.end(), std::back_inserter(models));
    return models;
  }
  static std::vector<Post> UpdateByUser_id(const int& user_id, const Post& post) {
    soci::session* sql = Database::GetInstance()->GetSession();
    std::vector<Post> models;
    soci::rowset<Post> modelRS = (sql->prepare << "UPDATE Posts SET date = :date, description = :description, id = :id, title = :title, user_id = :user_id WHERE user_id=:user_id RETURNING *", soci::use(post.date), soci::use(post.description), soci::use(post.id), soci::use(post.title), soci::use(post.user_id), soci::use(user_id));
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
		p.id = v.get<int>("id");
		p.title = v.get<std::string>("title");
		p.user_id = v.get<int>("user_id");

  }

  static void to_base(const Post& p, soci::values& v, soci::indicator& ind) {
		v.set("date", p.date);
		v.set("description", p.description);
		v.set("id", p.id);
		v.set("title", p.title);
		v.set("user_id", p.user_id);

    ind = soci::i_ok;
  }
};

class User {
public:
  User() = default;
  ~User() = default;

	std::string email;
	int id;


  
  static std::vector<User> SelectAll() {
    soci::session* sql = Database::GetInstance()->GetSession();
    soci::rowset<User> modelsRS = (sql->prepare << "SELECT * FROM Users");
    std::vector<User> models;
    std::move(modelsRS.begin(), modelsRS.end(), std::back_inserter(models));
    return models;
  }
  static std::vector<User> SelectByEmail(const std::string& email) { 
    soci::session* sql = Database::GetInstance()->GetSession();
    soci::rowset<User> modelsRS = (sql->prepare << "SELECT * FROM Users WHERE email = :email", soci::use(email));
    std::vector<User> models;
    std::move(modelsRS.begin(), modelsRS.end(), std::back_inserter(models));
    return models;
  }
  static std::vector<User> SelectById(const int& id) { 
    soci::session* sql = Database::GetInstance()->GetSession();
    soci::rowset<User> modelsRS = (sql->prepare << "SELECT * FROM Users WHERE id = :id", soci::use(id));
    std::vector<User> models;
    std::move(modelsRS.begin(), modelsRS.end(), std::back_inserter(models));
    return models;
  }
  static User Insert(const User& user) {
    soci::session* sql = Database::GetInstance()->GetSession();
    User model;
    *sql << "INSERT INTO Users (email, id) VALUES (:email, :id) RETURNING * ", soci::use(user), soci::into(model);
    return model;
  }
  static std::vector<User> DeleteByEmail(const std::string& email) {
    soci::session* sql = Database::GetInstance()->GetSession();
    std::vector<User> models;
    soci::rowset<User> modelRS = (sql->prepare << "DELETE FROM Users WHERE email = :email RETURNING *", soci::use(email));
    std::move(modelRS.begin(), modelRS.end(), std::back_inserter(models));
    return models;
  }
  static std::vector<User> DeleteById(const int& id) {
    soci::session* sql = Database::GetInstance()->GetSession();
    std::vector<User> models;
    soci::rowset<User> modelRS = (sql->prepare << "DELETE FROM Users WHERE id = :id RETURNING *", soci::use(id));
    std::move(modelRS.begin(), modelRS.end(), std::back_inserter(models));
    return models;
  }
  static std::vector<User> UpdateByEmail(const std::string& email, const User& user) {
    soci::session* sql = Database::GetInstance()->GetSession();
    std::vector<User> models;
    soci::rowset<User> modelRS = (sql->prepare << "UPDATE Users SET email = :email, id = :id WHERE email=:email RETURNING *", soci::use(user.email), soci::use(user.id), soci::use(email));
    std::move(modelRS.begin(), modelRS.end(), std::back_inserter(models));
    return models;
  }
  static std::vector<User> UpdateById(const int& id, const User& user) {
    soci::session* sql = Database::GetInstance()->GetSession();
    std::vector<User> models;
    soci::rowset<User> modelRS = (sql->prepare << "UPDATE Users SET email = :email, id = :id WHERE id=:id RETURNING *", soci::use(user.email), soci::use(user.id), soci::use(id));
    std::move(modelRS.begin(), modelRS.end(), std::back_inserter(models));
    return models;
  }
};


template<>
struct soci::type_conversion<User> {
  typedef soci::values base_type;

  static void from_base(soci::values const& v, soci::indicator ind, User& p) {
		p.email = v.get<std::string>("email");
		p.id = v.get<int>("id");

  }

  static void to_base(const User& p, soci::values& v, soci::indicator& ind) {
		v.set("email", p.email);
		v.set("id", p.id);

    ind = soci::i_ok;
  }
};

