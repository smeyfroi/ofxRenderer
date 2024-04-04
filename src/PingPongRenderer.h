#pragma once

#include "Shader.h"
#include "PingPongFbo.h"

class PingPongRenderer : public Shader, public ofBaseDraws {
public:
  PingPongRenderer() : Shader() {}
  virtual ~PingPongRenderer() override {}
  
  void allocate(float width_, float height_) {
    width = width_;
    height = height_;
    pingPongFbo.allocate(width, height, getInternalFormat());
  }
  
  void clear() {
    pingPongFbo.getSource().begin();
    ofClear(getClearColor());
    pingPongFbo.getSource().end();
    pingPongFbo.getTarget().begin();
    ofClear(getClearColor());
    pingPongFbo.getTarget().end();
  }
  
//  void render(const ofBaseDraws& fbo_) {
//    fbo.begin();
//    shader.begin();
//    fbo_.draw(0, 0, fbo.getWidth(), fbo.getHeight());
//    shader.end();
//    fbo.end();
//  }
  
  void draw(float x, float y, float w, float h) const override {
    pingPongFbo.draw(x, y, w, h);
  }
  
  float getWidth() const override { return width; }
  float getHeight() const override { return height; }
  ofFbo& getFbo() { return pingPongFbo.getSource(); }

protected:
  float width, height;
  PingPongFbo pingPongFbo;
  virtual GLint getInternalFormat() { return GL_RGBA; }
  ofColor getClearColor() { return ofColor(0, 0, 0, 0); }
};
