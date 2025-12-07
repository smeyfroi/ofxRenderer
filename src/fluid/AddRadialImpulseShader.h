#pragma once

#include "Shader.h"
#include "ofGraphics.h"
#include "UnitQuadMesh.h"

class AddRadialImpulseShader : public Shader {

public:
  // Apply a radial velocity impulse into the velocities ping-pong FBO.
  // centerPx and radiusPx are in pixel units of the velocity field.
  void render(PingPongFbo &velocities,
              glm::vec2 centerPx,
              float radiusPx,
              float strength,
              float dt) {
    auto size = glm::vec2(velocities.getWidth(), velocities.getHeight());
    glm::vec2 centerUv = centerPx / size;
    float radiusUv = radiusPx / glm::min(size.x, size.y);

    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
 
    velocities.getTarget().begin();
    shader.begin();
    shader.setUniformTexture("tex0", velocities.getSource().getTexture(), 0);
    shader.setUniform2f("center", centerUv);
    shader.setUniform1f("radius", radiusUv);
    shader.setUniform1f("strength", strength);
    shader.setUniform1f("dt", dt);
    quadMesh.draw({0.0f, 0.0f}, size);
    shader.end();
    velocities.getTarget().end();
    velocities.swap();

    ofPopStyle();

  }

protected:
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0; // previous velocities
                uniform vec2 center;     // UV space
                uniform float radius;    // UV space
                uniform float strength;
                uniform float dt;
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
                  vec2 impulse = dir * strength * fade * dt;

                  vec2 vNew = oldV + impulse;

                  // Clamp to keep velocities bounded.
                  float maxSpeed = 5.0; // tune as needed
                  float speed = length(vNew);
                  if (speed > maxSpeed) {
                    vNew *= maxSpeed / speed;
                  }

                  fragColor = vec4(vNew, 0.0, 0.0);
                }
                );
  }

private:
  UnitQuadMesh quadMesh;
};
