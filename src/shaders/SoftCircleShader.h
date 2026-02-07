//
//  SoftCircleShader.h
//  fingerprint1
//
//  Created by Steve Meyfroidt on 16/08/2025.
//

#pragma once

#include <cmath>
#include <cstdint>

#include "../Shader.h"
#include "../UnitQuadMesh.h"
#include "ofGraphics.h"

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

    // Per-stamp seed avoids repeated edge patterns.
    shader.setUniform1f("edgeSeed", hashSeed(center));

    // Gentle drift keeps edges alive without buzzing.
    shader.setUniform1f("timeSec", ofGetElapsedTimef());

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
                uniform float edgeSeed; // per-stamp seed in [0,1)
                uniform float timeSec; // time, for gentle drift

                void main() {
                  vec2 center = vec2(0.5, 0.5);
                  vec2 d = texCoordVarying - center;
                  float dist = length(d);
                  float normalizedDist = dist * 2.0;

                  float theta = atan(d.y, d.x);

                  // Apply edge modulation mostly near the boundary so the interior stays soft.
                  float aaBase = fwidth(normalizedDist) * 1.5;
                  float bandWidth = clamp(max(fadeWidth * 0.75, aaBase * 2.0), 0.002, 0.25);
                  float edgeBand = smoothstep(1.0 - bandWidth, 1.0, normalizedDist);

                  float TAU = 6.28318530718;
                  float phase = edgePhase + edgeSeed * TAU;

                  // Gentle drift: rotates the edge pattern over tens of seconds.
                  phase += timeSec * 0.14;

                  // Slight per-stamp frequency jitter reduces repetitiveness.
                  float s1 = fract(edgeSeed * 17.0 + 0.1);
                  float s2 = fract(edgeSeed * 37.0 + 0.2);
                  float s3 = fract(edgeSeed * 73.0 + 0.3);
                  vec3 freqJitter = vec3(s1, s2, s3) - 0.5;
                  vec3 freq = edgeFreq * (1.0 + freqJitter * 0.18);

                  float edgeSignal = sin(freq.x * theta + phase)
                                     + 0.5 * sin(freq.y * theta + phase * 1.7 + edgeSeed * 2.0)
                                     + 0.25 * sin(freq.z * theta + phase * 2.3 + edgeSeed * 4.0);
                  edgeSignal *= (1.0 / 1.75);

                  float edgeShaped = sign(edgeSignal) * pow(abs(edgeSignal), max(edgeSharpness, 0.001));
                  float boundary = 1.0 + edgeAmount * edgeShaped * edgeBand;
                  boundary = clamp(boundary, 0.2, 2.0);

                  float nd = normalizedDist / boundary;

                   // Softness is edge width in normalized circle space.
                   // Use derivatives so Softness=0 is still anti-aliased.

                  float aa = fwidth(nd) * 1.5;

                  // Avoid harsh cutouts when the edge boundary is modulated.
                  if (nd > 1.0 + aa) discard;

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
                     // Uses premultiplied alpha so scaling/filtering doesn't create halos on either
                     // light or dark backgrounds.
                     // Requires GL_ONE, GL_ONE_MINUS_SRC_ALPHA blend func.
                     float t = smoothstep(0.25, 0.5, fadeWidth);
                     float alpha = mix(dabAlpha, edgeAlpha, t);
                     float a = color.a * alpha;
                     fragColor = vec4(color.rgb * a, a);
                   }


                }
                );
  }

private:
  static float hashSeed(const glm::vec2& p) {
    // Fast, stable hash for per-stamp variation.
    // Uses integer mixing so nearby pixels don't correlate too strongly.
    const std::uint32_t x = static_cast<std::uint32_t>(std::floor(p.x));
    const std::uint32_t y = static_cast<std::uint32_t>(std::floor(p.y));

    std::uint32_t h = x * 374761393u + y * 668265263u; // Large primes
    h = (h ^ (h >> 13u)) * 1274126177u;
    h ^= (h >> 16u);

    return static_cast<float>(h & 0x00FFFFFFu) * (1.0f / 16777216.0f);
  }

  UnitQuadMesh quadMesh;
};
