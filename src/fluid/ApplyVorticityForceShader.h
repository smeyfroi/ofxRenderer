#pragma once

#include "Shader.h"

class ApplyVorticityForceShader : public Shader {
  
public:
  ApplyVorticityForceShader() {}

  void render(PingPongFbo& velocities_, ofFbo& curls_, float vorticityStrength_, float dt_) {
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    velocities_.getTarget().begin();
    {
      shader.begin();
      shader.setUniformTexture("curls", curls_.getTexture(), 1);
      shader.setUniform2f("texSize", glm::vec2(velocities_.getWidth(), velocities_.getHeight()));
      shader.setUniform1f("vorticityStrength", vorticityStrength_);
      shader.setUniform1f("dt", dt_);
      ofSetColor(255);
      velocities_.getSource().draw(0, 0);
      shader.end();
    }
    velocities_.getTarget().end();
    velocities_.swap();
  }
  
protected:
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0; // velocities
                uniform sampler2D curls;
                uniform vec2 texSize;
                uniform float dt;
                uniform float vorticityStrength;

                void main(){
                  vec2 xy = gl_TexCoord[0].st;
                  vec2 oldV = texture2D(tex0, xy).xy;

                  vec2 off = vec2(1.0, 0.0) / texSize;
                  float curlN = abs(texture2D(curls, xy+off.yx).x);
                  float curlS = abs(texture2D(curls, xy-off.yx).x);
                  float curlE = abs(texture2D(curls, xy+off.xy).x);
                  float curlW = abs(texture2D(curls, xy-off.xy).x);
                  float curlC = texture2D(curls, xy).x;
                  
                  vec2 dw = normalize(0.5 * vec2(curlN - curlS, curlE - curlW) + 0.000001) * vec2(-1, 1) ; // 0.5 constant changes with gridscale
                  
                  vec2 fvc = dw * curlC * dt * vorticityStrength;

                  gl_FragColor = vec4(oldV + fvc, 0.0, 0.0);
                  
//                  vec3 vorticity = vec3(length(curlE) - length(curlW), length(curlN) - length(curlS), 0.0);
//                  vorticity = length(vorticity) > 0.0 ? normalize(vorticity) : vec3(0.0);
//                  vec3 curlC = texture2D(curls, xy).xyz;
//                  vec2 vortForce = vorticityStrength * cross(vorticity, curlC).xy * dt;
//                  
//                  gl_FragColor = vec4(oldV + vortForce, 0.0, 0.0);
                }
                );
  }
  
};
