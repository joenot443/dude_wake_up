//
//  VideoSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/7/24.
//

#include <stdio.h>
#include "VideoSource.hpp"
#include "CommonViews.hpp"

void VideoSource::drawMaskSettings() {
 CommonViews::ShaderCheckbox(settings->maskEnabled);
 if (settings->maskEnabled->boolValue) {
   CommonViews::Slider("Mask Tolerance", "##maskTolerance", settings->maskTolerance);
   CommonViews::ShaderColor(settings->maskColor);
 }
}
