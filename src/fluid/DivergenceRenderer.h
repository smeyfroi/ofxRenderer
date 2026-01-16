#pragma once

#include "Renderer.h"

class DivergenceRenderer : public Renderer {

public:
  void render(const ofBaseDraws& velocities_) override {
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    fbo.begin();
    shader.begin();
    {
      shader.setUniform2f("texSize", glm::vec2(fbo.getWidth(), fbo.getHeight()));
      velocities_.draw(0, 0, fbo.getWidth(), fbo.getHeight());
    }
    shader.end();
    fbo.end();
    ofPopStyle();
  }

protected:
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0; // velocities
                uniform vec2 texSize;
                out vec4 fragColor;

                void main(){
                  vec2 xy = gl_FragCoord.xy / texSize;
                  xy.y = 1.0 - xy.y;

                  vec2 off = vec2(1.0, 0.0) / texSize;

                  vec2 vN = texture(tex0, xy+off.yx).xy;
                  vec2 vS = texture(tex0, xy-off.yx).xy;
                  vec2 vE = texture(tex0, xy+off.xy).xy;
                  vec2 vW = texture(tex0, xy-off.xy).xy;
                  
                  // This also needs obstacle support, see https://github.com/patriciogonzalezvivo/ofxFluid/blob/master/src/ofxFluid.cpp#L161

                  fragColor.r = (vE.x - vW.x + vN.y - vS.y) * 0.5;
                }
                );
  }
  
  GLint getInternalFormat() override { return GL_R16F; }
};
