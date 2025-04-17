#pragma once

#include "Shader.h"
#include "ofGraphics.h"

class LogisticFnShader : public Shader {

public:
  void render(const ofBaseDraws& drawable, glm::vec4 clampFactor) {
    shader.begin();
    shader.setUniform4f("clampFactor", clampFactor);
    drawable.draw(0, 0);
    shader.end();
  }
  
  // clampFactor element == 0.0 means don't apply a Logistic Curve
  void render(PingPongFbo& fbo_,
              glm::vec4 clampFactor)
  {
    fbo_.getTarget().begin();
    render(fbo_.getSource(), clampFactor);
    fbo_.getTarget().end();
    fbo_.swap();
  }

protected:
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0;
                uniform vec4 clampFactor;

                void main() {
                  vec2 xy = gl_TexCoord[0].st;
                  vec4 color = texture2D(tex0, xy);
                  
//                  float e = 2.718;
//                  float x1 = 1.0 - pow(e, -clampFactor*xy.x); // logistic function
//                  gl_FragColor = vec4(x1, xy.y, 0.0, 1.0);

                  vec4 e = vec4(2.718);
                  vec4 epsilon = vec4(0.01);
                  gl_FragColor = mix(color,
                                     1.0 - pow(e, -clampFactor*color),
                                     step(epsilon, clampFactor)
                                     );
                }
                );
  }
};
