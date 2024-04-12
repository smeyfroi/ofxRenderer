#pragma once

#include "Shader.h"

class AddTextureShader : public Shader {

public:
  void render(const ofFbo& fbo_, const ofFbo& addedTexture_, float multiplier_) {
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    shader.begin();
    shader.setUniformTexture("addedTexture", addedTexture_.getTexture(), 1);
    shader.setUniform1f("multiplier", multiplier_);
    fbo_.draw(0, 0);
    shader.end();
  }

protected:
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0;
                uniform sampler2D addedTexture;
                uniform float multiplier;

                void main() {
                  vec2 xy = gl_TexCoord[0].st;
                  vec4 color = texture2D(tex0, xy);
                  vec4 addedColor = texture2D(addedTexture, xy) * multiplier;
                  gl_FragColor = color + addedColor;
                }
                );
  }
};
