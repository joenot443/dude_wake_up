#ifndef TileItem_hpp
#define TileItem_hpp

#include "ofMain.h"
#include "ShaderType.hpp"
#include "ofxImGui.h"
#include "AvailableShader.hpp"
#include <stdio.h>

class TileItem {
public:
  virtual ~TileItem() {} 
  std::string name;
  std::string category = "";
  ImTextureID textureID;
  int index;
  ShaderType shaderType;
  // Closure which will be called when the tile is clicked
  std::function<void()> dragCallback;

  TileItem(std::string name, ImTextureID textureID, int index,
           std::function<void()> dragCallback, std::string category = "", ShaderType type = ShaderTypeNone)
      : name(name), textureID(textureID), index(index), shaderType(type), category(category),
        dragCallback(dragCallback){};
};

static std::shared_ptr<TileItem> tileItemForShader(std::shared_ptr<AvailableShader> shader) {
  // Create a closure which will be called when the tile is clicked
  std::function<void()> dragCallback = [shader]()
  {
    // Create a payload to carry the video source
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
      // Set payload to carry the index of our item (could be anything)
      ImGui::SetDragDropPayload("NewShader", &shader->type,
                                sizeof(ShaderType));
      ImGui::Text("%s", shader->name.c_str());
      ImGui::EndDragDropSource();
    }
  };
  ImTextureID textureId = (ImTextureID)(uint64_t) shader->preview->texData.textureID;
  return std::make_shared<TileItem>(shader->name, textureId, 0, dragCallback, "", shader->type);
}

static std::vector<std::shared_ptr<TileItem>> tileItemsForShaders(std::vector<std::shared_ptr<AvailableShader>> shaders) {
  std::vector<std::shared_ptr<TileItem>> tileItems = {};
  for (auto shader : shaders)
  {
    tileItems.push_back(tileItemForShader(shader));
  }
  return tileItems;
}

#endif /* TileItem_hpp */
