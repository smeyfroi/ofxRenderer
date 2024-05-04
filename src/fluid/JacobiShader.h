#pragma once

#include "Shader.h"

class JacobiShader : public Shader {

public:
  JacobiShader(float diffusionStrength_ = 0, int iterations_ = 10) {
    diffusionStrengthParameter = diffusionStrength_;
    iterationsParameter = iterations_;
  }
  
  void render(PingPongFbo& x, const ofTexture& b, float dt, float alpha = 0, float rBeta = 0) {
    if (diffusionStrengthParameter == 0) return;
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    shader.begin();
    setupShaders();
    shader.setUniformTexture("b", b, 1);
    shader.setUniform2f("texSize", glm::vec2(x.getSource().getWidth(), x.getSource().getHeight()));
    float dk = diffusionStrengthParameter * dt;
    shader.setUniform1f("alpha", (alpha == 0) ? 4/dk : alpha);
    shader.setUniform1f("rBeta", (rBeta == 0) ? (1/(4/dk*(1+dk))) : rBeta);
    ofSetColor(255);
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
      diffusionStrengthParameter.setName(prefix + diffusionStrengthParameter.getName());
      parameters.add(diffusionStrengthParameter);
      iterationsParameter.setName(prefix + iterationsParameter.getName());
      parameters.add(iterationsParameter);
    }
    return parameters;
  }

protected:
  void setupShaders() override {
    shader.setUniform1f("diffusionStrength", diffusionStrengthParameter);
  }
  
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0; // current values
                uniform sampler2D b;
                uniform vec2 texSize;
                uniform float alpha;
                uniform float rBeta;
                
                void main() {
                  vec2 xy = gl_TexCoord[0].st;
                  vec2 off = vec2(1.0, 0.0) / texSize;

                  vec4 pN = texture2D(tex0, xy+off.yx);
                  vec4 pS = texture2D(tex0, xy-off.yx);
                  vec4 pE = texture2D(tex0, xy+off.xy);
                  vec4 pW = texture2D(tex0, xy-off.xy);
                  vec4 bC = texture2D(b, xy);
                  
                  gl_FragColor = vec4(pW + pE + pS + pN + alpha * bC) * rBeta;
                }
                );
  }

private:
  ofParameterGroup parameters { "Diffusion" };
  ofParameter<float> diffusionStrengthParameter {"diffusionStrength", 0.005, 0.0, 0.05 };
  ofParameter<int> iterationsParameter {"iterations", 20, 10, 40 };
};
