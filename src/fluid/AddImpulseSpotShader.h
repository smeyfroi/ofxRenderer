#pragma once

#include "Shader.h"

class AddImpulseSpotShader : public Shader {

public:
  void render(PingPongFbo& fbo, glm::vec2 position, float radius, glm::vec4 value) {
    fbo.getTarget().begin();
    shader.begin();
    {
      shader.setUniform2f("texSize", glm::vec2(fbo.getWidth(), fbo.getHeight()));
      shader.setUniform2f("position", position);
      shader.setUniform1f("radius", radius);
      shader.setUniform4f("value", value);
      fbo.getSource().draw(0, 0);
    }
    shader.end();
    fbo.getTarget().end();
    fbo.swap();
  }
  
protected:
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0;
                uniform vec2 texSize;
                uniform vec2 position;
                uniform float radius;
                uniform vec4 value;
                
                void main() {
                  vec2 xy = gl_TexCoord[0].st;
                  vec4 oldValue = texture2D(tex0, xy);
                  float fade = 1.0 - smoothstep(0, radius, distance(position, xy * texSize));
                  gl_FragColor = oldValue + (value * fade);
                }
                );
  }
};
