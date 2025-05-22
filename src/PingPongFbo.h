#pragma once

#include "ofFbo.h"

class PingPongFbo : public ofBaseDraws {
public:
  PingPongFbo() : currentIndex(0) {}
  ~PingPongFbo() override {}
  
  void allocate(size_t width_, size_t height_, GLint internalFormat_) {
    width = width_;
    height = height_;
    getSource().allocate(width, height, internalFormat_);
    getTarget().allocate(width, height, internalFormat_);
  }
  
  void allocate(ofFboSettings settings) {
    width = settings.width;
    height = settings.height;
    getSource().allocate(settings);
    getTarget().allocate(settings);
  }
  
  ofFbo& getSource() { return fbos[1 - currentIndex]; }
  ofFbo& getTarget() { return fbos[currentIndex]; }
  void swap() { currentIndex = 1 - currentIndex; }
  
  void draw(float x, float y) const override {
    fbos[1 - currentIndex].draw(x, y, width, height);
  }
  void draw(float x, float y, float w, float h) const override {
    fbos[1 - currentIndex].draw(x, y, w, h);
  }
  
  float getWidth() const override { return width; }
  float getHeight() const override { return height; }

private:
  int currentIndex;
  size_t width, height;
  ofFbo fbos[2];
};
