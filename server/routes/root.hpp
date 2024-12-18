#pragma once

#include "../../dependencies/CppHttp/include/CppHttp.hpp"
#include "../models.hpp"

ROUTE_GET("/", root) {
  std::string response = "Posts: ";
  std::ostream os = std::ostream(req.m_info.consoleStream);

  Post post;
  post.title = "Test";
  post.description = "Test description";
  std::stringstream("2021-01-01 00:00:00") >> std::get_time(&post.date, "%Y-%m-%d %H:%M:%S");

  os << "TEST" << std::endl;
  Post::Insert(post);

  for (auto post : Post::SelectAll()) {
    response += post.title + " ";
  }

  return {(CppHttp::Net::ResponseType)200, response, {}};    
}
