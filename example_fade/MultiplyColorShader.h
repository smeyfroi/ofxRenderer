#pragma once

#include "Shader.h"
#include "ofGraphics.h"

class MultiplyColorShader : public Shader {

public:
  // clampFactor == 0.0 means don't apply a Logistic Curve to clamp
  void render(PingPongFbo& fbo_,
              glm::vec4 multiplyBy,
              glm::vec4 clampFactor = { 0.0, 0.0, 0.0, 0.0 })
  {
    fbo_.getTarget().begin();
    {
      shader.begin();
      shader.setUniform4f("multiplyBy", multiplyBy);
      shader.setUniform4f("clampFactor", clampFactor);
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
                uniform vec4 clampFactor;
                
                void main() {
                  vec2 xy = gl_TexCoord[0].st;
                  
//                  float e = 2.718;
//                  float x1 = 1.0 - pow(e, -clampFactor*xy.x); // logistic function
//                  gl_FragColor = vec4(x1, xy.y, 0.0, 1.0);

                  vec4 color = texture2D(tex0, xy);
                  vec4 multiplied = color * multiplyBy;
//                  gl_FragColor = min(vec4(1,1,1,1), multiplied);

                  // Clamp upper to 1.0 with a Logistic function
                  // where a (clampFactor) is steepness of curve: y={1-({e^{-ax}})};a=2
                  vec4 e = vec4(2.718);
                  vec4 epsilon = vec4(0.01);
                  gl_FragColor = mix(multiplied,
                                     1.0 - pow(e, -clampFactor*multiplied),
                                     step(epsilon, clampFactor)
                                     );
                }
                );
  }
};
