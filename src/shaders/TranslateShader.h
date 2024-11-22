#pragma once

#include "Shader.h"

class TranslateShader : public Shader {

public:
  void render(PingPongFbo& fbo_, glm::vec2 translateBy_) {
    fbo_.getTarget().begin();
    {
      shader.begin();
      shader.setUniform2f("translateBy", translateBy_);
      fbo_.getSource().draw(0, 0);
      shader.end();
    }
    fbo_.getTarget().end();
    fbo_.swap();
  }

protected:
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0;
                uniform vec2 translateBy;

                void main() {
                  vec2 xy = gl_TexCoord[0].st;
                  gl_FragColor = texture2D(tex0, xy - translateBy);
                }
                );
  }
  
private:
};
