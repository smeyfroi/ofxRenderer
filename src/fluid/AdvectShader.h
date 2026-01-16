#pragma once

#include "Shader.h"
#include "ofUtils.h"

class AdvectShader : public Shader {

public:
  void render(PingPongFbo& values, const ofTexture& velocities, float dt, float dissipation) {
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    values.getTarget().begin();
    shader.begin();
    {
      shader.setUniformTexture("tex0", values.getSource().getTexture(), 1);
      shader.setUniformTexture("velocities", velocities, 2);
      shader.setUniform2f("texSize", glm::vec2(values.getWidth(), values.getHeight()));
      shader.setUniform1f("dt", dt);
      shader.setUniform1f("dissipation", dissipation);
      values.getSource().draw(0, 0);
    }
    shader.end();
    values.getTarget().end();
    values.swap();
    ofPopStyle();
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
                uniform vec2 texSize;
                uniform float dt;
                uniform float dissipation;
                out vec4 fragColor;

                void main() {
                  vec2 xy = gl_FragCoord.xy / texSize;
                  xy.y = 1.0 - xy.y;

                  vec2 velocity = texture(velocities, xy).xy;
                  vec2 fromXy = xy - dt * velocity;
//                  float obstacleDensity = 1.0 - texture2D(obstacleDensities, xy).x;
//                  gl_FragColor = obstacleDensity * dissipation * texture2D(tex0, fromXy);
                  fragColor = dissipation * texture(tex0, fromXy);
//                  fragColor = texture(tex0, xy);
                }
                );
  }
};
