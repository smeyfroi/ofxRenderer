#pragma once

#include "Shader.h"
#include "PingPongFbo.h"

// Manage vertex/fragment shaders, rendering them into an external PingPongFbo
class PingPongShader : public Shader {
  
public:
  PingPongShader() {}
  virtual ~PingPongShader() {}
  
  void render(PingPongFbo& fbo) {
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    ofSetColor(255);

    fbo.getTarget().begin();
    {
      shader.begin();
      setupShader(shader);
      fbo.getSource().draw(0, 0, fbo.getWidth(), fbo.getHeight());
      shader.end();
    }
    fbo.getTarget().end();
    fbo.swap();
  }
};
