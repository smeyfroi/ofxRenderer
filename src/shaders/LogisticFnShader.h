#pragma once

#include "Shader.h"
#include "ofGraphics.h"

class LogisticFnShader : public Shader {

public:
  void render(const ofBaseDraws& drawable, glm::vec4 clampFactor) {
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    shader.begin();
    shader.setUniform4f("clampFactor", clampFactor);
    drawable.draw(0, 0);
    shader.end();
    ofPopStyle();
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
                in vec2 texCoordVarying;
                in vec2 colorVarying;
                out vec4 outputColor;
                uniform vec4 clampFactor;

                void main(void) {
                  vec4 color = texture(tex0, texCoordVarying);
                  
                  vec4 e = vec4(2.718);
                  vec4 epsilon = vec4(0.01);
                  
                  // SCALED TO [0.0, 1.0]
                  outputColor = mix(color,
                                    1.0 - pow(e, -clampFactor*color),
                                    step(epsilon, clampFactor)
                                    ) * 0.5 + 0.5;
                }
    );
  }
};
