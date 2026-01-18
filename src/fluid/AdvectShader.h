#pragma once

#include "Shader.h"
#include "ofUtils.h"

class AdvectShader : public Shader {

public:
  void render(PingPongFbo& values, const ofTexture& velocities, float dt, float dissipation, float maxValue = 0.0f) {
    // Backwards-compatible path: obstacles disabled.
    render(values,
           velocities,
           dt,
           dissipation,
           maxValue,
           values.getSource().getTexture(),
           false,
           0.5f,
           false);
  }

  void render(PingPongFbo& values,
              const ofTexture& velocities,
              float dt,
              float dissipation,
              float maxValue,
              const ofTexture& obstacles,
              bool obstaclesEnabled,
              float obstacleThreshold,
              bool obstacleInvert) {
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    values.getTarget().begin();
    shader.begin();
    {
      shader.setUniformTexture("tex0", values.getSource().getTexture(), 1);
      shader.setUniformTexture("velocities", velocities, 2);
      shader.setUniformTexture("obstacles", obstacles, 3);
      shader.setUniform1i("obstaclesEnabled", obstaclesEnabled ? 1 : 0);
      shader.setUniform1f("obstacleThreshold", obstacleThreshold);
      shader.setUniform1i("obstacleInvert", obstacleInvert ? 1 : 0);
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
                uniform sampler2D obstacles;
                uniform int obstaclesEnabled;
                uniform float obstacleThreshold;
                uniform int obstacleInvert;
                uniform float dt;
                uniform float dissipation;
                uniform float maxValue;
                in vec2 texCoordVarying;
                out vec4 fragColor;

                float obstacleMask(vec2 uv) {
                  // Sample obstacles at texel centers to avoid linear-filter "bleed" at boundaries.
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

                vec4 sampleMasked(sampler2D tex, vec2 uv) {
                  vec2 sz = vec2(textureSize(tex, 0));

                  // Reconstruct bilinear footprint using texel centers so we can
                  // suppress contributions from solid obstacle texels.
                  vec2 uvPx = uv * sz - 0.5;
                  vec2 base = floor(uvPx);
                  vec2 f = uvPx - base;

                  vec2 uv00 = (base + vec2(0.5, 0.5)) / sz;
                  vec2 uv10 = (base + vec2(1.5, 0.5)) / sz;
                  vec2 uv01 = (base + vec2(0.5, 1.5)) / sz;
                  vec2 uv11 = (base + vec2(1.5, 1.5)) / sz;

                  float m00 = 1.0 - obstacleSolid(uv00);
                  float m10 = 1.0 - obstacleSolid(uv10);
                  float m01 = 1.0 - obstacleSolid(uv01);
                  float m11 = 1.0 - obstacleSolid(uv11);

                  float w00 = (1.0 - f.x) * (1.0 - f.y) * m00;
                  float w10 = f.x * (1.0 - f.y) * m10;
                  float w01 = (1.0 - f.x) * f.y * m01;
                  float w11 = f.x * f.y * m11;

                  float wSum = w00 + w10 + w01 + w11;
                  if (wSum <= 1.0e-6) return vec4(0.0);

                  vec4 c00 = texture(tex, uv00);
                  vec4 c10 = texture(tex, uv10);
                  vec4 c01 = texture(tex, uv01);
                  vec4 c11 = texture(tex, uv11);

                  return (c00 * w00 + c10 * w10 + c01 * w01 + c11 * w11) / wSum;
                }

                void main() {
                  vec2 xy = texCoordVarying.xy;

                  if (obstacleSolid(xy) > 0.5) {
                    fragColor = vec4(0.0);
                    return;
                  }

                  vec2 velocity = sampleMasked(velocities, xy).xy;
                  vec2 fromXy = xy - dt * velocity;
                  if (obstacleSolid(fromXy) > 0.5) {
                    fromXy = xy;
                  }

                  fragColor = dissipation * sampleMasked(tex0, fromXy);

                  if (maxValue > 0.0) {
                    fragColor = clamp(fragColor, vec4(0.0), vec4(maxValue));
                  }
                }
                );
  }
};
