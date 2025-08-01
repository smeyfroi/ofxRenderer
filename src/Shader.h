#pragma once

#include "ofMain.h"
#include "PingPongFbo.h"

#define GLSL(shader) "OF_GLSL_SHADER_HEADER\n" #shader

// Manage vertex/fragment shaders, rendering them onto a drawable in some way
class Shader {

public:
  Shader() {}
  virtual ~Shader() {}

  void load() {
    bool shaderLoaded = shader.setupShaderFromSource(GL_VERTEX_SHADER, getVertexShader())
      && shader.setupShaderFromSource(GL_FRAGMENT_SHADER, getFragmentShader())
      && shader.bindDefaults()
      && shader.linkProgram();
    if (!shaderLoaded) {
      ofLogError() << typeid(*this).name() << " not loaded";
      ofExit();
    }
  }

  // Basic convenience implementation
  virtual void render(const ofBaseDraws& fbo_) {
    shader.begin();
    fbo_.draw(0, 0);
    shader.end();
  }
  
  void begin() {
    shader.begin();
  }

  void end() {
    shader.end();
  }

  // Basic convenience implementation
  // NOTE: this does not belong in this class. It is for postprocessing a buffer.
  virtual void render(PingPongFbo& fbo_) {
    fbo_.getTarget().begin();
    {
      shader.begin();
      fbo_.getSource().draw(0, 0);
      shader.end();
    }
    fbo_.getTarget().end();
    fbo_.swap();
  }

protected:
  ofShader shader;

  virtual std::string getVertexShader() {
    return GLSL(
                uniform mat4 modelViewProjectionMatrix;
                in vec4 position;
                in vec2 texcoord;
                in vec4 color;
                out vec2 texCoordVarying;
                out vec4 colorVarying;

                void main() {
                  gl_Position = modelViewProjectionMatrix * position;
                  texCoordVarying = texcoord;
                  colorVarying = color;
                }
    );
  }

  virtual std::string getFragmentShader() {
    return GLSL(
//                uniform sampler2DRect tex0;
                in vec2 texCoordVarying;
                in vec2 colorVarying;
                out vec4 outputColor;

                void main(void) {
                  outputColor = vec4(texCoordVarying, 0.0, 1.0);
//                  outputColor = texture(tex0, texCoordVarying);
                }
    );
  }
};
