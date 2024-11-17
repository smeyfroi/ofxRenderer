#pragma once

#include "Shader.h"
#include "ofGraphics.h"

// TODO: rename to MultiplyShader
class FadeShader : public Shader {

public:
  void render(PingPongFbo& fbo_, glm::vec4 fadeBy) {
    fbo_.getTarget().begin();
    {
      shader.begin();
      shader.setUniform4f("fadeBy", fadeBy);
      ofEnableBlendMode(OF_BLENDMODE_ALPHA);
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
                uniform vec4 fadeBy;
                
                void main() {
                  vec2 xy = gl_TexCoord[0].st;
                  vec4 color = texture2D(tex0, xy);
                  vec4 fadedColor = color * fadeBy;
                  gl_FragColor = fadedColor;
                }
                );
  }
};
