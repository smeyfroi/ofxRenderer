#pragma once

#include <algorithm>

#include "Renderer.h"

class VorticityRenderer : public Renderer {

public:
  VorticityRenderer() {}
  
  void render(const ofBaseDraws& velocities_) override {
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    fbo.begin();
    shader.begin();
    const float gridSize = std::min(fbo.getWidth(), fbo.getHeight());
    shader.setUniform2f("texSize", glm::vec2(fbo.getWidth(), fbo.getHeight()));
    shader.setUniform1f("halfInvDx", 0.5f * gridSize);
    velocities_.draw(0, 0, fbo.getWidth(), fbo.getHeight());
    shader.end();
    fbo.end();
    ofPopStyle();
  }

protected:
  std::string getFragmentShader() override {
    return GLSL(
                 uniform sampler2D tex0;
                 uniform vec2 texSize;
                 uniform float halfInvDx;
                 in vec2 texCoordVarying;
                 out vec4 fragColor;

                 void main(){
                   vec2 xy = texCoordVarying.xy;


                  vec2 off = vec2(1.0, 0.0) / texSize;

                  vec2 vN = texture(tex0, xy+off.yx).xy;
                  vec2 vS = texture(tex0, xy-off.yx).xy;
                  vec2 vE = texture(tex0, xy+off.xy).xy;
                  vec2 vW = texture(tex0, xy-off.xy).xy;
                  
                  float dVyDx = vE.y - vW.y;
                  float dVxDy = vN.x - vS.x;

                  // 2D curl (z component): curl = dVy/dx - dVx/dy
                  fragColor.r = (dVyDx - dVxDy) * halfInvDx;
                }
                );
  }
  
  GLint getInternalFormat() override { return GL_R32F; }
};
