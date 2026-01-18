#pragma once

#include "Renderer.h"

class DivergenceRenderer : public Renderer {

public:
  void render(const ofBaseDraws& velocities_) override {
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    fbo.begin();
    shader.begin();
    {
      const auto texSize = glm::vec2(fbo.getWidth(), fbo.getHeight());
      shader.setUniform2f("texSize", texSize);
      shader.setUniform2f("halfInvCell", 0.5f * texSize);
      shader.setUniform1i("obstaclesEnabled", 0);
      velocities_.draw(0, 0, fbo.getWidth(), fbo.getHeight());
    }
    shader.end();
    fbo.end();
    ofPopStyle();
  }

  void renderWithObstacles(const ofBaseDraws& velocities_,
                           const ofTexture& obstacles,
                           float obstacleThreshold,
                           bool obstacleInvert) {
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    fbo.begin();
    shader.begin();
    {
      const auto texSize = glm::vec2(fbo.getWidth(), fbo.getHeight());
      shader.setUniform2f("texSize", texSize);
      shader.setUniform2f("halfInvCell", 0.5f * texSize);
      shader.setUniformTexture("obstacles", obstacles, 1);
      shader.setUniform1i("obstaclesEnabled", 1);
      shader.setUniform1f("obstacleThreshold", obstacleThreshold);
      shader.setUniform1i("obstacleInvert", obstacleInvert ? 1 : 0);
      velocities_.draw(0, 0, fbo.getWidth(), fbo.getHeight());
    }
    shader.end();
    fbo.end();
    ofPopStyle();
  }

protected:
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0; // velocities
                uniform sampler2D obstacles;
                uniform int obstaclesEnabled;
                uniform float obstacleThreshold;
                uniform int obstacleInvert;
                uniform vec2 texSize;
                uniform vec2 halfInvCell;
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
                    fragColor.r = 0.0;
                    return;
                  }

                  vec2 off = vec2(1.0, 0.0) / texSize;

                  float solidN = obstacleSolid(xy + off.yx);
                  float solidS = obstacleSolid(xy - off.yx);
                  float solidE = obstacleSolid(xy + off.xy);
                  float solidW = obstacleSolid(xy - off.xy);

                  vec2 vN = (solidN > 0.5) ? vec2(0.0) : texture(tex0, xy + off.yx).xy;
                  vec2 vS = (solidS > 0.5) ? vec2(0.0) : texture(tex0, xy - off.yx).xy;
                  vec2 vE = (solidE > 0.5) ? vec2(0.0) : texture(tex0, xy + off.xy).xy;
                  vec2 vW = (solidW > 0.5) ? vec2(0.0) : texture(tex0, xy - off.xy).xy;

                  fragColor.r = (vE.x - vW.x) * halfInvCell.x + (vN.y - vS.y) * halfInvCell.y;
                }
                );
  }

  GLint getInternalFormat() override { return GL_R16F; }
};
