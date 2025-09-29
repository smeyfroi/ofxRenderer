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

  void render(PingPongFbo& fbo_, glm::vec2 translateBy_, float mixNew_, float fadeMultiplier_,
              ofTexture& field1Texture_, float field1Multiplier_, glm::vec2 field1Bias_,
              ofTexture& field2Texture_, float field2Multiplier_, glm::vec2 field2Bias_) {
    fbo_.getTarget().begin();
    {
      shader.begin();
      shader.setUniformTexture("tex0", fbo_.getSource().getTexture(), 1);
      shader.setUniform2f("translateBy", translateBy_);
      shader.setUniform1f("mixNew", mixNew_);
      shader.setUniform1f("fadeMultiplier", fadeMultiplier_);
      shader.setUniformTexture("field1Texture", field1Texture_, 2);
      shader.setUniform1f("field1Multiplier", field1Multiplier_);
      shader.setUniform2f("field1Bias", field1Bias_);
      shader.setUniformTexture("field2Texture", field2Texture_, 3);
      shader.setUniform1f("field2Multiplier", field2Multiplier_);
      shader.setUniform2f("field2Bias", field2Bias_);
      quadMesh.draw({ 0.0, 0.0 }, { fbo_.getWidth(), fbo_.getHeight() });
      shader.end();
    }
    fbo_.getTarget().end();
    fbo_.swap();
  }

  void render(PingPongFbo& fbo_, glm::vec2 translateBy_, float mixNew_, float fadeMultiplier_,
              ofTexture& field1Texture_, float field1Multiplier_, glm::vec2 field1Bias_) {
    fbo_.getTarget().begin();
    {
      shader.begin();
      shader.setUniformTexture("tex0", fbo_.getSource().getTexture(), 1);
      shader.setUniform2f("translateBy", translateBy_);
      shader.setUniform1f("mixNew", mixNew_);
      shader.setUniform1f("fadeMultiplier", fadeMultiplier_);
      shader.setUniformTexture("field1Texture", field1Texture_, 2);
      shader.setUniform1f("field1Multiplier", field1Multiplier_);
      shader.setUniform2f("field1Bias", field1Bias_);
      shader.setUniformTexture("field2Texture", emptyFieldTexture, 3);
      shader.setUniform1f("field2Multiplier", 0.0);
      shader.setUniform2f("field2Bias", { 0.0, 0.0 });
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
      shader.setUniformTexture("field1Texture", emptyFieldTexture, 2);
      shader.setUniform1f("field1Multiplier", 0.0);
      shader.setUniform2f("field1Bias", { 0.0, 0.0 });
      shader.setUniformTexture("field2Texture", emptyFieldTexture, 3);
      shader.setUniform1f("field2Multiplier", 0.0);
      shader.setUniform2f("field2Bias", { 0.0, 0.0 });
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
                uniform sampler2D field1Texture;
                uniform float field1Multiplier;
                uniform vec2 field1Bias;
                uniform sampler2D field2Texture;
                uniform float field2Multiplier;
                uniform vec2 field2Bias;
                uniform float mixNew;
                uniform float fadeMultiplier;
                in vec2 texCoordVarying;
                out vec4 fragColor;

                void main() {
                  vec2 field1TranslateBy = (field1Bias + texture(field1Texture, texCoordVarying).xy) * field1Multiplier;
                  vec2 field2TranslateBy = (field2Bias + texture(field2Texture, texCoordVarying).xy) * field2Multiplier;
                  vec2 totalTranslation = translateBy + field1TranslateBy + field2TranslateBy;
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
