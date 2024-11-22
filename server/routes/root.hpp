#pragma once

#include "../../dependencies/CppHttp/include/CppHttp.hpp"
#include "../models.hpp"

ROUTE_GET("/", root) {
  std::string response = "Posts: ";

  std::ostream os = std::ostream(req.m_info.consoleStream);
  os << "TEST" << std::endl;

  Post post;
  post.title = "Test";
  post.description = "Test description";

  Post::Insert(post);

  for (auto post : Post::SelectAll()) {
    response += post.title + " ";
  }

  return {(CppHttp::Net::ResponseType)200, response, {}};    
}
