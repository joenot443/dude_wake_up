//
//  StrandTileView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 6/30/24.
//

#ifndef StrandTileView_hpp
#define StrandTileView_hpp

#include <stdio.h>
#include "AvailableStrand.hpp"

struct StrandTileView {
  static void draw(std::shared_ptr<AvailableStrand> strand);
};

#endif /* StrandTileView_hpp */
