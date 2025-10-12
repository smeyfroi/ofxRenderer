#pragma once

#include "ofFbo.h"
#include "ofGraphics.h"
#include <algorithm>
#include <iterator>

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
  
  void allocate(glm::vec2 size, GLint internalFormat, int wrap=GL_CLAMP_TO_EDGE, bool useStencil=false, int numSamples=0) {
    ofFboSettings settings;
    settings.width = size.x;
    settings.height = size.y;
    settings.internalformat = internalFormat;
    settings.wrapModeVertical = wrap;
    settings.wrapModeHorizontal = wrap;
    settings.useStencil = useStencil; // TODO: probably enable this all the time once more Mods use it
    settings.numSamples = numSamples; // FIXME: this is more a function of the FBO user, but must be configured at FBO creation
    settings.useDepth = false; // NOTE: so far we don't use depth in MarkSynth
//    settings.textureTarget = GL_TEXTURE_2D; // default is GL_TEXTURE_2D
    allocate(settings);
  }
  
  void allocate(const ofFboSettings& settings) {
    width = settings.width;
    height = settings.height;
    std::for_each(std::begin(fbos), std::end(fbos), [&settings](ofFbo& fbo) {
      fbo.allocate(settings);
    });
  }
  
  bool isAllocated() {
    return getSource().isAllocated(); // shortcut
  }
  
  ofFbo& getSource() { return fbos[1 - currentIndex]; }
  ofFbo& getTarget() { return fbos[currentIndex]; }
  void swap() { currentIndex = 1 - currentIndex; }
  
  void clearFloat(ofFloatColor color) {
    std::for_each(std::begin(fbos), std::end(fbos), [color](ofFbo& fbo) {
      fbo.begin();
      ofClearFloat(color);
      fbo.end();
    });
  }
  
  void clearFloat(float r, float g, float b, float a) {
    clearFloat(ofFloatColor(r, g, b, a));
  }
  
  void clear(float brightness, float a) {
    std::for_each(std::begin(fbos), std::end(fbos), [brightness, a](ofFbo& fbo) {
      fbo.begin();
      ofClear(brightness, a);
      fbo.end();
    });
  }
  
  void draw(float x, float y) const override {
    draw(x, y, width, height);
  }
  void draw(float x, float y, float w, float h) const override {
    fbos[1 - currentIndex].draw(x, y, w, h); // workaround inherited getSource non-constness
  }
  
  float getWidth() const override { return width; }
  float getHeight() const override { return height; }

private:
  int currentIndex;
  size_t width, height;
  ofFbo fbos[2];
};
