//
//  ThresholdedAddShader.h
//  example_threshold_ios
//
//  Created by Steve Meyfroidt on 08/08/2025.
//

// ************************************************ DOESN'T WORK

#pragma once

#include "Shader.h"

class ThresholdedAddShader : public Shader {

public:
  void begin(const ofFbo& existingFbo_, ofFloatColor color) {
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    ofSetColor(255);
    existingFbo_.draw(0, 0);
    
    shader.begin();
    shader.setUniformTexture("tex0", existingFbo_.getTexture(), 0);
    shader.setUniform2f("viewSize", existingFbo_.getWidth(), existingFbo_.getHeight());
    shader.setUniform4f("color", color.r, color.g, color.b, color.a);
  }
  void end() {
    shader.end();
  }

protected:
  std::string getVertexShader() override {
    return GLSL(
                uniform mat4 modelViewProjectionMatrix;
                in vec4 position;

                void main() {
                  gl_Position = modelViewProjectionMatrix * position;
                }
    );
  }

  std::string getFragmentShader() override {
    return GLSL(
                in vec2 texCoordVarying;
                uniform sampler2D tex0;
                uniform vec2 viewSize;
                uniform vec4 color;
                out vec4 fragColor;

                void main() {
                  vec2 texCoord = gl_FragCoord.xy / viewSize;
                  vec4 existingColor = texture(tex0, texCoord);
                  
                  vec4 potentialColor = existingColor + color;
                  
                  float any_exceeds = step(1.0, max(max(potentialColor.r, potentialColor.g), max(potentialColor.b, potentialColor.a)));

                  vec4 thresholdedColor = mix(potentialColor, existingColor, any_exceeds);
                  fragColor = vec4(thresholdedColor.rgb, clamp(thresholdedColor.a, 0.0, 1.0));
                }
                );
  }
};
