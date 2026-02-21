#pragma once

#include "PingPongFbo.h"
#include "Shader.h"
#include "ofGraphics.h"

class ApplyTemperatureBuoyancyShader : public Shader {

public:
  void render(PingPongFbo& velocities,
              const PingPongFbo& temperatures,
              float dt,
              float buoyancyStrength,
              float ambientTemperature,
              float temperatureThreshold,
              float gravityForceX,
              float gravityForceY) {
    // Backwards-compatible path: obstacles disabled.
    render(velocities,
           temperatures,
           dt,
           buoyancyStrength,
           ambientTemperature,
           temperatureThreshold,
           gravityForceX,
           gravityForceY,
           velocities.getSource().getTexture(),
           false,
           0.5f,
           false);
  }

  void render(PingPongFbo& velocities,
              const PingPongFbo& temperatures,
              float dt,
              float buoyancyStrength,
              float ambientTemperature,
              float temperatureThreshold,
              float gravityForceX,
              float gravityForceY,
              const ofTexture& obstacles,
              bool obstaclesEnabled,
              float obstacleThreshold,
              bool obstacleInvert) {
    if (buoyancyStrength == 0.0f) return;

    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);

    velocities.getTarget().begin();
    {
      shader.begin();
      shader.setUniform1f("dt", dt);
      shader.setUniform1f("buoyancyStrength", buoyancyStrength);
      shader.setUniform1f("ambientTemperature", ambientTemperature);
      shader.setUniform1f("temperatureThreshold", temperatureThreshold);
      shader.setUniform2f("gravityForce", gravityForceX, gravityForceY);
      shader.setUniformTexture("temperatures", temperatures.getSource().getTexture(), 1);
      shader.setUniformTexture("obstacles", obstacles, 2);
      shader.setUniform1i("obstaclesEnabled", obstaclesEnabled ? 1 : 0);
      shader.setUniform1f("obstacleThreshold", obstacleThreshold);
      shader.setUniform1i("obstacleInvert", obstacleInvert ? 1 : 0);
      velocities.getSource().draw(0, 0);
      shader.end();
    }
    velocities.getTarget().end();
    velocities.swap();

    ofPopStyle();
  }

protected:
  std::string getFragmentShader() override {
    return GLSL(
                 uniform sampler2D tex0; // velocities
                 uniform sampler2D temperatures;
                 uniform sampler2D obstacles;
                 uniform int obstaclesEnabled;
                 uniform float obstacleThreshold;
                 uniform int obstacleInvert;
                 uniform float dt;
                 uniform float buoyancyStrength;
                 uniform float ambientTemperature;
                 uniform float temperatureThreshold;
                 uniform vec2 gravityForce;
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

                 float signedThreshold(float v, float threshold) {
                   float s = (v < 0.0) ? -1.0 : 1.0;
                   float mag = max(abs(v) - threshold, 0.0);
                   return s * mag;
                 }

                 void main() {
                   vec2 uv = texCoordVarying.xy;

                   if (obstacleSolid(uv) > 0.5) {
                     fragColor = vec4(0.0);
                     return;
                   }

                   vec2 velocity = texture(tex0, uv).xy;

                   float temperature = texture(temperatures, uv).r;
                   float delta = signedThreshold(temperature - ambientTemperature, temperatureThreshold);

                   vec2 vNew = velocity + dt * buoyancyStrength * delta * gravityForce;
                   fragColor = vec4(vNew, 0.0, 0.0);
                 }
    );
  }
};
