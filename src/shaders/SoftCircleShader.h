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
  void render(glm::vec2 center, float radius, ofFloatColor color, float fadeWidth = 0.3) {
    shader.begin();
    shader.setUniform1f("fadeWidth", fadeWidth);
    shader.setUniform4f("color", color);
    quadMesh.draw(center, radius * 2.0);
    shader.end();
  }

protected:
  std::string getFragmentShader() override {
    return GLSL(
                in vec2 texCoordVarying;
                out vec4 fragColor;
                
                uniform float fadeWidth;
                uniform vec4 color;

                void main() {
                  vec2 center = vec2(0.5, 0.5);
                  float dist = distance(texCoordVarying, center);
                  float normalizedDist = dist  * 2.0;
                  if (normalizedDist > 1.0) discard;

                  float alpha = 1.0 - smoothstep(1.0 - fadeWidth, 1.0, normalizedDist);

                  fragColor = vec4(color.rgb, color.a * alpha);
                }
                );
  }
  
private:
  UnitQuadMesh quadMesh;
};
