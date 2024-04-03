#pragma once

#include "Shader.h"

// An ofBaseDraws that manages vertex/fragment shaders, rendering
// with them onto an internal FBO that can be drawn onto an external buffer
class Renderer : public Shader, public ofBaseDraws {
public:
  Renderer() : Shader() {}
  virtual ~Renderer() override {}
  
  void allocate(float width_, float height_) {
    width = width_;
    height = height_;
    fbo.allocate(width, height, getInternalFormat());
  }
  
  void clear() {
    fbo.begin();
    ofClear(getClearColor());
    fbo.end();
  }
  
  void render(const ofBaseDraws& fbo_) {
    fbo.begin();
    shader.begin();
    fbo_.draw(0, 0, fbo.getWidth(), fbo.getHeight());
    shader.end();
    fbo.end();
  }
  
  void draw(float x, float y, float w, float h) const override {
    fbo.draw(x, y, w, h);
  }
  
  float getWidth() const override { return width; }
  float getHeight() const override { return height; }
  ofFbo& getFbo() { return fbo; }

protected:
  float width, height;
  ofFbo fbo;
  virtual GLint getInternalFormat() { return GL_RGBA; }
  virtual ofColor getClearColor() { return ofColor(0, 0, 0, 0); }
};
