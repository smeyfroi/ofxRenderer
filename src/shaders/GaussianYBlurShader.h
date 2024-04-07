#pragma once

#include "PingPongShader.h"

class GaussianYBlurShader : public PingPongShader {

public:
  GaussianYBlurShader(float blurAmount_) :
  blurAmount { blurAmount_ }
  {}
  
  inline void setBlurAmount(float blurAmount_) { blurAmount = blurAmount_; }
  
protected:
  void setupShaders() override {
    shader.setUniform1f("blurAmount", blurAmount);
  }
  
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0;
                uniform float blurAmount;

                void main() {
                  vec2 xy = gl_TexCoord[0].st;
                  vec4 color = vec4(0.0, 0.0, 0.0, 0.0);

                  color += 0.000229 * texture2D(tex0, xy + vec2(0.0, blurAmount * -4.0));
                  color += 0.005977 * texture2D(tex0, xy + vec2(0.0, blurAmount * -3.0));
                  color += 0.060598 * texture2D(tex0, xy + vec2(0.0, blurAmount * -2.0));
                  color += 0.241732 * texture2D(tex0, xy + vec2(0.0, blurAmount * -1.0));

                  color += 0.382928 * texture2D(tex0, xy);

                  color += 0.241732 * texture2D(tex0, xy + vec2(0.0, blurAmount * 1.0));
                  color += 0.060598 * texture2D(tex0, xy + vec2(0.0, blurAmount * 2.0));
                  color += 0.005977 * texture2D(tex0, xy + vec2(0.0, blurAmount * 3.0));
                  color += 0.000229 * texture2D(tex0, xy + vec2(0.0, blurAmount * 4.0));

                  gl_FragColor = color;
                }
                );
  }
  
private:
  float blurAmount;
};
