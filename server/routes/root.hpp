#pragma once

#include "../../dependencies/CppHttp/include/CppHttp.hpp"
#include "../models.hpp"

Post post;

ROUTE_GET("/", root) {
  std::string response = "Posts: ";

  post.user_id = 0;
  post.title = "Test";
  post.description = "Test description";
  std::stringstream("2021-01-01 00:00:00") >> std::get_time(&post.date, "%Y-%m-%d %H:%M:%S");

  Post::Insert(post);

  for (auto post : Post::SelectAll()) {
    response += post.title + " ";
  }

  return {(CppHttp::Net::ResponseType)200, response, {}};    
}
