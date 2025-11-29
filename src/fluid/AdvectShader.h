#pragma once

#include "Shader.h"
#include "UnitQuadMesh.h"
#include "ofUtils.h"

class AdvectShader : public Shader {

public:
  void render(PingPongFbo& values, const ofTexture& velocities, float dt, float dissipation) {
    values.getTarget().begin();
    shader.begin();
    {
      shader.setUniformTexture("tex0", values.getSource().getTexture(), 1);
      shader.setUniformTexture("velocities", velocities, 2);
      shader.setUniform1f("dt", dt);
      shader.setUniform1f("dissipation", dissipation);
      quadMesh.draw({ 0.0, 0.0 }, { values.getWidth(), values.getHeight() });
    }
    shader.end();
    values.getTarget().end();
    values.swap();
  }
  
  static ofParameter<float> createDissipationParameter(const std::string& prefix, float value=0.996) {
    return ofParameter<float> { prefix+"Dissipation", value, 0.995, 0.9999 };
  }

protected:
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0; // previous values
                uniform sampler2D velocities;
//                uniform sampler2D obstacleDensities;
                uniform float dt;
                uniform float dissipation;
                in vec2 texCoordVarying;
                out vec4 fragColor;
                
                void main() {
                  vec2 xy = texCoordVarying.xy;
                  vec2 velocity = texture(velocities, xy).xy;
                  vec2 fromXy= xy - dt*velocity;
//                  float obstacleDensity = 1.0 - texture2D(obstacleDensities, xy).x;
//                  gl_FragColor = obstacleDensity * dissipation * texture2D(tex0, fromXy);
                  fragColor = dissipation * texture(tex0, fromXy);
//                  fragColor = texture(tex0, xy);
                }
                );
  }
  
private:
  UnitQuadMesh quadMesh;
};
