//
//  LibraryService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/20/23.
//

#include "LibraryService.hpp"
#include "httplib.h"
#include "base64.h"

void LibraryService::setup() {
  
}

void LibraryService::uploadChainer(const std::string &name, const std::string &author, const std::shared_ptr<ShaderChainer> chainer, std::function<void()> success_callback, std::function<void(const std::string &)> error_callback) {
  // Encode chainer data to base64
  std::string chainer_base64 = httplib::detail::base64_encode(chainer->serialize().dump());

  // Prepare the JSON body using nlohmann/json
  nlohmann::json json_body;
  json_body["name"] = name;
  json_body["author"] = author;
  json_body["chainer"] = chainer_base64;

  // Create an httplib client instance
  httplib::Client client("http://localhost", 3000);

  // Send POST request
  auto res = client.Post("/chainers/new", json_body.dump(), "application/json");

  if (res && res->status == 200) {
      std::cout << "Successful chainer upload." << std::endl;
      success_callback();
  } else {
    std::string error_message = "Error: Failed to upload chainer";
      error_callback(error_message);
  }
}
