#pragma once

#include "Shader.h"

class VelocityCflClampShader : public Shader {

public:
  void render(PingPongFbo& velocities, float dt, float maxDispUv) {
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);

    velocities.getTarget().begin();
    shader.begin();
    {
      shader.setUniformTexture("tex0", velocities.getSource().getTexture(), 0);
      shader.setUniform1f("dt", dt);
      shader.setUniform1f("maxDisp", maxDispUv);
      velocities.getSource().draw(0, 0);
    }
    shader.end();
    velocities.getTarget().end();

    velocities.swap();

    ofPopStyle();
  }

protected:
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0;
                uniform float dt;
                uniform float maxDisp;

                in vec2 texCoordVarying;
                out vec4 fragColor;

                void main() {
                  vec2 uv = texCoordVarying.xy;
                  vec2 v = texture(tex0, uv).xy;

                  float speed = length(v);
                  float disp = speed * dt;
                  if (disp > maxDisp && disp > 0.0) {
                    v *= maxDisp / disp;
                  }

                  fragColor = vec4(v, 0.0, 0.0);
                }
                );
  }
};
