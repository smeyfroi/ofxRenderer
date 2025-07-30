#include "Effect.h"

class FadeEffect : public Effect {
public:
  FadeEffect(float fadeAmount = 0.001) : fadeAmount(fadeAmount) {}

  void draw(float w, float h) {
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    ofSetColor(ofFloatColor {0.0, 0.0, 0.0, fadeAmount });
    ofFill();
    ofDrawRectangle(0, 0, w, h);
  }

  float fadeAmount;
};
