#pragma once

#include "Shader.h"
#include "ofGraphics.h"

class AddRadialImpulseShader : public Shader {

public:
  // Apply a radial + swirl velocity impulse into the velocities ping-pong FBO.
  // centerPx/radiusPx are in pixel units of the velocity field.
  // radialVelocityPx/swirlVelocityPx are in pixels-per-step (converted internally to UV velocity).
  void render(PingPongFbo& velocities,
              glm::vec2 centerPx,
              float radiusPx,
              float radialVelocityPx,
              float swirlVelocityPx,
              float dt) {
    const auto size = glm::vec2(velocities.getWidth(), velocities.getHeight());
    const glm::vec2 centerUv = centerPx / size;
    const float minDim = std::min(size.x, size.y);
    const float invMinDim = 1.0f / std::max(1.0f, minDim);
    const float radiusUv = radiusPx * invMinDim;

    // Convert from pixels of desired displacement per step to UV velocity.
    // The advect pass uses: fromUv = uv - dt * velocityUv
    // so to achieve a displacement of `dispUv` per step we need velocityUv = dispUv / dt.
    const float dtSafe = std::max(dt, 1.0e-6f);
    const float radialStrengthUv = (radialVelocityPx * invMinDim) / dtSafe;
    const float swirlStrengthUv = (swirlVelocityPx * invMinDim) / dtSafe;

    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
 
    velocities.getTarget().begin();
    shader.begin();
    shader.setUniformTexture("tex0", velocities.getSource().getTexture(), 0);
    shader.setUniform2f("center", centerUv);
    shader.setUniform1f("radius", radiusUv);
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

  // Backwards-compatible overload (radial-only).
  void render(PingPongFbo& velocities, glm::vec2 centerPx, float radiusPx, float radialVelocityPx, float dt) {
    render(velocities, centerPx, radiusPx, radialVelocityPx, 0.0f, dt);
  }

protected:
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0; // previous velocities
                uniform vec2 center;     // UV space
                 uniform float radius;    // UV space
                 uniform float radialStrength;
                 uniform float swirlStrength;
                 uniform float dt;
                uniform float maxDisp; // UV units (domain lengths)
                in vec2 texCoordVarying;
                out vec4 fragColor;

                void main() {
                  vec2 uv = texCoordVarying.xy;
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

                   vec2 impulse = (dir * radialStrength + tangent * swirlStrength) * fade;

                   vec2 vNew = oldV + impulse;

                  // Clamp by displacement per step (CFL-style): dt * |v| should not move too many cells.
                  // This makes stability much more resolution- and dt-independent.
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
