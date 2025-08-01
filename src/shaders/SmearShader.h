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
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
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
  std::string getVertexShader() override {
    return GLSL(
                uniform mat4 modelViewProjectionMatrix;

                in vec4 position;
                in vec2 texcoord;

                out vec2 texCoordVarying;

                void main() {
                  gl_Position = modelViewProjectionMatrix * position;
                  texCoordVarying = texcoord;
                }
    );
  }

  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0;
                uniform vec2 translateBy;
                uniform float alpha;
                in vec2 texCoordVarying;
                out vec4 fragColor;

                void main() {
                  vec4 smearColor = texture(tex0, texCoordVarying - translateBy);
                  vec4 existingColor = texture(tex0, texCoordVarying);
                  fragColor = mix(existingColor, smearColor, alpha);
                }
                );
  }
};
