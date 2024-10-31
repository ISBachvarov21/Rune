#pragma once

#include "../../dependencies/CppHttp/include/CppHttp.hpp"
#include "../models.hpp"

ROUTE_GET("/", root) {
  User user;
  user.name = "John";
  user.age = 30;
  User::Insert(user);
  std::string response = "Users: ";
  for (const User& user : User::SelectAll()) {
    response += "\n" + user.name + " " + std::to_string(user.age) + " ";
  }
  return {(CppHttp::Net::ResponseType)200, response, {}};    
}
