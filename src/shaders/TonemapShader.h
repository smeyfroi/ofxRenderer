//
//  TonemapShader.h
//  ofxMarkSynth
//
//  Created by Steve Meyfroidt on 17/08/2025.
//
//  HDR to LDR tonemapping shader to prevent color accumulation
//

#pragma once

#include "Shader.h"

class TonemapShader : public Shader {
  
public:
  void begin(int tonemapType = 3, // 3 = ACES
             float exposure = 1.0,
             float gamma = 2.2,
             float whitePoint = 11.2) {
    shader.begin();
    shader.setUniform1i("u_tonemapType", tonemapType);
    shader.setUniform1f("u_exposure", exposure);
    shader.setUniform1f("u_gamma", gamma);
    shader.setUniform1f("u_whitePoint", whitePoint);
  }
  
  void end() {
    shader.end();
  }
  
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D u_texture;
                uniform int u_tonemapType;
                uniform float u_exposure;
                uniform float u_gamma;
                uniform float u_whitePoint;
                
                in vec2 texCoordVarying;
                out vec4 fragColor;
                
                vec3 acesTonemap(vec3 color) {
                  const float A = 2.51;
                  const float B = 0.03;
                  const float C = 2.43;
                  const float D = 0.59;
                  const float E = 0.14;
                  return clamp((color * (A * color + B)) / (color * (C * color + D) + E), 0.0, 1.0);
                }
                
                vec3 filmicTonemap(vec3 color) {
                  const float A = 0.15;
                  const float B = 0.50;
                  const float C = 0.10;
                  const float D = 0.20;
                  const float E = 0.02;
                  const float F = 0.30;
                  const float W = 11.2;
                  
                  vec3 curr = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
                  float whiteScale = 1.0 / (((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F);
                  return curr * whiteScale;
                }
                
                vec3 reinhardTonemap(vec3 color) {
                  return color / (1.0 + color);
                }
                
                vec3 reinhardExtendedTonemap(vec3 color, float whitePoint) {
                  vec3 numerator = color * (1.0 + (color / (whitePoint * whitePoint)));
                  return numerator / (1.0 + color);
                }
                
                vec3 exposureTonemap(vec3 color, float exposure) {
                  return 1.0 - exp(-color * exposure);
                }
                
                void main() {
                  vec3 hdrColor = texture(u_texture, texCoordVarying).rgb;
                  
                  // Apply exposure
                  hdrColor *= u_exposure;
                  
                  vec3 ldrColor;
                  
                  // Apply selected tonemapping
                  if (u_tonemapType == 0) {
                    // Linear (clamp)
                    ldrColor = clamp(hdrColor, 0.0, 1.0);
                  } else if (u_tonemapType == 1) {
                    // Reinhard
                    ldrColor = reinhardTonemap(hdrColor);
                  } else if (u_tonemapType == 2) {
                    // Reinhard Extended
                    ldrColor = reinhardExtendedTonemap(hdrColor, u_whitePoint);
                  } else if (u_tonemapType == 3) {
                    // ACES
                    ldrColor = acesTonemap(hdrColor);
                  } else if (u_tonemapType == 4) {
                    // Filmic
                    ldrColor = filmicTonemap(hdrColor);
                  } else if (u_tonemapType == 5) {
                    // Exposure
                    ldrColor = exposureTonemap(hdrColor, u_exposure);
                  } else {
                    // Default to Reinhard
                    ldrColor = reinhardTonemap(hdrColor);
                  }
                  
                  // Apply gamma correction
                  ldrColor = pow(ldrColor, vec3(1.0 / u_gamma));
                  
                  fragColor = vec4(ldrColor, 1.0);
                }
                );
  }
};
