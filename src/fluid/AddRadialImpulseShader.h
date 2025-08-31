#pragma once

#include "Shader.h"
#include "ofGraphics.h"
#include "UnitQuadMesh.h"

class AddRadialImpulseShader : public Shader {

public:
//  void render(PingPongFbo& fbo, glm::vec2 position, float radius, float value) {
  void render(glm::vec2 position, float radius, float value) {
    shader.begin();
    shader.setUniform1f("value", value); // positive goes outwards
    quadMesh.draw(position, radius * 2.0);
    shader.end();
  }
  
protected:
  std::string getFragmentShader() override {
    return GLSL(
                in vec2 texCoordVarying;
                out vec4 fragColor;
                
                uniform float value;
                
                void main() {
                  vec2 center = vec2(0.5, 0.5);
                  vec2 v = texCoordVarying - center;
                  float dist = length(v);
                  if (dist > 0.5) discard;
                  
                  float fade = 1.0 - smoothstep(0, 1.0, 2.0 * dist);
                  fragColor = vec4(value * fade * normalize(v), 0.0, 1.0);
                }
                );
  }
  
private:
  UnitQuadMesh quadMesh;
};
