#pragma once

#include "Shader.h"
#include "ofGraphics.h"

class MultiplyColorShader : public Shader {

public:
  // clampFactor == 0 means don't apply a Logistic Curve to clamp
  void render(PingPongFbo& fbo_, glm::vec4 multiplyBy, float clampFactor = 0.0) {
    fbo_.getTarget().begin();
    {
      shader.begin();
      shader.setUniform4f("multiplyBy", multiplyBy);
      shader.setUniform1f("clamp", clampFactor);
      fbo_.getSource().draw(0, 0);
      shader.end();
    }
    fbo_.getTarget().end();
    fbo_.swap();
  }

protected:
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0;
                uniform vec4 multiplyBy;
                uniform float clampFactor;
                
                void main() {
                  float e = 2.718;
                  vec2 xy = gl_TexCoord[0].st;
                  vec4 color = texture2D(tex0, xy);
                  vec4 multiplied = color * multiplyBy;
                  gl_FragColor = min(vec4(1,1,1,1), multiplied);
//                  if (clampFactor > 0) gl_FragColor = vec4(multiplied.rgb, 1.0);
//                  else gl_FragColor = multiplied;
                  // Logistic function where a is steepness of curve: y={1-({e^{-ax}})};a=2
//                  if (clampFactor > 0) gl_FragColor = vec4(1.0 - pow(e, -clampFactor*multiplied.r),
//                                                           1.0 - pow(e, -clampFactor*multiplied.g),
//                                                           1.0 - pow(e, -clampFactor*multiplied.b),
//                                                           1.0 - pow(e, -clampFactor*multiplied.a));
//                  else gl_FragColor = multiplied;
                }
                );
  }
};
