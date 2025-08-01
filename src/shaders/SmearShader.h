//
//  SmearShader.h
//  example_smear
//
//  Created by Steve Meyfroidt on 01/08/2025.
//

#pragma once

#include "Shader.h"

class SmearShader : public Shader {

public:
  void render(PingPongFbo& fbo_, glm::vec2 translateBy_, float alpha_) {
    fbo_.getTarget().begin();
    {
      shader.begin();
      shader.setUniform2f("translateBy", translateBy_);
      shader.setUniform1f("alpha", alpha_);
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
                uniform float alpha;

                void main() {
                  vec2 xy = gl_TexCoord[0].st;
                  vec4 smearColor = texture2D(tex0, xy - translateBy);
                  vec4 existingColor = texture2D(tex0, xy);
                  gl_FragColor = mix(existingColor, smearColor, alpha);
                }
                );
  }
  
private:
};
