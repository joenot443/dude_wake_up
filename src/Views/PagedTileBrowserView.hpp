#ifndef PagedTileBrowserView_hpp
#define PagedTileBrowserView_hpp

#include "ofMain.h"
#include "TileItem.hpp"
#include "ofxImGui.h"
#include "UUID.hpp"
#include "AbstractTileBrowserView.hpp"

class PagedTileBrowserView : public AbstractTileBrowserView {
public:
  void setup() override;
  void draw() override;
  void update() override;
  void setTileItems(std::vector<std::shared_ptr<TileItem>> items) override;
  void setCallback(std::function<void(std::shared_ptr<TileItem>)> callback) override;
  void setWidth(float width);
  float leftPadding = 0.0f;

  PagedTileBrowserView(int columns, int rows) 
    : columnCount(columns)
    , rowCount(rows)
    , currentPage(0)
    , viewWidth(0)
    , tileBrowserId(UUID::generateUUID()) {};
  
  PagedTileBrowserView(std::vector<std::shared_ptr<TileItem>> items, int columns, int rows)
    : tileItems(items)
    , columnCount(columns)
    , rowCount(rows)
    , currentPage(0)
    , viewWidth(0)
    , tileBrowserId(UUID::generateUUID()) {};

private:
  std::vector<std::shared_ptr<TileItem>> tileItems;
  std::function<void(std::shared_ptr<TileItem>)> tileClickCallback;
  int columnCount;
  int rowCount;
  int currentPage;
  std::string tileBrowserId;
  float viewWidth;
  
  int getItemsPerPage() const { return columnCount * rowCount; }
  int getTotalPages() const { 
    return (tileItems.size() + getItemsPerPage() - 1) / getItemsPerPage(); 
  }
  
  ImVec2 calculateTileSize() const;
};

#endif /* PagedTileBrowserView_hpp */ 