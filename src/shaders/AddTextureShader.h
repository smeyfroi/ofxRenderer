#pragma once

#include "Shader.h"
#include "PingPongFbo.h"

// Handles different-sized addedTexture
// Clamps values to [-1, 1]
class AddTextureShader : public Shader {

public:
  void render(PingPongFbo& fbo_, const ofTexture& addedTexture_, float multiplier_, float offset_ = 0.0f) {
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    fbo_.getTarget().begin();
    shader.begin();
    shader.setUniformTexture("addedTexture", addedTexture_, 1);
    shader.setUniform1f("multiplier", multiplier_);
    shader.setUniform1f("offset", offset_);
    fbo_.getSource().draw(0, 0);
    shader.end();
    fbo_.getTarget().end();
    fbo_.swap();
  }
  
protected:
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0;
                uniform sampler2D addedTexture;
                uniform float multiplier;
                uniform float offset;

                in vec2 texCoordVarying;
                out vec4 fragColor;

                void main() {
                  vec2 xy = texCoordVarying.xy;
                  vec4 color = texture(tex0, xy);
                  vec4 rawAddedColor = texture(addedTexture, xy);
                  vec4 addedColor = rawAddedColor * multiplier + vec4(offset);
                  fragColor = clamp(color + addedColor, vec4(-1.0), vec4(1.0));
                }
                );
  }
};
