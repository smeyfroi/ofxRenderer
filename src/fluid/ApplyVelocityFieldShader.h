#pragma once

#include "Shader.h"
#include "ofGraphics.h"

// Inject an external velocity field texture into a velocity ping-pong buffer.
// The velocityField texture is expected to store XY velocities in RG.
// fieldScale is a linear multiplier applied to the sampled field.
class ApplyVelocityFieldShader : public Shader {

public:
  void render(PingPongFbo& velocities, const ofTexture& velocityField, float fieldScale) {
    render(velocities,
           velocityField,
           fieldScale,
           velocities.getSource().getTexture(),
           false,
           0.5f,
           false);
  }

  void render(PingPongFbo& velocities,
              const ofTexture& velocityField,
              float fieldScale,
              const ofTexture& obstacles,
              bool obstaclesEnabled,
              float obstacleThreshold,
              bool obstacleInvert) {
    if (fieldScale == 0.0f) return;

    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);

    velocities.getTarget().begin();
    shader.begin();
    shader.setUniformTexture("tex0", velocities.getSource().getTexture(), 0);
    shader.setUniformTexture("velocityField", velocityField, 1);
    shader.setUniformTexture("obstacles", obstacles, 2);
    shader.setUniform1i("obstaclesEnabled", obstaclesEnabled ? 1 : 0);
    shader.setUniform1f("obstacleThreshold", obstacleThreshold);
    shader.setUniform1i("obstacleInvert", obstacleInvert ? 1 : 0);
    shader.setUniform1f("fieldScale", fieldScale);
    velocities.getSource().draw(0, 0);
    shader.end();
    velocities.getTarget().end();
    velocities.swap();

    ofPopStyle();
  }

protected:
  std::string getFragmentShader() override {
    return GLSL(
      uniform sampler2D tex0; // previous velocities
      uniform sampler2D velocityField;
      uniform sampler2D obstacles;
      uniform int obstaclesEnabled;
      uniform float obstacleThreshold;
      uniform int obstacleInvert;
      uniform float fieldScale;
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

      void main() {
        vec2 uv = texCoordVarying.xy;
        vec2 v = texture(tex0, uv).xy;

        if (obstacleSolid(uv) > 0.5) {
          fragColor = vec4(v, 0.0, 1.0);
          return;
        }

        vec2 f = texture(velocityField, uv).xy;
        fragColor = vec4(v + fieldScale * f, 0.0, 1.0);
      }
    );
  }
};
