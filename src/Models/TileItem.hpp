#ifndef TileItem_hpp
#define TileItem_hpp

#include "ofMain.h"
#include "Console.hpp"
#include "ShaderType.hpp"
#include "ofxImGui.h"
#include "AvailableShader.hpp"
#include "UUID.hpp"
#include <stdio.h>

enum TileType {
  TileType_Shader,
  TileType_Source,
  TileType_Library,
  TileType_Strand,
  TileType_File,
};

class TileItem {
public:
  virtual ~TileItem() {} 
  std::string name;
  std::string category = "";
  std::string id;
  ImTextureID textureID;
  int index;
  ShaderType shaderType;
  TileType tileType;
  // Closure which will be called when the tile is clicked
  std::function<void(std::string)> dragCallback;

  TileItem(std::string name, ImTextureID textureID, int index,
           std::function<void(std::string)> dragCallback, std::string category = "", TileType tileType = TileType_Shader, ShaderType type = ShaderTypeNone)
  : name(name), textureID(textureID), index(index), shaderType(type), category(category), tileType(tileType), id(UUID::generateUUID()),
        dragCallback(dragCallback) {};
};

static std::shared_ptr<TileItem> tileItemForShader(std::shared_ptr<AvailableShader> shader) {
  // Create a closure which will be called when the tile is clicked
  std::function<void(std::string)> dragCallback = [shader](std::string tileId)
  {
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
      ImGui::SetDragDropPayload("NewShader", &shader->type,
                                sizeof(ShaderType));
      ImGui::Text("%s", shader->name.c_str());
     ImGui::Image(shader->preview->texData.textureID, ImVec2(128.0, 80.0));
      ImGui::EndDragDropSource();
    }
  };
  ImTextureID textureId = (ImTextureID)(uint64_t) shader->preview->texData.textureID;
  return std::make_shared<TileItem>(shader->name, textureId, 0, dragCallback, "", TileType_Shader, shader->type);
}

static std::vector<std::shared_ptr<TileItem>> tileItemsForShaders(std::vector<std::shared_ptr<AvailableShader>> shaders, std::string category = "") {
  std::vector<std::shared_ptr<TileItem>> tileItems = {};
  for (auto shader : shaders)
  {
    if (shader == nullptr) {
      log("Null shader in config, discarding");
      continue;
    }
    std::shared_ptr<TileItem> tileItem = tileItemForShader(shader);
    if (tileItem == nullptr) {
      continue;
    }
    if (category.length() != 0) {
      tileItem->category = category;
    }
    tileItems.push_back(tileItem);
  }
  return tileItems;
}

#endif /* TileItem_hpp */
