#pragma once

#include "Shader.h"

class JacobiShader : public Shader {

public:
  void render(PingPongFbo& x, const ofTexture& b, float dt, float alpha, float rBeta, int iterations) {
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    shader.begin();
    shader.setUniformTexture("b", b, 1);
    shader.setUniform2f("texSize", glm::vec2(x.getSource().getWidth(), x.getSource().getHeight()));
    shader.setUniform1f("alpha", alpha);
    shader.setUniform1f("rBeta", rBeta);
    for (int i = 0; i < iterations; i++) {
      x.getTarget().begin();
      x.getSource().draw(0, 0);
      x.getTarget().end();
      x.swap();
    }
    shader.end();
  }
  
  static ofParameter<int> createIterationsParameter(const std::string& prefix, int value=40) {
    return ofParameter<int> { prefix+"iterations", value, 10, 80 };
  }

protected:
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0; // current values
                uniform sampler2D b; // divergence
                uniform vec2 texSize;
                uniform float alpha;
                uniform float rBeta;
                in vec2 texCoordVarying;
                out vec4 fragColor;

                void main() {
                  vec2 xy = texCoordVarying.xy;
                  vec2 off = vec2(1.0, 0.0) / texSize;

                  // For obstacle support see https://github.com/patriciogonzalezvivo/ofxFluid/blob/master/src/ofxFluid.cpp#L75
                  
                  vec4 pN = texture(tex0, xy+off.yx);
                  vec4 pS = texture(tex0, xy-off.yx);
                  vec4 pE = texture(tex0, xy+off.xy);
                  vec4 pW = texture(tex0, xy-off.xy);

                  vec4 bC = texture(b, xy);
                  
                  fragColor = (pW + pE + pS + pN + alpha * bC) * rBeta;
                }
                );
  }
};
