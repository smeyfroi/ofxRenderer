#include "Effect.h"

class FadeEffect : public Effect {
public:
  FadeEffect(float fadeAmount = 0.001) : fadeAmount(fadeAmount) {}

  void draw() {
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    ofSetColor(ofFloatColor {0.0, 0.0, 0.0, fadeAmount });
    ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
  }

  float fadeAmount;
};
