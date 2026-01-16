#pragma once

#include "Shader.h"

class ApplyVorticityForceShader : public Shader {
  
public:
  void render(PingPongFbo& velocities_, ofFbo& curls_, float vorticityStrength_, float dt_) {
    ofPushStyle();
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
    ofPopStyle();
  }
  
protected:
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0; // velocities
                uniform sampler2D curls;
                uniform vec2 texSize;
                uniform float dt;
                uniform float vorticityStrength;
                out vec4 fragColor;

                void main(){
                  vec2 xy = gl_FragCoord.xy / texSize;
                  xy.y = 1.0 - xy.y;

                  vec2 oldV = texture(tex0, xy).xy;

                  vec2 off = vec2(1.0, 0.0) / texSize;
                  float curlN = abs(texture(curls, xy+off.yx).x);
                  float curlS = abs(texture(curls, xy-off.yx).x);
                  float curlE = abs(texture(curls, xy+off.xy).x);
                  float curlW = abs(texture(curls, xy-off.xy).x);
                  float curlC = texture(curls, xy).x;
                  
                  vec2 dw = normalize(0.5 * vec2(curlN - curlS, curlE - curlW) + 0.000001) * vec2(-1, 1);
                  
                  vec2 fvc = dw * curlC * dt * vorticityStrength;

                  fragColor = vec4(oldV + fvc, 0.0, 0.0);
                }
                );
  }
  
};
