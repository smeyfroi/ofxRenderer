//
//  RefractiveRectangleShader.h
//  example_refractiverectangle
//
//  Created by Steve Meyfroidt on 04/10/2025.
//

#pragma once

#include "Shader.h"
#include "UnitQuadMesh.h"

class RefractiveRectangleShader : public Shader {
public:
  void render(const glm::vec2 centrePosition, const glm::vec2 size, float angle, const ofFbo& backgroundFbo) {
    shader.begin();
    shader.setUniformTexture("backgroundTex", backgroundFbo.getTexture(), 0);
    shader.setUniform1f("edgeThicknessNorm", edgeThicknessNormParameter);
    shader.setUniform1f("refractionStrength", refractionStrengthParameter);
    shader.setUniform1f("reflectionStrength", reflectionStrengthParameter);
    shader.setUniform1f("reflectionFalloff", reflectionFalloffParameter);
    shader.setUniform1f("reflectionOffset", reflectionOffsetParameter);
    shader.setUniform1f("fresnelStrength", fresnelStrengthParameter);
    shader.setUniform1f("fresnelFalloff", fresnelFalloffParameter);

    quadMesh.draw(centrePosition, size, angle);

    shader.end();
  }
  
  std::string getParameterGroupName() { return "Refractive Rectangle"; }

  ofParameterGroup& getParameterGroup() {
    if (parameters.size() == 0) {
      parameters.setName(getParameterGroupName());
      parameters.add(edgeThicknessNormParameter);
      parameters.add(refractionStrengthParameter);
      parameters.add(reflectionStrengthParameter);
      parameters.add(reflectionFalloffParameter);
      parameters.add(reflectionOffsetParameter);
      parameters.add(fresnelStrengthParameter);
      parameters.add(fresnelFalloffParameter);
    }
    return parameters;
  }

protected:
  std::string getVertexShader() override {
    return GLSL(
                in vec4 position;

                uniform mat4 modelViewProjectionMatrix;

                out vec2 fragTexCoord;
                out vec2 localPos;

                void main() {
                    vec4 screenPos = modelViewProjectionMatrix * position;
                    gl_Position = screenPos;
                    
                    vec2 ndcPos = screenPos.xy / screenPos.w;
                    fragTexCoord = ndcPos * 0.5 + 0.5;
                    fragTexCoord.y = 1.0 - fragTexCoord.y;
                    
                    localPos = position.xy;
                }
    );
  }

  std::string getFragmentShader() override {
    return GLSL(
                in vec2 fragTexCoord;
                in vec2 localPos;
                out vec4 fragColor;

                uniform sampler2D backgroundTex;
                uniform float edgeThicknessNorm;
                uniform float refractionStrength;
                uniform float reflectionStrength;
                uniform float reflectionFalloff;
                uniform float reflectionOffset;
                uniform float fresnelStrength;
                uniform float fresnelFalloff;

                void main() {
                  vec2 absLocal = abs(localPos);
                  const float rectHalfSize = 0.5;
                  
                  if (absLocal.x > rectHalfSize || absLocal.y > rectHalfSize) {
                    discard;
                  }
                  
                  vec2 sampleUV = fragTexCoord;
                  
                  if (absLocal.x <= rectHalfSize && absLocal.y <= rectHalfSize) {
                    float distLeft = rectHalfSize - absLocal.x;
                    float distTop = rectHalfSize - absLocal.y;
                    
                    float minDist = min(distLeft, distTop);
                    
                    if (minDist < edgeThicknessNorm) {
                      float edgePercent = 1.0 - (minDist / edgeThicknessNorm);
                      float falloff = pow(edgePercent, 3.0);
                      
                      vec2 normalLocal = vec2(0.0);
                      
                      if (minDist == distLeft) {
                        normalLocal = vec2(sign(localPos.x), 0.0);
                      } else {
                        normalLocal = vec2(0.0, sign(localPos.y));
                      }
                      
                      if (distLeft < edgeThicknessNorm && distTop < edgeThicknessNorm) {
                        normalLocal = normalize(vec2(sign(localPos.x), sign(localPos.y)));
                        float cornerFalloff = min(distLeft / edgeThicknessNorm, distTop / edgeThicknessNorm);
                        falloff = pow(1.0 - cornerFalloff, 3.0);
                      }
                      
                      float distortionAmount = falloff * refractionStrength;
                      vec2 distortionUV = normalLocal * distortionAmount;
                      
                      sampleUV = fragTexCoord + distortionUV;
                      sampleUV = clamp(sampleUV, vec2(0.0), vec2(1.0));
                    }
                  }
                  
                  vec4 backgroundColor = texture(backgroundTex, sampleUV);
                  
                  vec2 centerDist = abs(localPos) / rectHalfSize;
                  float distFromCenter = length(centerDist);
                  
                  float reflectionMask = (1.0 - distFromCenter) * reflectionStrength;
                  reflectionMask = pow(reflectionMask, reflectionFalloff);
                  reflectionMask = clamp(reflectionMask, 0.0, 1.0);
                  
                  if (reflectionMask > 0.01 && length(localPos) > 0.01) {
                    vec2 reflectOffset = normalize(localPos) * reflectionOffset;
                    vec2 reflectUV = fragTexCoord - reflectOffset;
                    
                    if (reflectUV.x >= 0.0 && reflectUV.x <= 1.0 && reflectUV.y >= 0.0 && reflectUV.y <= 1.0) {
                      vec4 reflection = texture(backgroundTex, reflectUV);
                      backgroundColor.rgb = mix(backgroundColor.rgb, reflection.rgb, reflectionMask);
                    }
                  }
                  
                  float edgeOnly = min(absLocal.x, absLocal.y) / rectHalfSize;
                  edgeOnly = 1.0 - edgeOnly;
                  float edgeBrightness = pow(distFromCenter, fresnelFalloff) * fresnelStrength * edgeOnly;
                  backgroundColor.rgb += vec3(edgeBrightness);
                  
                  fragColor = backgroundColor;
                }
    );
  }
  
private:
  UnitQuadMesh quadMesh;
  
  ofParameterGroup parameters;
  ofParameter<float> edgeThicknessNormParameter { "edgeThicknessNorm", 0.15, 0.0, 1.0 };
  ofParameter<float> refractionStrengthParameter { "refractionStrength", 0.06, 0.0, 0.1 };
  ofParameter<float> reflectionStrengthParameter { "reflectionStrength", 0.8, 0.0, 4.0 };
  ofParameter<float> reflectionFalloffParameter { "reflectionFalloff", 1.2, 0.0, 4.0 };
  ofParameter<float> reflectionOffsetParameter { "reflectionOffset", 0.05, 0.0, 1.0 };
  ofParameter<float> fresnelStrengthParameter { "fresnelStrength", 0.05, 0.0, 1.0 };
  ofParameter<float> fresnelFalloffParameter { "fresnelFalloff", 10.0, 0.0, 20.0 };
};
