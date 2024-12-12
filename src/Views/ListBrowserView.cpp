//
//  ListBrowserView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 4/6/23.
//

#include "ListBrowserView.hpp"
#include "Strings.hpp"

void ListBrowserView::setup() {
  
}

void ListBrowserView::update() {
  
}

void ListBrowserView::draw() {
  if (ImGui::BeginListBox(formatString("##%s", id.c_str()).c_str(), ImGui::GetContentRegionAvail()))
  {
      for (int n = 0; n < tileItems.size(); n++)
      {
        ImGui::Selectable(tileItems[n]->name.c_str(), false);
        tileItems[n]->dragCallback();
      }
      ImGui::EndListBox();
  }
}
