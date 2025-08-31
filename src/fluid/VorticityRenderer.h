#pragma once

#include "Renderer.h"

class VorticityRenderer : public Renderer {

public:
  VorticityRenderer() {}
  
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
                in vec2 texCoordVarying;
                out vec4 fragColor;

                void main(){
                  vec2 xy = texCoordVarying.xy;
                  vec2 off = vec2(1.0, 0.0) / texSize;

                  vec2 vN = texture(tex0, xy+off.yx).xy;
                  vec2 vS = texture(tex0, xy-off.yx).xy;
                  vec2 vE = texture(tex0, xy+off.xy).xy;
                  vec2 vW = texture(tex0, xy-off.xy).xy;
                  
                  float dfx = vE.x - vW.x;
                  float dfy = vN.y - vS.y;

                  fragColor.x = 0.5 * (dfy - dfx);
                }
                );
  }
  
  GLint getInternalFormat() override { return GL_R32F; }
};
