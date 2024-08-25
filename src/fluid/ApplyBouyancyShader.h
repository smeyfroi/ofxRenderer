#pragma once

#include "Shader.h"

class ApplyBouyancyShader : public Shader {

public:
  void render(PingPongFbo& velocities, PingPongFbo& temperatures, PingPongFbo& values, float dt) {
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    shader.begin();
    setupShaders();
    shader.setUniform1f("dt", dt);
    shader.setUniformTexture("temperatures", temperatures.getSource(), 1);
    shader.setUniformTexture("values", values.getSource(), 2);

    velocities.getTarget().begin();
    velocities.getSource().draw(0, 0);
    velocities.getTarget().end();
    velocities.swap();
    shader.end();
  }
  
  ofParameterGroup& getParameterGroup() {
    if (parameters.size() == 0) {
      parameters.add(ambientTemperatureParameter);
      parameters.add(smokeBouyancyParameter);
      parameters.add(smokeWeightParameter);
      parameters.add(gravityForceXParameter);
      parameters.add(gravityForceYParameter);
    }
    return parameters;
  }

protected:
  void setupShaders() override {
    shader.setUniform1f("ambientTemperature", ambientTemperatureParameter);
    shader.setUniform1f("smokeBouyancy", smokeBouyancyParameter);
    shader.setUniform1f("smokeWeight", smokeWeightParameter);
    shader.setUniform1f("gravityForceX", gravityForceXParameter);
    shader.setUniform1f("gravityForceY", gravityForceYParameter);
  }
  
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0; // velocities
                uniform sampler2D temperatures;
                uniform sampler2D values; // current values (density)
                uniform float ambientTemperature;
                uniform float smokeBouyancy;
                uniform float smokeWeight;
                uniform float gravityForceX;
                uniform float gravityForceY;
                uniform float dt;

                void main() {
                  vec2 xy = gl_TexCoord[0].st;

                  float temperature = texture2D(temperatures, xy).r;
                  vec2 velocity = texture2D(tex0, xy).xy;
                  
                  gl_FragColor.xy = velocity;
                  
                  if (temperature > ambientTemperature) {
                    float value = texture2D(values, xy);
                    float density = (value.x + value.y + value.z) / 3.0;
                    gl_FragColor.xy += (dt * (temperature - ambientTemperature) * smokeBouyancy - density * smokeWeight) * vec2(gravityForceX, gravityForceY);
                  }
                }
                );
  }

private:
  ofParameterGroup parameters { "Bouyancy" };
  ofParameter<float> ambientTemperatureParameter {"ambientTemperature", 0.0, 0.0, 1.0 };
  ofParameter<float> smokeBouyancyParameter {"smokeBouyancy", 1.0, 0.0, 10.0 };
  ofParameter<float> smokeWeightParameter {"smokeWeight", 0.05, 0.0, 1.0 };
  ofParameter<float> gravityForceXParameter {"gravityForceX", 0.0, -5.0, 5.0 };
  ofParameter<float> gravityForceYParameter {"gravityForceY", -0.98, -5.0, 5.0 };
};
