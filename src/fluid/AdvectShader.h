#pragma once

#include "Shader.h"
#include "ofUtils.h"

class AdvectShader : public Shader {

public:
  void render(PingPongFbo& values, const ofTexture& velocities, float dt, float dissipation, float maxValue = 0.0f) {
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    values.getTarget().begin();
    shader.begin();
    {
      shader.setUniformTexture("tex0", values.getSource().getTexture(), 1);
      shader.setUniformTexture("velocities", velocities, 2);
      shader.setUniform1f("dt", dt);
      shader.setUniform1f("dissipation", dissipation);
      shader.setUniform1f("maxValue", maxValue);
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
                uniform float dt;
                uniform float dissipation;
                uniform float maxValue;
                in vec2 texCoordVarying;
                out vec4 fragColor;
 
                void main() {
                  vec2 xy = texCoordVarying.xy;

                  vec2 velocity = texture(velocities, xy).xy;
                  vec2 fromXy = xy - dt * velocity;
//                  float obstacleDensity = 1.0 - texture2D(obstacleDensities, xy).x;
//                  gl_FragColor = obstacleDensity * dissipation * texture2D(tex0, fromXy);
                  fragColor = dissipation * texture(tex0, fromXy);

                  if (maxValue > 0.0) {
                    fragColor = clamp(fragColor, vec4(0.0), vec4(maxValue));
                  }
//                  fragColor = texture(tex0, xy);
                }
                );
  }
};
