//
//  SourceTransform.hpp
//  dude_wake_up
//
//  Reusable two-pass crop + place operation. Owns the parameters, the
//  intermediate "cropped" FBO, and the IOSurface plumbing for the SwiftUI
//  inspector. TransformShader uses this; sources (WebcamSource, etc.) use it
//  to apply the same transform pipeline in their own saveFrame().
//
//  The crop pass writes the cropped subregion of the input into croppedFbo
//  stretched-to-fill (so the IOSurface backing it has a stable size). The
//  place pass draws croppedFbo back onto the output FBO at the placement
//  rectangle dictated by mode/translate/scale. The aspect distortion in the
//  intermediate cancels out because the place pass draws back at the correct
//  (cropWidth, cropHeight)-derived aspect.
//

#ifndef SourceTransform_hpp
#define SourceTransform_hpp

#include "ofMain.h"
#include "Parameter.hpp"
#include "WaveformOscillator.hpp"
#include "Oscillator.hpp"
#include "IOSurfaceTextureManager.hpp"
#include <memory>
#include <string>
#include <vector>

class SourceTransform {
public:
  // Crop region (fractions of input frame)
  std::shared_ptr<Parameter> minX;
  std::shared_ptr<Parameter> maxX;
  std::shared_ptr<Parameter> minY;
  std::shared_ptr<Parameter> maxY;

  // Placement
  std::shared_ptr<Parameter> translateX;
  std::shared_ptr<Parameter> translateY;
  std::shared_ptr<Parameter> scale;
  std::shared_ptr<Parameter> scaleFromCenter;
  std::shared_ptr<Parameter> mode;
  std::shared_ptr<Parameter> center;

  // Content modifiers
  std::shared_ptr<Parameter> verticalFlip;
  std::shared_ptr<Parameter> horizontalFlip;
  std::shared_ptr<Parameter> backgroundEnabled;
  std::shared_ptr<Parameter> backgroundColor;

  // Oscillators
  std::shared_ptr<WaveformOscillator> minXOscillator;
  std::shared_ptr<WaveformOscillator> maxXOscillator;
  std::shared_ptr<WaveformOscillator> minYOscillator;
  std::shared_ptr<WaveformOscillator> maxYOscillator;
  std::shared_ptr<WaveformOscillator> translateXOscillator;
  std::shared_ptr<WaveformOscillator> translateYOscillator;
  std::shared_ptr<WaveformOscillator> scaleOscillator;

  /// Intermediate FBO holding the cropped-but-not-yet-placed pixels (F2).
  /// Stretched to fill its own (canvas-sized) extent so its IOSurface has a
  /// stable size; the stretch is undone in the place pass.
  std::shared_ptr<ofFbo> croppedFbo;

  /// Owner id (shader id or source id) — used as the suffix for the
  /// "transformCrop:<ownerId>" synthetic IOSurface that exposes croppedFbo.
  std::string ownerId;

  SourceTransform(std::string ownerId) : ownerId(ownerId) {
    minX = std::make_shared<Parameter>("Min X", 0.0, 0.0, 1.0);
    maxX = std::make_shared<Parameter>("Max X", 1.0, 0.0, 1.0);
    minY = std::make_shared<Parameter>("Min Y", 0.0, 0.0, 1.0);
    maxY = std::make_shared<Parameter>("Max Y", 1.0, 0.0, 1.0);
    translateX = std::make_shared<Parameter>("X", 0.0, -1.0, 1.0);
    translateY = std::make_shared<Parameter>("Y", 0.0, -1.0, 1.0);
    scale = std::make_shared<Parameter>("Scale", 1.0, 0.0, 5.0);
    scaleFromCenter = std::make_shared<Parameter>("Scale From Center", 0.0, 0.0, 1.0, ParameterType_Bool);
    mode = std::make_shared<Parameter>("Mode", 0, ParameterType_Int);
    center = std::make_shared<Parameter>("Center", ParameterType_Bool);
    verticalFlip = std::make_shared<Parameter>("Vertical Flip", 0.0, 0.0, 1.0, ParameterType_Bool);
    horizontalFlip = std::make_shared<Parameter>("Horizontal Flip", 0.0, 0.0, 1.0, ParameterType_Bool);
    backgroundEnabled = std::make_shared<Parameter>("Background Enabled", 0.0, 0.0, 1.0, ParameterType_Bool);
    backgroundColor = std::make_shared<Parameter>("Background Color", ParameterType_Color);
    backgroundColor->setColor({0.0f, 0.0f, 0.0f, 1.0f});

    mode->options = {"Standard", "Fill", "Fit", "Fill + Aspect"};

    minXOscillator = std::make_shared<WaveformOscillator>(minX);
    maxXOscillator = std::make_shared<WaveformOscillator>(maxX);
    minYOscillator = std::make_shared<WaveformOscillator>(minY);
    maxYOscillator = std::make_shared<WaveformOscillator>(maxY);
    translateXOscillator = std::make_shared<WaveformOscillator>(translateX);
    translateYOscillator = std::make_shared<WaveformOscillator>(translateY);
    scaleOscillator = std::make_shared<WaveformOscillator>(scale);
  }

  /// Append all transform params to a host Settings' parameters vector. Call
  /// before `registerParameters()` on the host.
  void appendParameters(std::vector<std::shared_ptr<Parameter>>& params) {
    params.insert(params.end(), {
      minX, maxX, minY, maxY,
      scale, scaleFromCenter,
      translateX, translateY,
      mode, center,
      verticalFlip, horizontalFlip,
      backgroundEnabled, backgroundColor
    });
  }

