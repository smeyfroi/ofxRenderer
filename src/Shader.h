#pragma once

#include "ofMain.h"

#define GLSL(shader) "#version 120\n" #shader

// Manage vertex/fragment shaders, rendering them onto a drawable in some way
class Shader {
  
public:
  Shader() {}
  virtual ~Shader() {}
  
  void loadShaders() {
    bool shaderLoaded = shader.setupShaderFromSource(GL_VERTEX_SHADER, getVertexShader())
      && shader.setupShaderFromSource(GL_FRAGMENT_SHADER, getFragmentShader())
      && shader.linkProgram();
    if (!shaderLoaded) {
      ofLogError() << typeid(*this).name() << " not loaded";
      ofExit();
    }
  }
  
  void render(const ofBaseDraws& fbo_) {
    shader.begin();
    setupShaders();
    fbo_.draw(0, 0);
    shader.end();
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
