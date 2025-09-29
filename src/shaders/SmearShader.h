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
  SmearShader() {
    ofPixels emptyFieldPixels;
    emptyFieldPixels.allocate(1, 1, OF_PIXELS_RG);
    emptyFieldPixels.setColor(ofColor::black);
    emptyFieldTexture.allocate(emptyFieldPixels);
    emptyFieldTexture.loadData(emptyFieldPixels);
  }

  void render(PingPongFbo& fbo_, glm::vec2 translateBy_, float mixNew_, float fadeMultiplier_, ofTexture& fieldTexture_, float fieldMultiplier_, glm::vec2 fieldBias_) {
    fbo_.getTarget().begin();
    {
      shader.begin();
      shader.setUniformTexture("tex0", fbo_.getSource().getTexture(), 1);
      shader.setUniform2f("translateBy", translateBy_);
      shader.setUniform1f("mixNew", mixNew_);
      shader.setUniform1f("fadeMultiplier", fadeMultiplier_);
      shader.setUniformTexture("fieldTexture", fieldTexture_, 2);
      shader.setUniform1f("fieldMultiplier", fieldMultiplier_);
      shader.setUniform2f("fieldBias", fieldBias_);
      quadMesh.draw({ 0.0, 0.0 }, { fbo_.getWidth(), fbo_.getHeight() });
      shader.end();
    }
    fbo_.getTarget().end();
    fbo_.swap();
  }

  void render(PingPongFbo& fbo_, glm::vec2 translateBy_, float mixNew_, float fadeMultiplier_) {
    fbo_.getTarget().begin();
    {
      shader.begin();
      shader.setUniformTexture("tex0", fbo_.getSource().getTexture(), 1);
      shader.setUniform2f("translateBy", translateBy_);
      shader.setUniform1f("mixNew", mixNew_);
      shader.setUniform1f("fadeMultiplier", fadeMultiplier_);
      shader.setUniformTexture("fieldTexture", emptyFieldTexture, 2);
      shader.setUniform1f("fieldMultiplier", 0.0);
      shader.setUniform2f("fieldBias", { 0.0, 0.0 });
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
                uniform sampler2D fieldTexture;
                uniform float fieldMultiplier;
                uniform float mixNew;
                uniform float fadeMultiplier;
                uniform vec2 fieldBias;
                in vec2 texCoordVarying;
                out vec4 fragColor;

                void main() {
                  vec2 fieldTranslateBy = (fieldBias + texture(fieldTexture, texCoordVarying).xy) * fieldMultiplier;
                  vec2 totalTranslation = translateBy + fieldTranslateBy;
                  vec4 smearColor = texture(tex0, texCoordVarying - totalTranslation);
                  vec4 existingColor = texture(tex0, texCoordVarying);
                  fragColor = mix(existingColor, smearColor, mixNew);
                  fragColor.a *= fadeMultiplier;
                }
                );
  }
  
private:
  UnitQuadMesh quadMesh;
  ofTexture emptyFieldTexture;
};
