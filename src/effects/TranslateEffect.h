#include "Effect.h"
#include "PingPongFbo.h"

/*
 * translateBy is a normalised value.
 */
class TranslateEffect : public Effect {
public:
  TranslateEffect(glm::vec2 translateBy_ = { 0.0, 0.001 }, float alpha_ = 1.0) :
  translateBy { translateBy_ },
  alpha { alpha_ }
  {}

  void draw(PingPongFbo& fbo) {
    fbo.getTarget().begin();
    {
      ofEnableBlendMode(OF_BLENDMODE_ALPHA);
      ofSetColor(ofFloatColor {1.0, 1.0, 1.0, alpha });
      ofPushMatrix();
      ofTranslate(translateBy * glm::vec2 { ofGetWidth(), ofGetHeight() });
      fbo.getSource().draw(0, 0, ofGetWidth(), ofGetHeight());
      ofPopMatrix();
    }
    fbo.getTarget().end();
    fbo.swap();
  }

  glm::vec2 translateBy;
  float alpha;
};
