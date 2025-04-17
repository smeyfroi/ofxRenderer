#pragma once

#include "Shader.h"
#include "PingPongFbo.h"

// Handles different-sized addedTexture
// Clamps values to [-1, 1]
class AddTextureShader : public Shader {

public:
  void render(PingPongFbo& fbo_, const ofFbo& addedTexture_, float multiplier_) {
    fbo_.getTarget().begin();
    shader.begin();
    shader.setUniformTexture("addedTexture", addedTexture_.getTexture(), 1);
    shader.setUniform1f("multiplier", multiplier_);
    fbo_.getSource().draw(0, 0);
    shader.end();
    fbo_.getTarget().end();
    fbo_.swap();
  }
  
  // Is this for adding the crystals? The scaling is wrong because we're in normalised coords!
//  void render(const ofFbo& fbo_, const ofFbo& addedTexture_, float multiplier_, float textureScale_ = 0.0) {
//    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
//    shader.begin();
//    shader.setUniformTexture("addedTexture", addedTexture_.getTexture(), 1);
//    shader.setUniform1f("multiplier", multiplier_);
//    if (textureScale_ == 0.0) {
//      shader.setUniform1f("textureScale", fbo_.getWidth()/addedTexture_.getWidth());
//    } else {
//      shader.setUniform1f("textureScale", textureScale_);
//    }
//    fbo_.draw(0, 0);
//    shader.end();
//  }

protected:

  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0;
                uniform sampler2D addedTexture;
                uniform float multiplier;
//                uniform float textureScale;
//                uniform float offsetX;
//                uniform float offsetY;

                void main() {
                  vec2 xy = gl_TexCoord[0].st;
                  vec4 color = texture2D(tex0, xy);
                  vec4 addedColor = texture2D(addedTexture, xy) * multiplier;
                  gl_FragColor = clamp(color + addedColor, vec4(-1.0), vec4(1.0)); // ***** NOTE CLAMP TO KEEP FLUID SIM STABLE
                }
                );
  }
};