  /// Append oscillators to a host Settings' oscillators vector.
  void appendOscillators(std::vector<std::shared_ptr<Oscillator>>& oscs) {
    oscs.insert(oscs.end(), {
      minXOscillator, maxXOscillator,
      minYOscillator, maxYOscillator,
      translateXOscillator, translateYOscillator,
      scaleOscillator
    });
  }

  /// Two-pass crop + place. Reads from `input` (a texture sized
  /// frameWidth × frameHeight in input pixels) and writes the result to
  /// `output`. Exposes the intermediate cropped frame via IOSurface under
  /// the synthetic id "transformCrop:<ownerId>" if anyone is subscribed.
  void apply(ofTexture& input, std::shared_ptr<ofFbo> output) {
    if (!output || !output->isAllocated()) return;

    float frameWidth = input.getWidth();
    float frameHeight = input.getHeight();
    float canvasW = output->getWidth();
    float canvasH = output->getHeight();

    float cropX = minX->value * frameWidth;
    float cropY = minY->value * frameHeight;
    float cropWidth = (maxX->value - minX->value) * frameWidth;
    float cropHeight = (maxY->value - minY->value) * frameHeight;

    // ===== PASS 1: CROP =====
    if (!croppedFbo) croppedFbo = std::make_shared<ofFbo>();
    if (!croppedFbo->isAllocated() ||
        (int)croppedFbo->getWidth() != (int)canvasW ||
        (int)croppedFbo->getHeight() != (int)canvasH) {
      croppedFbo->allocate(canvasW, canvasH, GL_RGBA);
    }

    croppedFbo->begin();
    ofClear(0, 0, 0, 0);
    if (cropWidth > 0 && cropHeight > 0) {
      input.drawSubsection(0, 0,
                           croppedFbo->getWidth(), croppedFbo->getHeight(),
                           cropX, cropY,
                           cropWidth, cropHeight);
    }
    croppedFbo->end();

    // Expose F2 to the SwiftUI inspector if it's currently observed.
    {
      std::string syntheticId = "transformCrop:" + ownerId;
      auto* iom = IOSurfaceTextureManager::getInstance();
      if (iom && iom->isSharing(syntheticId)) {
        iom->blitExternalFrame(syntheticId, croppedFbo->getId(),
                                croppedFbo->getWidth(), croppedFbo->getHeight());
      }
    }

    // ===== PASS 2: PLACE =====
    float canvasAspect = canvasW / canvasH;
    float cropAspect = (cropHeight > 0) ? cropWidth / cropHeight : canvasAspect;

    float drawX = 0, drawY = 0;
    float drawW = canvasW, drawH = canvasH;

    // Auto-fit modes (Fill / Fit / Fill+Aspect) compute the natural placement
    // for the cropped frame, then apply `scale` as a uniform multiplier on
    // top and re-center. Scale=1 reproduces the natural behavior; scale<1
    // shrinks centered (background visible around); scale>1 overflows.
    float modeScale = scale->value;

    switch (mode->intValue) {
      case 1:
        // Fill — stretch to canvas, then apply scale and recenter.
        drawW = canvasW * modeScale;
        drawH = canvasH * modeScale;
        drawX = (canvasW - drawW) * 0.5f;
        drawY = (canvasH - drawH) * 0.5f;
        break;
      case 2:
        // Fit — preserve cropped aspect, fit within canvas.
        if (cropAspect > canvasAspect) {
          drawW = canvasW * modeScale;
          drawH = (canvasW / cropAspect) * modeScale;
        } else {
          drawH = canvasH * modeScale;
          drawW = (canvasH * cropAspect) * modeScale;
        }
        drawX = (canvasW - drawW) * 0.5f;
        drawY = (canvasH - drawH) * 0.5f;
        break;
      case 3:
        // Fill + Aspect — preserve cropped aspect, cover canvas.
        if (cropAspect > canvasAspect) {
          drawH = canvasH * modeScale;
          drawW = (canvasH * cropAspect) * modeScale;
        } else {
          drawW = canvasW * modeScale;
          drawH = (canvasW / cropAspect) * modeScale;
        }
        drawX = (canvasW - drawW) * 0.5f;
        drawY = (canvasH - drawH) * 0.5f;
        break;
      default: {
        drawW = cropWidth * scale->value;
        drawH = cropHeight * scale->value;
        float tX = center->boolValue ? (canvasW - cropWidth) * 0.5f : translateX->value * canvasW;
        float tY = center->boolValue ? (canvasH - cropHeight) * 0.5f : translateY->value * canvasH;
        drawX = tX;
        drawY = tY;
        if (scaleFromCenter->boolValue) {
          drawX -= (drawW - cropWidth) * 0.5f;
          drawY -= (drawH - cropHeight) * 0.5f;
        }
        break;
      }
    }

    output->begin();

    if (verticalFlip->value) ofScale(1, -1);
    if (horizontalFlip->value) ofScale(-1, 1);

    if (backgroundEnabled->boolValue) {
      auto c = backgroundColor->color->data();
      ofClear(c[0] * 255, c[1] * 255, c[2] * 255, c[3] * 255);
    } else {
      ofClear(0, 0, 0, 0);
    }

    ofPushMatrix();
    croppedFbo->draw(drawX, drawY, drawW, drawH);
    ofPopMatrix();

    output->end();
  }
};

#endif /* SourceTransform_hpp */
