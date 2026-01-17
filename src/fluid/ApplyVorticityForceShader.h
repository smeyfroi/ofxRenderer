#pragma once

#include <algorithm>

#include "Shader.h"

class ApplyVorticityForceShader : public Shader {
  
public:
  void render(PingPongFbo& velocities_, ofFbo& curls_, float vorticityStrength_, float dt_) {
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    velocities_.getTarget().begin();
    {
      shader.begin();
      const auto texSize = glm::vec2(velocities_.getWidth(), velocities_.getHeight());
      shader.setUniformTexture("curls", curls_.getTexture(), 1);
      shader.setUniform2f("texSize", texSize);
      shader.setUniform2f("halfInvCell", 0.5f * texSize);
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
                uniform vec2 halfInvCell;
                uniform float dt;
                uniform float vorticityStrength;
                in vec2 texCoordVarying;
                out vec4 fragColor;

                void main(){
                  vec2 xy = texCoordVarying.xy;

                  vec2 oldV = texture(tex0, xy).xy;

                  vec2 off = vec2(1.0, 0.0) / texSize;
                  float curlN = abs(texture(curls, xy + off.yx).x);
                  float curlS = abs(texture(curls, xy - off.yx).x);
                  float curlE = abs(texture(curls, xy + off.xy).x);
                  float curlW = abs(texture(curls, xy - off.xy).x);
                  float curlC = texture(curls, xy).x;

                  vec2 grad = vec2(curlE - curlW, curlN - curlS) * halfInvCell;

                  // Normalize without introducing a directional bias.
                  float gradLen = length(grad);
                  vec2 N = (gradLen > 1e-6) ? (grad / gradLen) : vec2(0.0);

                  // 2D vorticity confinement force: (N x omega)
                  // omega is curlC in the z direction.
                  vec2 fvc = vec2(N.y, -N.x) * curlC * dt * vorticityStrength;

                  fragColor = vec4(oldV + fvc, 0.0, 0.0);
                }
                );
  }
  
};
