#pragma once

#include "Shader.h"

class JacobiShader : public Shader {

public:
  void render(PingPongFbo& x, const ofTexture& b, float dt, float alpha, float rBeta) {
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    shader.begin();
    setupShaders();
    shader.setUniformTexture("b", b, 1);
    shader.setUniform2f("texSize", glm::vec2(x.getSource().getWidth(), x.getSource().getHeight()));
    shader.setUniform1f("alpha", alpha);
    shader.setUniform1f("rBeta", rBeta);
    for (int i = 0; i < iterationsParameter; i++) {
      x.getTarget().begin();
      x.getSource().draw(0, 0);
      x.getTarget().end();
      x.swap();
    }
    shader.end();
  }
  
  ofParameterGroup& getParameterGroup(std::string prefix) {
    if (parameters.size() == 0) {
      parameters.setName(prefix + parameters.getName());
      iterationsParameter.setName(prefix + iterationsParameter.getName());
      parameters.add(iterationsParameter);
    }
    return parameters;
  }

protected:
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0; // current values
                uniform sampler2D b; // divergence
                uniform vec2 texSize;
                uniform float alpha;
                uniform float rBeta;
                
                void main() {
                  vec2 xy = gl_TexCoord[0].st;
                  vec2 off = vec2(1.0, 0.0) / texSize;

                  // For obstacle support see https://github.com/patriciogonzalezvivo/ofxFluid/blob/master/src/ofxFluid.cpp#L75
                  
                  vec4 pN = texture2D(tex0, xy+off.yx);
                  vec4 pS = texture2D(tex0, xy-off.yx);
                  vec4 pE = texture2D(tex0, xy+off.xy);
                  vec4 pW = texture2D(tex0, xy-off.xy);

                  vec4 bC = texture2D(b, xy);
                  
                  gl_FragColor = (pW + pE + pS + pN + alpha * bC) * rBeta;
                }
                );
  }

private:
  ofParameterGroup parameters { "Diffusion" };
  ofParameter<int> iterationsParameter {"iterations", 40, 10, 80 };
};
