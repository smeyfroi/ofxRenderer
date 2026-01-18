#pragma once

#include "Shader.h"

class SubtractDivergenceShader : public Shader {

public:
  void render(PingPongFbo& velocities_, ofFbo& pressures_) {
    // Backwards-compatible path: obstacles disabled.
    render(velocities_, pressures_, velocities_.getSource().getTexture(), false, 0.5f, false);
  }

  void render(PingPongFbo& velocities_,
              ofFbo& pressures_,
              const ofTexture& obstacles,
              bool obstaclesEnabled,
              float obstacleThreshold,
              bool obstacleInvert) {
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    velocities_.getTarget().begin();
    {
      shader.begin();
      const auto texSize = glm::vec2(velocities_.getWidth(), velocities_.getHeight());
      shader.setUniformTexture("pressures", pressures_.getTexture(), 1);
      shader.setUniformTexture("obstacles", obstacles, 2);
      shader.setUniform1i("obstaclesEnabled", obstaclesEnabled ? 1 : 0);
      shader.setUniform1f("obstacleThreshold", obstacleThreshold);
      shader.setUniform1i("obstacleInvert", obstacleInvert ? 1 : 0);
      shader.setUniform2f("texSize", texSize);
      shader.setUniform2f("halfInvCell", 0.5f * texSize);
      ofSetColor(255);
      velocities_.getSource().draw(0, 0);
      shader.end();
    }
    velocities_.getTarget().end();
    velocities_.swap();
    ofPopStyle();
  }

protected:
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0; // velocities
                uniform sampler2D pressures;
                uniform sampler2D obstacles;
                uniform int obstaclesEnabled;
                uniform float obstacleThreshold;
                uniform int obstacleInvert;
                uniform vec2 texSize;
                uniform vec2 halfInvCell;
                in vec2 texCoordVarying;
                out vec4 fragColor;

                float obstacleMask(vec2 uv) {
                  vec2 sz = vec2(textureSize(obstacles, 0));
                  vec2 uvQ = (floor(uv * sz) + 0.5) / sz;
                  vec4 o = texture(obstacles, uvQ);
                  float m = max(o.a, dot(o.rgb, vec3(0.333333)));
                  if (obstacleInvert == 1) m = 1.0 - m;
                  return m;
                }

                float obstacleSolid(vec2 uv) {
                  if (obstaclesEnabled == 0) return 0.0;
                  return step(obstacleThreshold, obstacleMask(uv));
                }

                void main() {
                  vec2 xy = texCoordVarying.xy;

                  if (obstacleSolid(xy) > 0.5) {
                    fragColor = vec4(0.0);
                    return;
                  }

                  vec2 off = vec2(1.0, 0.0) / texSize;

                  float pC = texture(pressures, xy).r;

                  float pN = (obstacleSolid(xy + off.yx) > 0.5) ? pC : texture(pressures, xy + off.yx).r;
                  float pS = (obstacleSolid(xy - off.yx) > 0.5) ? pC : texture(pressures, xy - off.yx).r;
                  float pE = (obstacleSolid(xy + off.xy) > 0.5) ? pC : texture(pressures, xy + off.xy).r;
                  float pW = (obstacleSolid(xy - off.xy) > 0.5) ? pC : texture(pressures, xy - off.xy).r;

                  vec2 grad = vec2(pE - pW, pN - pS) * halfInvCell;

                  vec2 oldV = texture(tex0, xy).xy;
                  vec2 newV = oldV - grad;

                  fragColor.rg = newV;
                }
                );
  }
};
