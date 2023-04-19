//
//  FeedbackSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 4/12/23.
//

#include <stdio.h>
#include "FeedbackSource.hpp"
#include "FeedbackSourceService.hpp"

bool FeedbackSource::beingConsumed() {
  return FeedbackSourceService::getService()->isSourceBeingConsumed(id);
}
