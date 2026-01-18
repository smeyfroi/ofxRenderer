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
    // Backwards-compatible path: obstacles disabled.
    render(velocities,
           values,
           dt,
           buoyancyStrength,
           densityScale,
           densityThreshold,
           gravityForceX,
           gravityForceY,
           velocities.getSource().getTexture(),
           false,
           0.5f,
           false);
  }

  void render(PingPongFbo& velocities,
              const PingPongFbo& values,
              float dt,
              float buoyancyStrength,
              float densityScale,
              float densityThreshold,
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
      shader.setUniform1f("densityScale", densityScale);
      shader.setUniform1f("densityThreshold", densityThreshold);
      shader.setUniform2f("gravityForce", gravityForceX, gravityForceY);
      shader.setUniformTexture("values", values.getSource().getTexture(), 1);
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
                 uniform sampler2D obstacles;
                 uniform int obstaclesEnabled;
                 uniform float obstacleThreshold;
                 uniform int obstacleInvert;
                 uniform float dt;
                 uniform float buoyancyStrength;
                 uniform float densityScale;
                 uniform float densityThreshold;
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

                 void main() {
                   vec2 uv = texCoordVarying.xy;

                   if (obstacleSolid(uv) > 0.5) {
                     fragColor = vec4(0.0);
                     return;
                   }

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
