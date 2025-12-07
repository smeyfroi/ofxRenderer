#pragma once

#include "Shader.h"

class ApplyBouyancyShader : public Shader {

public:
  void render(PingPongFbo& velocities, PingPongFbo& temperatures, PingPongFbo& values, float dt, float ambientTemperature, float smokeBouyancy, float smokeWeight, float gravityForceX, float gravityForceY) {
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    shader.begin();
    shader.setUniform1f("dt", dt);
    shader.setUniform1f("ambientTemperature", ambientTemperature);
    shader.setUniform1f("smokeBouyancy", smokeBouyancy);
    shader.setUniform1f("smokeWeight", smokeWeight);
    shader.setUniform1f("gravityForceX", gravityForceX);
    shader.setUniform1f("gravityForceY", gravityForceY);
    shader.setUniformTexture("temperatures", temperatures.getSource(), 1);
    shader.setUniformTexture("values", values.getSource(), 2);
    velocities.getTarget().begin();
    velocities.getSource().draw(0, 0);
    velocities.getTarget().end();
    velocities.swap();
    shader.end();
    ofPopStyle();
  }
  
  static ofParameter<float> createAmbientTemperatureParameter(float value=0.0) {
    return ofParameter<float> { "ambientTemperature", value, 0.0, 100.0 };
  }

  static ofParameter<float> createSmokeBouyancyParameter(float value=1.0) {
    return ofParameter<float> { "smokeBouyancy", value, 0.0, 10.0 };
  }

  static ofParameter<float> createSmokeWeightParameter(float value=0.05) {
    return ofParameter<float> { "smokeWeight", value, 0.0, 1.0 };
  }

  static ofParameter<float> createGravityForceXParameter(float value=0.0) {
    return ofParameter<float> { "gravityForceX", value, -5.0, 5.0 };
  }

  static ofParameter<float> createGravityForceYParameter(float value=-0.98) {
    return ofParameter<float> { "gravityForceY", value, -5.0, 5.0 };
  }

protected:
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
                    gl_FragColor.xy += 10.0;

//                    vec4 value = texture2D(values, xy);
//                    float density = value.a;
////                    float density = (value.r + value.g + value.b) * value.a / 3.0;
//                    gl_FragColor.xy += (dt * (temperature - ambientTemperature) * smokeBouyancy - density * smokeWeight) * vec2(gravityForceX, gravityForceY);
                  }
                }
                );
  }
};
