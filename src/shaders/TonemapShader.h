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
             float whitePoint = 11.2,
             float contrast = 1.0,
             float saturation = 1.0,
             float brightness = 0.0,
             float hueShift = 0.0) {
    shader.begin();
    shader.setUniform1i("u_tonemapType", tonemapType);
    shader.setUniform1f("u_exposure", exposure);
    shader.setUniform1f("u_gamma", gamma);
    shader.setUniform1f("u_whitePoint", whitePoint);
    shader.setUniform1f("u_contrast", contrast);
    shader.setUniform1f("u_saturation", saturation);
    shader.setUniform1f("u_brightness", brightness);
    shader.setUniform1f("u_hueShift", hueShift);
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
                uniform float u_contrast;
                uniform float u_saturation;
                uniform float u_brightness;
                uniform float u_hueShift;
                
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

                vec3 adjustSaturation(vec3 color, float saturation) {
                  float gray = dot(color, vec3(0.299, 0.587, 0.114));
                  return mix(vec3(gray), color, saturation);
                }

                vec3 adjustBrightness(vec3 color, float brightness) {
                  return clamp(color + brightness, 0.0, 1.0);
                }

                vec3 adjustHue(vec3 color, float hue) {
                  float angle = hue * 6.2831853; // hue in [0,1], convert to radians
                  float cosA = cos(angle);
                  float sinA = sin(angle);
                  mat3 rot = mat3(
                    vec3(0.299 + 0.701 * cosA + 0.168 * sinA, 0.587 - 0.587 * cosA + 0.330 * sinA, 0.114 - 0.114 * cosA - 0.497 * sinA),
                    vec3(0.299 - 0.299 * cosA - 0.328 * sinA, 0.587 + 0.413 * cosA + 0.035 * sinA, 0.114 - 0.114 * cosA + 0.292 * sinA),
                    vec3(0.299 - 0.3 * cosA + 1.25 * sinA,    0.587 - 0.588 * cosA - 1.05 * sinA, 0.114 + 0.886 * cosA - 0.203 * sinA)
                  );
                  return clamp(rot * color, 0.0, 1.0);
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
                  
                  // Apply contrast, saturation, brightness and hue-shift adjustments
                  ldrColor = (ldrColor - 0.5) * u_contrast + 0.5;
                  ldrColor = adjustSaturation(ldrColor, u_saturation);
                  ldrColor = adjustBrightness(ldrColor, u_brightness);
                  ldrColor = adjustHue(ldrColor, u_hueShift);
                  
                  // Apply gamma correction
                  ldrColor = pow(ldrColor, vec3(1.0 / u_gamma));
                  
                  fragColor = vec4(ldrColor, 1.0);
                }
                );
  }
};
