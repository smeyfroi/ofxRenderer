//
//  Clamp.h
//  fingerprint1
//
//  Created by Steve Meyfroidt on 07/06/2025.
//

#pragma once

#include "Shader.h"
#include "ofGraphics.h"

class ClampShader : public Shader {

public:
  void render(const ofBaseDraws& drawable) {
    shader.begin();
    drawable.draw(0, 0);
    shader.end();
  }
  
  void render(PingPongFbo& fbo_)
  {
    fbo_.getTarget().begin();
    render(fbo_.getSource());
    fbo_.getTarget().end();
    fbo_.swap();
  }

protected:
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0;

                void main() {
                  vec2 xy = gl_TexCoord[0].st;
                  vec4 color = texture2D(tex0, xy);
                  gl_FragColor = clamp(color, vec4(0.0), vec4(1.0));
                }
                );
  }
};
