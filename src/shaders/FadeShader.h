#pragma once

#include "Shader.h"

class FadeShader : public Shader {

public:
  FadeShader(ofFloatColor fadeBy_) :
  fadeBy { fadeBy_ }
  {}
  
protected:
  void setupShaders() override {
    shader.setUniform4f("fadeBy", fadeBy);
  }
  
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0;
                uniform vec4 fadeBy;
                
                void main() {
                  vec2 xy = gl_TexCoord[0].st;
                  vec4 color = texture2D(tex0, xy);
                  vec4 fadedColor = color * fadeBy;
                  gl_FragColor = fadedColor;
                }
                );
  }
  
private:
  ofFloatColor fadeBy;
};
