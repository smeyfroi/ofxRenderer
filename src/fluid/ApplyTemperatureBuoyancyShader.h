#pragma once

#include "../PingPongFbo.h"
#include "../Shader.h"
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
                 uniform float dt;
                 uniform float buoyancyStrength;
                 uniform float ambientTemperature;
                 uniform float temperatureThreshold;
                 uniform vec2 gravityForce;
                 in vec2 texCoordVarying;
                 out vec4 fragColor;

                 float signedThreshold(float v, float threshold) {
                   float s = (v < 0.0) ? -1.0 : 1.0;
                   float mag = max(abs(v) - threshold, 0.0);
                   return s * mag;
                 }

                 void main() {
                   vec2 uv = texCoordVarying.xy;
                   vec2 velocity = texture(tex0, uv).xy;

                   float temperature = texture(temperatures, uv).r;
                   float delta = signedThreshold(temperature - ambientTemperature, temperatureThreshold);

                   vec2 vNew = velocity + dt * buoyancyStrength * delta * gravityForce;
                   fragColor = vec4(vNew, 0.0, 0.0);
                 }
    );
  }
};
