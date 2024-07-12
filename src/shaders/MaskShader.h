#pragma once

#include "Shader.h"

class MaskShader : public Shader {
  
public:
  void render(const ofBaseDraws& foreground, const ofFbo& maskFbo, float width, float height) {
    shader.begin();
    shader.setUniformTexture("mask", maskFbo.getTexture(), 1);
//    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
//    ofSetColor(255);
    foreground.draw(0, 0, width, height);
    shader.end();
  }

protected:
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0;
                uniform sampler2D mask;

                void main() {
                  vec2 xy = gl_TexCoord[0].st;
                  
                  // Draw foreground color with the mask red
                  vec4 foregroundColor = texture2D(tex0, xy);
                  float maskAlpha = texture2D(mask, xy).r;
                  gl_FragColor = vec4(foregroundColor.rgb, maskAlpha * foregroundColor.a);
                }
                );
  }
};
