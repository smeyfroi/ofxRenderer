#pragma once

#include "Shader.h"

class MultiplyShader : public Shader {

public:
  MultiplyShader(ofFloatColor multiplyBy_) :
  multiplyBy { multiplyBy_ }
  {}
  
  inline void setMultiplyBy(ofFloatColor multiplyBy_) { multiplyBy = multiplyBy_; }
  
  void render(const ofBaseDraws& fbo_, float x, float y, float w, float h) {
    shader.begin();
    setupShaders();
    fbo_.draw(x, y, w, h);
    shader.end();
  }

protected:
  void setupShaders() override {
    shader.setUniform4f("multiplyBy", multiplyBy);
  }
  
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0;
                uniform vec4 multiplyBy;

                void main() {
                  vec2 xy = gl_TexCoord[0].st;
                  vec4 color = texture2D(tex0, xy);
                  gl_FragColor = color * multiplyBy;
                }
                );
  }
  
private:
  ofFloatColor multiplyBy;
};
