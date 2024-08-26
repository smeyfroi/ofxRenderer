#pragma once

#include "Shader.h"

class AddRadialImpulseShader : public Shader {

public:
  void render(PingPongFbo& fbo, glm::vec2 position, float radius, float value) {
    fbo.getTarget().begin();
    shader.begin();
    {
      shader.setUniform2f("texSize", glm::vec2(fbo.getWidth(), fbo.getHeight()));
      shader.setUniform2f("position", position);
      shader.setUniform1f("radius", radius);
      shader.setUniform1f("value", -1.0 * value); // negative goes outwards
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
                uniform float value;
                
                void main() {
                  vec2 xy = gl_TexCoord[0].st;
                  vec4 oldValue = texture2D(tex0, xy);

                  vec2 v = position - xy * texSize;
                  float fade = 1.0 - smoothstep(0, radius, length(v));
                  gl_FragColor = oldValue + vec4(value * fade * normalize(v), 0.0, 0.0);
                }
                );
  }
};
