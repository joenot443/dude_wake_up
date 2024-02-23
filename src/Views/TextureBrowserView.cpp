//
//  TextureBrowserView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2/16/24.
//

#include "TextureBrowserView.hpp"
#include "ParameterTileBrowserView.hpp"
#include "TextureService.hpp"
#include "CommonViews.hpp"

std::vector<std::shared_ptr<ParameterTileItem>> tileItemsFromTextures(std::vector<std::shared_ptr<Texture>> textures)
{

};

TextureBrowserView::TextureBrowserView(std::shared_ptr<Parameter> parameter) : parameter(parameter), textures(TextureService::getService()->availableTextures()), tileBrowserView(tileItemsFromTextures(textures), parameter)
{
}

void TextureBrowserView::setup()
{
}

void TextureBrowserView::draw()
{
//  CommonViews::draw(textureTileItems, parameter);
}
