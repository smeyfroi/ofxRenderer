#pragma once

#include "Shader.h"

class SubtractDivergenceShader : public Shader {
  
public:
  void render(PingPongFbo& velocities_, ofFbo& pressures_) {
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    velocities_.getTarget().begin();
    {
      shader.begin();
      shader.setUniformTexture("pressures", pressures_.getTexture(), 1);
      shader.setUniform2f("texSize", glm::vec2(velocities_.getWidth(), velocities_.getHeight()));
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
                uniform sampler2D pressures;
                uniform vec2 texSize;
                in vec2 texCoordVarying;
                out vec4 fragColor;

                void main(){
                  vec2 xy = texCoordVarying.xy;
                  vec2 off = vec2(1.0, 0.0) / texSize;

                  // Needs to support obstacles https://github.com/patriciogonzalezvivo/ofxFluid/blob/master/src/ofxFluid.cpp#L113

                  float pN = texture(pressures, xy+off.yx).r;
                  float pS = texture(pressures, xy-off.yx).r;
                  float pE = texture(pressures, xy+off.xy).r;
                  float pW = texture(pressures, xy-off.xy).r;
                  vec2 grad = vec2(pE - pW, pN - pS) * 0.5;
                  
                  vec2 oldV = texture(tex0, xy).xy;
                  vec2 newV = oldV - grad;

                  fragColor.rg = newV;
                }
                );
  }
  
};
