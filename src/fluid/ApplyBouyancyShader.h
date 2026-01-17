#pragma once

#include "Shader.h"
#include "ofGraphics.h"

class ApplyBouyancyShader : public Shader {

public:
  void render(PingPongFbo& velocities,
              const PingPongFbo& values,
              float dt,
              float buoyancyStrength,
              float densityScale,
              float densityThreshold,
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
      shader.setUniform1f("densityScale", densityScale);
      shader.setUniform1f("densityThreshold", densityThreshold);
      shader.setUniform2f("gravityForce", gravityForceX, gravityForceY);
      shader.setUniformTexture("values", values.getSource().getTexture(), 1);
      velocities.getSource().draw(0, 0);
      shader.end();
    }
    velocities.getTarget().end();
    velocities.swap();

    ofPopStyle();
  }

  static ofParameter<float> createBuoyancyStrengthParameter(float value = 0.0f) {
    return ofParameter<float> { "Buoyancy Strength", value, 0.0f, 3.0f };
  }

  static ofParameter<float> createDensityScaleParameter(float value = 1.0f) {
    return ofParameter<float> { "Buoyancy Density Scale", value, 0.0f, 10.0f };
  }

  static ofParameter<float> createDensityThresholdParameter(float value = 0.0f) {
    return ofParameter<float> { "Buoyancy Threshold", value, 0.0f, 1.0f };
  }

  static ofParameter<float> createGravityForceXParameter(float value = 0.0f) {
    return ofParameter<float> { "Gravity Force X", value, -5.0f, 5.0f };
  }

  static ofParameter<float> createGravityForceYParameter(float value = -1.0f) {
    return ofParameter<float> { "Gravity Force Y", value, -5.0f, 5.0f };
  }

protected:
  std::string getFragmentShader() override {
    return GLSL(
                 uniform sampler2D tex0; // velocities
                 uniform sampler2D values;
                 uniform float dt;
                 uniform float buoyancyStrength;
                 uniform float densityScale;
                 uniform float densityThreshold;
                 uniform vec2 gravityForce;
                 in vec2 texCoordVarying;
                 out vec4 fragColor;

                 void main() {
                   vec2 uv = texCoordVarying.xy;
                   vec2 velocity = texture(tex0, uv).xy;

                   vec4 v = texture(values, uv);
                   float density = max(v.a, dot(v.rgb, vec3(0.333333)));
                   density = max(0.0, densityScale * (density - densityThreshold));

                   vec2 vNew = velocity + dt * buoyancyStrength * density * gravityForce;
                   fragColor = vec4(vNew, 0.0, 0.0);
                 }
    );
  }
};
