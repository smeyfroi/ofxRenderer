#pragma once

#include "Shader.h"

class VelocityBoundaryShader : public Shader {

public:
  void render(PingPongFbo& velocities) override {
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);

    velocities.getTarget().begin();
    shader.begin();
    {
      shader.setUniformTexture("tex0", velocities.getSource().getTexture(), 1);
      shader.setUniform2f("texSize", glm::vec2(velocities.getWidth(), velocities.getHeight()));
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
                 uniform vec2 texSize;
                 in vec2 texCoordVarying;
                 out vec4 fragColor;

                 void main() {
                   vec2 uv = texCoordVarying.xy;
                   vec2 v = texture(tex0, uv).xy;


                  // gl_FragCoord is bottom-left origin in pixel space.
                  // First/last column/row are considered boundaries.
                  float px = gl_FragCoord.x;
                  float py = gl_FragCoord.y;

                  if (px < 1.5 || px > texSize.x - 0.5) {
                    v.x = 0.0;
                  }

                  if (py < 1.5 || py > texSize.y - 0.5) {
                    v.y = 0.0;
                  }

                  fragColor = vec4(v, 0.0, 0.0);
                }
                );
  }
};
