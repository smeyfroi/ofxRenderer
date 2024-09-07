#pragma once

#include "Shader.h"

class MaskShader : public Shader {
  
public:
  void render(const ofBaseDraws& foreground, const ofFbo& maskFbo, float width, float height, bool invert = false) {
    shader.begin();
    shader.setUniformTexture("mask", maskFbo.getTexture(), 1);
    shader.setUniform1i("invert", invert);
    foreground.draw(0, 0, width, height);
    shader.end();
  }

protected:
  std::string getVertexShader() override {
    return GLSL(
                void main() {
                  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
                  gl_TexCoord[0] = gl_MultiTexCoord0;
                  gl_FrontColor = gl_Color; // allow ofSetColor to work
                }
                );
  }

  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0;
                uniform sampler2D mask;
                uniform int invert;

                void main() {
                  vec2 xy = gl_TexCoord[0].st;
                  
                  // Draw foreground color with the mask red as alpha
                  vec4 foregroundColor = texture2D(tex0, xy);
                  float positiveAlpha = texture2D(mask, xy).r;
                  float negativeAlpha = 1.0 - positiveAlpha;
                  float maskAlpha = (invert*negativeAlpha) + ((1.0-invert)*positiveAlpha);
                  gl_FragColor = gl_Color * vec4(foregroundColor.rgb, maskAlpha * foregroundColor.a);
                }
                );
  }
};
