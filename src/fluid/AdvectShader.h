#pragma once

#include "Shader.h"

class AdvectShader : public Shader {

public:
  void render(PingPongFbo& values, const ofTexture& velocities, float dt, float dissipation) {
    values.getTarget().begin();
    shader.begin();
    {
      shader.setUniformTexture("velocities", velocities, 1);
      shader.setUniform1f("dt", dt);
      shader.setUniform1f("dissipation", dissipation);
      values.getSource().draw(0, 0);
    }
    shader.end();
    values.getTarget().end();
    values.swap();
  }
  
  static ofParameter<float> createDissipationParameter(const std::string& prefix, float value=0.996) {
    return ofParameter<float> { prefix+"dissipation", value, 0.990, 1.0 };
  }

protected:
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0; // previous values
                uniform sampler2D velocities;
//                uniform sampler2D obstacleDensities;
                uniform float dt;
                uniform float dissipation;
                
                void main() {
                  vec2 xy = gl_TexCoord[0].st;
                  vec2 velocity = texture2D(velocities, xy).xy;
                  vec2 fromXy= xy - dt*velocity;
//                  float obstacleDensity = 1.0 - texture2D(obstacleDensities, xy).x;
//                  gl_FragColor = obstacleDensity * dissipation * texture2D(tex0, fromXy);
                  gl_FragColor = dissipation * texture2D(tex0, fromXy);
                }
                );
  }
};
