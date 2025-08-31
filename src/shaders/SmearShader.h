//
//  SmearShader.h
//  example_smear
//
//  Created by Steve Meyfroidt on 01/08/2025.
//

#pragma once

#include "Shader.h"
#include "UnitQuadMesh.h"

class SmearShader : public Shader {

public:
  void render(PingPongFbo& fbo_, glm::vec2 translateBy_, float mixNew_, float fadeMultiplier_ = 1.0) {
    fbo_.getTarget().begin();
    {
      shader.begin();
      shader.setUniformTexture("tex0", fbo_.getSource().getTexture(), 1);
      shader.setUniform2f("translateBy", translateBy_);
      shader.setUniform1f("mixNew", mixNew_);
      shader.setUniform1f("fadeMultiplier", fadeMultiplier_);
      quadMesh.draw({ 0.0, 0.0 }, { fbo_.getWidth(), fbo_.getHeight() });
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
                uniform float mixNew;
                uniform float fadeMultiplier;
                in vec2 texCoordVarying;
                out vec4 fragColor;

                void main() {
                  vec4 smearColor = texture(tex0, texCoordVarying - translateBy);
                  vec4 existingColor = texture(tex0, texCoordVarying);
                  fragColor = mix(existingColor, smearColor, mixNew);
                  fragColor.a *= fadeMultiplier;
                }
                );
  }
  
private:
  UnitQuadMesh quadMesh;
};
