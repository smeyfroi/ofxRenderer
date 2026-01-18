#pragma once

#include "Shader.h"
#include "ofGraphics.h"

class AddRadialImpulseShader : public Shader {

public:
  // Apply a velocity impulse into the velocities ping-pong FBO.
  // centerPx/radiusPx are in pixel units of the velocity field.
  // addVelocityPx is pixels of desired displacement per step (vector/drag impulse).
  // radialVelocityPx/swirlVelocityPx are pixels of desired displacement per step (radial + tangential).
  void render(PingPongFbo& velocities,
              glm::vec2 centerPx,
              float radiusPx,
              glm::vec2 addVelocityPx,
              float radialVelocityPx,
              float swirlVelocityPx,
              float dt) {
    // Backwards-compatible path: obstacles disabled.
    render(velocities,
           centerPx,
           radiusPx,
           addVelocityPx,
           radialVelocityPx,
           swirlVelocityPx,
           dt,
           velocities.getSource().getTexture(),
           false,
           0.5f,
           false);
  }

  void render(PingPongFbo& velocities,
              glm::vec2 centerPx,
              float radiusPx,
              glm::vec2 addVelocityPx,
              float radialVelocityPx,
              float swirlVelocityPx,
              float dt,
              const ofTexture& obstacles,
              bool obstaclesEnabled,
              float obstacleThreshold,
              bool obstacleInvert) {
    const auto size = glm::vec2(velocities.getWidth(), velocities.getHeight());
    const glm::vec2 centerUv = centerPx / size;
    const float minDim = std::min(size.x, size.y);
    const float invMinDim = 1.0f / std::max(1.0f, minDim);
    const float radiusUv = radiusPx * invMinDim;

    const float dtSafe = std::max(dt, 1.0e-6f);

    const glm::vec2 addVelocityUv = (addVelocityPx / size) / dtSafe;
    const float radialStrengthUv = (radialVelocityPx * invMinDim) / dtSafe;
    const float swirlStrengthUv = (swirlVelocityPx * invMinDim) / dtSafe;

    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);

    velocities.getTarget().begin();
    shader.begin();
    shader.setUniformTexture("tex0", velocities.getSource().getTexture(), 0);
    shader.setUniformTexture("obstacles", obstacles, 1);
    shader.setUniform1i("obstaclesEnabled", obstaclesEnabled ? 1 : 0);
    shader.setUniform1f("obstacleThreshold", obstacleThreshold);
    shader.setUniform1i("obstacleInvert", obstacleInvert ? 1 : 0);
    shader.setUniform2f("center", centerUv);
    shader.setUniform1f("radius", radiusUv);
    shader.setUniform2f("addVelocity", addVelocityUv);
    shader.setUniform1f("radialStrength", radialStrengthUv);
    shader.setUniform1f("swirlStrength", swirlStrengthUv);
    shader.setUniform1f("dt", dt);

    const float dx = invMinDim;
    const float maxDisp = 4.0f * dx;
    shader.setUniform1f("maxDisp", maxDisp);

    velocities.getSource().draw(0, 0);
    shader.end();
    velocities.getTarget().end();
    velocities.swap();

    ofPopStyle();
  }

  // Backwards-compatible overload (radial + swirl only).
  void render(PingPongFbo& velocities, glm::vec2 centerPx, float radiusPx, float radialVelocityPx, float swirlVelocityPx, float dt) {
    render(velocities, centerPx, radiusPx, glm::vec2(0.0f), radialVelocityPx, swirlVelocityPx, dt);
  }

  // Backwards-compatible overload (radial-only).
  void render(PingPongFbo& velocities, glm::vec2 centerPx, float radiusPx, float radialVelocityPx, float dt) {
    render(velocities, centerPx, radiusPx, glm::vec2(0.0f), radialVelocityPx, 0.0f, dt);
  }

protected:
  std::string getFragmentShader() override {
    return GLSL(
                 uniform sampler2D tex0; // previous velocities
                 uniform sampler2D obstacles;
                 uniform int obstaclesEnabled;
                 uniform float obstacleThreshold;
                 uniform int obstacleInvert;
                 uniform vec2 center;     // UV space
                 uniform float radius;    // UV space
                 uniform vec2 addVelocity;
                 uniform float radialStrength;
                 uniform float swirlStrength;
                 uniform float dt;
                 uniform float maxDisp; // UV units (domain lengths)
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

                   if (obstacleSolid(uv) > 0.5) {
                     fragColor = vec4(0.0);
                     return;
                   }

                   vec2 oldV = texture(tex0, uv).xy;

                   vec2 d = uv - center;
                   float dist = length(d);
                   if (dist >= radius) {
                     fragColor = vec4(oldV, 0.0, 0.0);
                     return;
                   }

                   float t = clamp(dist / radius, 0.0, 1.0);
                   float fade = 1.0 - smoothstep(0.0, 1.0, t);

                   vec2 dir = dist > 0.0 ? normalize(d) : vec2(0.0);
                   vec2 tangent = vec2(-dir.y, dir.x);

                   vec2 impulse = (addVelocity + dir * radialStrength + tangent * swirlStrength) * fade;

                   vec2 vNew = oldV + impulse;

                   float speed = length(vNew);
                   float disp = speed * dt;
                   if (disp > maxDisp && disp > 0.0) {
                     vNew *= maxDisp / disp;
                   }

                   fragColor = vec4(vNew, 0.0, 0.0);
                 }
                 );
  }
};
