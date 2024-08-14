////
////  UploadChainerView.cpp
////  dude_wake_up
////
////  Created by Joe Crozier on 3/20/23.
////
//
//#include "UploadChainerView.h"
//#include "LibraryService.hpp"
//#include "ofMain.h"
//#include "ofxImGui.h"
//#include "imgui.h"
//#include "imgui_node_editor.h"
//
//namespace ed = ax::NodeEditor;
//
//void drawUploadEffectChainWindow(bool &open, std::shared_ptr<ShaderChainer> shaderChainer) {
//  if (!open) return;
//  // Get the user's mouse position
//  ImVec2 mouse_pos = ImGui::GetIO().MousePos;
//
//  // Set the window position and size
//  ImGui::SetNextWindowPos(mouse_pos, ImGuiCond_Once);
//  ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_Once);
//  
//  ImGui::Begin("Upload Effect Chain", &open);
//    
//    static char name[128] = "";
//    static char author[128] = "";
//    ImGui::InputText("Name", name, sizeof(name));
//    ImGui::InputText("Author", author, sizeof(author));
//    
//    if (ImGui::Button("Cancel")) {
//      open = false;
//    }
//    ImGui::SameLine();
//    if (ImGui::Button("Upload")) {
//      nlohmann::json empty_json = nlohmann::json::object();
//      
//      LibraryService::getService()->uploadChainer(
//                                                  name,
//                                                  author,
//                                                  shaderChainer,
//                                                  [&]() {
//                                                    log("Success");
//                                                    open = false;
//                                                  },
//                                                  [&](const std::string &error_message) {
//                                                    log(error_message);
//                                                    open = false;
//                                                  }
//                                                  );
//    }
//  ImGui::End();
//}
//
//
//void UploadChainerView::draw(std::shared_ptr<ShaderChainer> shaderChainer) {
//  bool open = true;
//  drawUploadEffectChainWindow(open, shaderChainer);
//}
