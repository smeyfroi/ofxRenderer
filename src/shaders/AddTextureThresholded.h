//
//  AddTextureThresholded.h
//  fingerprint1
//
//  Created by Steve Meyfroidt on 07/06/2025.
//

#pragma once

#include "Shader.h"
#include "PingPongFbo.h"

class AddTextureThresholdedShader : public Shader {

public:
  void render(PingPongFbo& fbo_, const ofFbo& addedTexture_, float threshold_ = 1.0) {
    fbo_.getTarget().begin();
    shader.begin();
    shader.setUniformTexture("addedTexture", addedTexture_.getTexture(), 1);
    shader.setUniform1f("threshold", threshold_);
    fbo_.getSource().draw(0, 0);
    shader.end();
    fbo_.getTarget().end();
    fbo_.swap();
  }

protected:

  std::string getFragmentShader() override {
    return R"(
      uniform sampler2D tex0;
      uniform sampler2D addedTexture;
      uniform float threshold;

      void main() {
        vec2 xy = gl_TexCoord[0].st;
        vec4 color = texture2D(tex0, xy);
        vec4 addedColor = texture2D(addedTexture, xy);
        
        vec4 potentialColor = color + addedColor;
        float r_exceeds = step(threshold, potentialColor.r);
        float g_exceeds = step(threshold, potentialColor.g);
        float b_exceeds = step(threshold, potentialColor.b);
        float any_exceeds = max(r_exceeds, max(g_exceeds, b_exceeds));

        vec4 thresholdedColor = mix(potentialColor, color, any_exceeds);
//      gl_FragColor = thresholdedColor;
        gl_FragColor = vec4(thresholdedColor.rgb, clamp(thresholdedColor.a, 0.0, 1.0));
      }
    )";
  }
};
