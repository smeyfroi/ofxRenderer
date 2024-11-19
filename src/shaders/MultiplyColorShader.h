#pragma once

#include "Shader.h"
#include "ofGraphics.h"

class MultiplyColorShader : public Shader {

public:
  void render(PingPongFbo& fbo_, glm::vec4 multiplyBy) {
    fbo_.getTarget().begin();
    {
      shader.begin();
      shader.setUniform4f("multiplyBy", multiplyBy);
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
                uniform vec4 multiplyBy;
                
                void main() {
                  vec2 xy = gl_TexCoord[0].st;
                  vec4 color = texture2D(tex0, xy);
                  gl_FragColor = color * multiplyBy;
                }
                );
  }
};
