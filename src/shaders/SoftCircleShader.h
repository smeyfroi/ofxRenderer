//
//  SoftCircleShader.h
//  fingerprint1
//
//  Created by Steve Meyfroidt on 16/08/2025.
//

#pragma once

#include "Shader.h"
#include "ofGraphics.h"
#include "UnitQuadMesh.h"

class SoftCircleShader : public Shader {

public:
  // falloff: 0 = Glow (default), 1 = Dab
  void render(glm::vec2 center, float radius, ofFloatColor color, float fadeWidth = 0.3, int falloff = 0) {
    shader.begin();
    shader.setUniform1f("fadeWidth", fadeWidth);
    shader.setUniform4f("color", color);
    shader.setUniform1i("falloff", falloff);
    quadMesh.draw(center, { radius * 2.0, radius * 2.0 }, 0.0);
    shader.end();
  }

protected:
  std::string getFragmentShader() override {
    return GLSL(
                in vec2 texCoordVarying;
                out vec4 fragColor;
                
                uniform float fadeWidth;
                uniform vec4 color;
                uniform int falloff; // 0 = Glow, 1 = Dab

                void main() {
                  vec2 center = vec2(0.5, 0.5);
                  float dist = distance(texCoordVarying, center);
                  float normalizedDist = dist  * 2.0;
                  if (normalizedDist > 1.0) discard;

                  // Linear smoothstep
//                  float alpha = 1.0 - smoothstep(1.0 - fadeWidth, 1.0, normalizedDist);
                  
                  // Quadratic falloff for more perceptually even brightness
//                  float alpha = 1.0 - (normalizedDist * normalizedDist);
//                  alpha = smoothstep(0.0, fadeWidth, alpha);
                  
                  // Exponential falloff (very smooth, natural light-like)
                  float alpha = exp(-normalizedDist * normalizedDist * 3.0);
                  
                  // Cubic fallof (stronger centre)
//                  float alpha = 1.0 - (normalizedDist * normalizedDist * normalizedDist);
                  
                  if (falloff == 1) {
                    // Dab: quadratic falloff for broader, softer marks
                    // Uses premultiplied alpha to avoid halo artifacts when marks overlap
                    // Requires GL_ONE, GL_ONE_MINUS_SRC_ALPHA blend func
                    float dabAlpha = 1.0 - (normalizedDist * normalizedDist);
                    dabAlpha = max(dabAlpha, 0.0);
                    float a = dabAlpha * color.a;
                    fragColor = vec4(color.rgb * a, a);
                  } else {
                    // Glow: exponential falloff for sharp center, glowing marks
                    fragColor = vec4(color.rgb, color.a * alpha);
                  }
                }
                );
  }
  
private:
  UnitQuadMesh quadMesh;
};
