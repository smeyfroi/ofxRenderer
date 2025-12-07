#pragma once

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
      ofPushStyle();
      ofClear(0, 255);
      ofEnableBlendMode(OF_BLENDMODE_ALPHA);
      ofSetColor(ofFloatColor {1.0, 1.0, 1.0, alpha });
      glm::vec2 pos = translateBy * glm::vec2 { fbo.getSource().getHeight(), fbo.getSource().getHeight() };
      fbo.getSource().draw(pos);
      ofPopStyle();
    }
    fbo.getTarget().end();
    fbo.swap();
  }

  glm::vec2 translateBy;
  float alpha;
};
