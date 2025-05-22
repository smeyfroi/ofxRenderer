#pragma once

#include "Shader.h"
#include "ofGraphics.h"

class MultiplyColorShader : public Shader {

public:
  void render(const ofBaseDraws& drawable, glm::vec4 multiplyBy) {
    shader.begin();
    shader.setUniform4f("multiplyBy", multiplyBy);
    drawable.draw(0, 0);
    shader.end();
  }
  
  void render(PingPongFbo& fbo_, glm::vec4 multiplyBy) {
    fbo_.getTarget().begin();
    render(fbo_.getSource(), multiplyBy);
    fbo_.getTarget().end();
    fbo_.swap();
  }

protected:
  std::string getFragmentShader() override {
    if (ofGetUsingArbTex()) { ofLogError() << "MultiplyColorShader requires ofDisableArbTex() during App.setup()"; }
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
