#pragma once

#include <algorithm>

#include "Shader.h"

class ApplyVorticityForceShader : public Shader {

public:
  void render(PingPongFbo& velocities_, ofFbo& curls_, float vorticityStrength_, float dt_) {
    // Backwards-compatible path: obstacles disabled.
    render(velocities_, curls_, vorticityStrength_, dt_, velocities_.getSource().getTexture(), false, 0.5f, false);
  }

  void render(PingPongFbo& velocities_,
              ofFbo& curls_,
              float vorticityStrength_,
              float dt_,
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
      shader.setUniformTexture("curls", curls_.getTexture(), 1);
      shader.setUniformTexture("obstacles", obstacles, 2);
      shader.setUniform1i("obstaclesEnabled", obstaclesEnabled ? 1 : 0);
      shader.setUniform1f("obstacleThreshold", obstacleThreshold);
      shader.setUniform1i("obstacleInvert", obstacleInvert ? 1 : 0);
      shader.setUniform2f("texSize", texSize);
      shader.setUniform2f("halfInvCell", 0.5f * texSize);
      shader.setUniform1f("vorticityStrength", vorticityStrength_);
      shader.setUniform1f("dt", dt_);
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
                uniform sampler2D curls;
                uniform sampler2D obstacles;
                uniform int obstaclesEnabled;
                uniform float obstacleThreshold;
                uniform int obstacleInvert;
                uniform vec2 texSize;
                uniform vec2 halfInvCell;
                uniform float dt;
                uniform float vorticityStrength;
                in vec2 texCoordVarying;
                out vec4 fragColor;

                float obstacleMask(vec2 uv) {
                  // Sample obstacles at texel centers to avoid linear-filter bleed at boundaries.
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

                void main(){
                  vec2 xy = texCoordVarying.xy;

                  if (obstacleSolid(xy) > 0.5) {
                    fragColor = vec4(0.0);
                    return;
                  }

                  vec2 oldV = texture(tex0, xy).xy;

                  vec2 off = vec2(1.0, 0.0) / texSize;
                  float curlN = abs(texture(curls, xy + off.yx).x);
                  float curlS = abs(texture(curls, xy - off.yx).x);
                  float curlE = abs(texture(curls, xy + off.xy).x);
                  float curlW = abs(texture(curls, xy - off.xy).x);
                  float curlC = texture(curls, xy).x;

                  vec2 grad = vec2(curlE - curlW, curlN - curlS) * halfInvCell;

                  float gradLen = length(grad);
                  vec2 N = (gradLen > 1e-6) ? (grad / gradLen) : vec2(0.0);

                  vec2 fvc = vec2(N.y, -N.x) * curlC * dt * vorticityStrength;

                  fragColor = vec4(oldV + fvc, 0.0, 0.0);
                }
                );
  }
};
