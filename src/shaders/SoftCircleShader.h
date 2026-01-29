//
//  SoftCircleShader.h
//  fingerprint1
//
//  Created by Steve Meyfroidt on 16/08/2025.
//

#pragma once

#include "../Shader.h"
#include "ofGraphics.h"
#include "../UnitQuadMesh.h"

class SoftCircleShader : public Shader {

public:
  // falloff: 0 = Glow (default), 1 = Dab
  void render(glm::vec2 center, float radius, ofFloatColor color, float fadeWidth = 0.3, int falloff = 0) {
    render(center,
           { radius * 2.0f, radius * 2.0f },
           /*angleRad*/ 0.0f,
           color,
           fadeWidth,
           falloff,
           /*edgeAmount*/ 0.0f,
           /*edgeFreq*/ glm::vec3 { 3.0f, 5.0f, 9.0f },
           /*edgeSharpness*/ 1.0f,
           /*edgePhase*/ 0.0f);
  }

  void render(glm::vec2 center,
              glm::vec2 size,
              float angleRad,
              ofFloatColor color,
              float fadeWidth = 0.3,
              int falloff = 0,
              float edgeAmount = 0.0,
              glm::vec3 edgeFreq = glm::vec3 { 3.0f, 5.0f, 9.0f },
              float edgeSharpness = 1.0,
              float edgePhase = 0.0) {
    shader.begin();
    shader.setUniform1f("fadeWidth", fadeWidth);
    shader.setUniform4f("color", color);
    shader.setUniform1i("falloff", falloff);
    shader.setUniform1f("edgeAmount", edgeAmount);
    shader.setUniform3f("edgeFreq", edgeFreq);
    shader.setUniform1f("edgeSharpness", edgeSharpness);
    shader.setUniform1f("edgePhase", edgePhase);
    quadMesh.draw(center, size, angleRad);
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
                uniform float edgeAmount; // 0 = none
                uniform vec3 edgeFreq; // angular frequencies
                uniform float edgeSharpness; // >1 = spikier
                uniform float edgePhase;

                void main() {
                  vec2 center = vec2(0.5, 0.5);
                  float dist = distance(texCoordVarying, center);
                  float normalizedDist = dist * 2.0;

                  vec2 d = texCoordVarying - center;
                  float theta = atan(d.y, d.x);

                  float edgeSignal = sin(edgeFreq.x * theta + edgePhase)
                                     + 0.5 * sin(edgeFreq.y * theta + edgePhase * 1.7)
                                     + 0.25 * sin(edgeFreq.z * theta + edgePhase * 2.3);
                  edgeSignal *= (1.0 / 1.75);

                  float edgeShaped = sign(edgeSignal) * pow(abs(edgeSignal), max(edgeSharpness, 0.001));
                  float boundary = 1.0 + edgeAmount * edgeShaped;
                  boundary = clamp(boundary, 0.2, 2.0);

                  float nd = normalizedDist / boundary;
                  if (nd > 1.0) discard;

                  // Softness is edge width in normalized circle space.
                  // Use derivatives so Softness=0 is still anti-aliased.
                  float aa = fwidth(nd) * 1.5;
                  float edgeWidth = max(fadeWidth, aa);
                  edgeWidth = clamp(edgeWidth, 0.0005, 1.0);

                  // Edge falloff: 1 inside, fades to 0 near the boundary.
                  float edgeAlpha = 1.0 - smoothstep(1.0 - edgeWidth, 1.0, nd);

                  float dabAlpha = 1.0 - (nd * nd);
                  dabAlpha = max(dabAlpha, 0.0);

                  if (falloff == 1) {
                    // Dab: quadratic falloff for broader, softer marks
                    // Uses premultiplied alpha to avoid halo artifacts when marks overlap
                    // Requires GL_ONE, GL_ONE_MINUS_SRC_ALPHA blend func
                    float a = dabAlpha * color.a;
                    fragColor = vec4(color.rgb * a, a);
                  } else {
                    // Glow: edge-controlled falloff (Softness)
                    // Blend toward dabAlpha when Softness is low to avoid a harsh, flat interior.
                    float t = smoothstep(0.25, 0.5, fadeWidth);
                    float alpha = mix(dabAlpha, edgeAlpha, t);
                    fragColor = vec4(color.rgb, color.a * alpha);
                  }
                }
                );
  }

private:
  UnitQuadMesh quadMesh;
};
