#pragma once

#include "Shader.h"

// Handles different-sized addedTexture
// Clamps to [-1, 1]
class AddTextureShader : public Shader {

public:
  void render(const ofFbo& fbo_, const ofFbo& addedTexture_, float multiplier_) {
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    shader.begin();
    shader.setUniformTexture("addedTexture", addedTexture_.getTexture(), 1);
    shader.setUniform1f("multiplier", multiplier_);
    shader.setUniform1f("textureScale", fbo_.getWidth()/addedTexture_.getWidth());
    fbo_.draw(0, 0);
    shader.end();
  }

protected:

  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0;
                uniform sampler2D addedTexture;
                uniform float multiplier;
                uniform float textureScale;

                void main() {
                  vec2 xy = gl_TexCoord[0].st;
                  vec4 color = texture2D(tex0, xy);
                  vec2 xy1 = xy * textureScale;
                  vec4 addedColor = texture2D(addedTexture, xy1) * multiplier;
                  gl_FragColor = clamp(color + addedColor, vec4(-1.0, -1.0, 0.0, 0.0), vec4(1.0, 1.0, 0.0, 0.0)); // ***** NOTE CLAMP TO KEEP FLUID SIM STABLE ALSO KEEPS MEMORY OVERLAYS WORKING
                }
                );
  }
};
