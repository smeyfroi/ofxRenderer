#pragma once

#include "Shader.h"

class AdvectShader : public Shader {

public:
  void render(PingPongFbo& values, const ofTexture& velocities, float dt) {
    values.getTarget().begin();
    shader.begin();
    {
      setupShaders();
      shader.setUniformTexture("velocities", velocities, 1);
      shader.setUniform1f("dt", dt);
      values.getSource().draw(0, 0);
    }
    shader.end();
    values.getTarget().end();
    values.swap();
  }
  
  ofParameterGroup& getParameterGroup(const std::string& prefix) {
    if (parameters.size() == 0) {
      parameters.setName(prefix + parameters.getName());
      dissipationParameter.setName(prefix + dissipationParameter.getName());
      parameters.add(dissipationParameter);
    }
    return parameters;
  }

protected:
  void setupShaders() override {
    shader.setUniform1f("dissipation", dissipationParameter);
  }
  
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
  
private:
  ofParameterGroup parameters { "Advection" };
  ofParameter<float> dissipationParameter { "dissipation", 0.996, 0.990, 1.0 };
};
