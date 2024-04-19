#pragma once

#include "ofMain.h"

#define GLSL(shader) "#version 120\n" #shader

// Manage vertex/fragment shaders, rendering them onto a drawable in some way
class Shader {

public:
  Shader() {}
  virtual ~Shader() {}

  void load() {
    bool shaderLoaded = shader.setupShaderFromSource(GL_VERTEX_SHADER, getVertexShader())
      && shader.setupShaderFromSource(GL_FRAGMENT_SHADER, getFragmentShader())
      && shader.linkProgram();
    if (!shaderLoaded) {
      ofLogError() << typeid(*this).name() << " not loaded";
      ofExit();
    }
  }

  // Basic convenience implementation
  virtual void render(const ofBaseDraws& fbo_) {
    shader.begin();
    setupShaders();
    fbo_.draw(0, 0);
    shader.end();
  }

  // Basic convenience implementation
  virtual void render(PingPongFbo& fbo_) {
//    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
//    ofSetColor(255);
    fbo_.getTarget().begin();
    {
      shader.begin();
      setupShaders();
      fbo_.getSource().draw(0, 0);
      shader.end();
    }
    fbo_.getTarget().end();
    fbo_.swap();
  }

protected:
  ofShader shader;

  virtual void setupShaders() {};

  virtual std::string getVertexShader() {
    return GLSL(
                void main() {
                  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
                  gl_TexCoord[0] = gl_MultiTexCoord0;
                }
                );
  }

  virtual std::string getFragmentShader() {
    return GLSL(
                void main() {
                  gl_FragColor = vec4(gl_TexCoord[0].st, 0.0, 1.0);
                }
                );
  }
};
