#pragma once

#include "ofMain.h"
#include "PingPongFbo.h"

//#define GLSL(shader) "#version 300 es\nprecision mediump float;\n" #shader
#define GLSL(shader) "#version 410\n" #shader

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
//                uniform sampler2D tex0;
                in vec2 texCoordVarying;
                in vec4 colorVarying;
                out vec4 fragColor;

                void main(void) {
                  fragColor = vec4(1.0, 0.0, 0.0, 1.0);
//                  fragColor = texture(tex0, texCoordVarying);
                }
    );
  }
};
