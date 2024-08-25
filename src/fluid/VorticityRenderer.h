#pragma once

#include "Renderer.h"

class VorticityRenderer : public Renderer {

public:
  VorticityRenderer() {}
  
  // TODO: hoist to the base class somehow
  void render(const ofBaseDraws& velocities_) override {
    fbo.begin();
    shader.begin();
    shader.setUniform2f("texSize", glm::vec2(velocities_.getWidth(), velocities_.getHeight()));
    velocities_.draw(0, 0);
    shader.end();
    fbo.end();
  }

protected:
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0;
                uniform vec2 texSize;

                void main(){
                  vec2 xy = gl_TexCoord[0].st;
                  vec2 off = vec2(1.0, 0.0) / texSize;

                  vec2 vN = texture2D(tex0, xy+off.yx).xy;
                  vec2 vS = texture2D(tex0, xy-off.yx).xy;
                  vec2 vE = texture2D(tex0, xy+off.xy).xy;
                  vec2 vW = texture2D(tex0, xy-off.xy).xy;
                  
                  float dfx = vE.x - vW.x;
                  float dfy = vN.y - vS.y;

                  gl_FragColor.x = 0.5 * (dfy - dfx); // 0.5 constant changes with gridscale
                }
                );
  }
  
  GLint getInternalFormat() override { return GL_R32F; }
};
