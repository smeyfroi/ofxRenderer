#pragma once

#include "Shader.h"

class JacobiShader : public Shader {

public:
  void render(PingPongFbo& x, const ofTexture& b, float dt, float alpha, float rBeta, int iterations) {
    // Backwards-compatible path: obstacles disabled.
    render(x, b, dt, alpha, rBeta, iterations, x.getSource().getTexture(), false, 0.5f, false);
  }

  void render(PingPongFbo& x,
              const ofTexture& b,
              float dt,
              float alpha,
              float rBeta,
              int iterations,
              const ofTexture& obstacles,
              bool obstaclesEnabled,
              float obstacleThreshold,
              bool obstacleInvert) {
    (void)dt; // kept for API consistency (pressure/diffusion are dt-dependent at higher layers)

    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    shader.begin();
    shader.setUniformTexture("b", b, 1);
    shader.setUniformTexture("obstacles", obstacles, 2);
    shader.setUniform1i("obstaclesEnabled", obstaclesEnabled ? 1 : 0);
    shader.setUniform1f("obstacleThreshold", obstacleThreshold);
    shader.setUniform1i("obstacleInvert", obstacleInvert ? 1 : 0);
    shader.setUniform2f("texSize", glm::vec2(x.getSource().getWidth(), x.getSource().getHeight()));
    shader.setUniform1f("alpha", alpha);
    shader.setUniform1f("rBeta", rBeta);
    for (int i = 0; i < iterations; i++) {
      x.getTarget().begin();
      x.getSource().draw(0, 0);
      x.getTarget().end();
      x.swap();
    }
    shader.end();
    ofPopStyle();
  }

  static ofParameter<int> createIterationsParameter(const std::string& prefix, int value=20) {
    return ofParameter<int> { prefix+"Iterations", value, 0, 30 };
  }

protected:
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0; // current values
                uniform sampler2D b;
                uniform sampler2D obstacles;
                uniform int obstaclesEnabled;
                uniform float obstacleThreshold;
                uniform int obstacleInvert;
                uniform vec2 texSize;
                uniform float alpha;
                uniform float rBeta;
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

                  vec4 xC = texture(tex0, xy);

                  vec4 xN = texture(tex0, xy + off.yx);
                  vec4 xS = texture(tex0, xy - off.yx);
                  vec4 xE = texture(tex0, xy + off.xy);
                  vec4 xW = texture(tex0, xy - off.xy);

                  if (obstacleSolid(xy + off.yx) > 0.5) xN = xC;
                  if (obstacleSolid(xy - off.yx) > 0.5) xS = xC;
                  if (obstacleSolid(xy + off.xy) > 0.5) xE = xC;
                  if (obstacleSolid(xy - off.xy) > 0.5) xW = xC;

                  vec4 bC = texture(b, xy);

                  fragColor = (xW + xE + xS + xN + alpha * bC) * rBeta;
                }
                );
  }
};
