//
//  FeedbackShader.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/5/22.
//

#include "FeedbackShader.hpp"


void FeedbackShader::shade() {
  if (feedback->enabled.boolValue) {
    shaderMixer->setUniform1i(feedback->enabled.shaderKey, feedback->enabled.boolValue);
    shaderMixer->setUniform1f(feedback->mixSettings.keyValue.shaderKey,  feedback->mixSettings.keyValue.value);
    shaderMixer->setUniform1f(feedback->mixSettings.keyThreshold.shaderKey,
                             feedback->mixSettings.keyThreshold.value);
    shaderMixer->setUniform1f(feedback->mixSettings.mix.shaderKey,
                             feedback->mixSettings.mix.value);

    shaderMixer->setUniform3f(formatString("fb%d_hsb_x", idx), feedback->hsbSettings.hue.value, feedback->hsbSettings.saturation.value, feedback->hsbSettings.brightness.value);
    shaderMixer->setUniform3f(formatString("fb%d_hue_x", idx), 10.0, 0.0, 0.0);
    shaderMixer->setUniform3f(formatString("fb%d_rescale", idx),
                             feedback->miscSettings.xOffset.value,
                             feedback->miscSettings.yOffset.value,
                             1.0);
    
    shaderMixer->setUniform1f(formatString("fb%d_rotate", idx), feedback->miscSettings.rotate.value);
    
    shaderMixer->setUniform3f(formatString("fb%d_invert", idx), feedback->hsbSettings.invertHue,
                             feedback->hsbSettings.invertSaturation,
                             feedback->hsbSettings.invertBrightness);

  } else {
    disableFeedback();
  }
}


void FeedbackShader::disableFeedback() {
  shaderMixer->setUniform1i(feedback->enabled.shaderKey, feedback->enabled.boolValue);
  shaderMixer->setUniform1f(feedback->mixSettings.keyValue.shaderKey,  0.0);
  shaderMixer->setUniform1f(feedback->mixSettings.keyThreshold.shaderKey,
                           0.0);
  shaderMixer->setUniform1f(feedback->mixSettings.mix.shaderKey,
                           0.0);
  shaderMixer->setUniform1f(feedback->mixSettings.mix.shaderKey,
                           0.0);
  shaderMixer->setUniform3f(formatString("fb%d_hsb_x", idx), 0.0, 0.0, 1.0);
  shaderMixer->setUniform3f(formatString("fb%c_hue_x", idx), 10.0, 0.0, 0.0);
  shaderMixer->setUniform3f(formatString("fb%c_rescale", idx),
                           0.0,
                           0.0,
                           1.0);
  
  shaderMixer->setUniform1f(formatString("fb%c_rotate", idx), 0.0);
  
  shaderMixer->setUniform3f(formatString("fb%c_invert", idx), feedback->hsbSettings.invertHue,
                           feedback->hsbSettings.invertSaturation,
                           feedback->hsbSettings.invertBrightness);
}
