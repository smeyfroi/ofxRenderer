#pragma once

#include "ofMain.h"
#include "SmearShader.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp{
  
public:
  void setup() override;
  void update() override;
  void draw() override;
  void exit() override;
  
  void keyPressed(int key) override;
  void keyReleased(int key) override;
  void mouseMoved(int x, int y ) override;
  void mouseDragged(int x, int y, int button) override;
  void mousePressed(int x, int y, int button) override;
  void mouseReleased(int x, int y, int button) override;
  void mouseScrolled(int x, int y, float scrollX, float scrollY) override;
  void mouseEntered(int x, int y) override;
  void mouseExited(int x, int y) override;
  void windowResized(int w, int h) override;
  void dragEvent(ofDragInfo dragInfo) override;
  void gotMessage(ofMessage msg) override;
		
private:
  int fieldWidth = 200;
  int fieldHeight = 200;
  ofTexture field1Texture, field2Texture;

  SmearShader smearShader;
  
  PingPongFbo fbo;

  ofxPanel gui;
  ofParameter<float> alphaParameter { "alpha", 0.998, 0.9, 1.0 };
  ofParameter<float> mixNewParameter { "mixNew", 0.9, 0.0, 1.0 };
  ofParameter<glm::vec2> translateByParameter { "translateBy", {0.0, 0.0001}, {-0.001, -0.001}, {0.001, 0.001} };
  ofParameter<float> field1MultiplierParameter { "field1Multiplier", 0.002, 0.0, 0.1 };
  ofParameter<glm::vec2> field1BiasParameter { "field1Bias", {-0.5, -0.5}, {-1.0, -1.0}, {1.0, 1.0} };
  ofParameter<float> field2MultiplierParameter { "field2Multiplier", 0.008, 0.0, 0.1 };
  ofParameter<glm::vec2> field2BiasParameter { "field2Bias", {-0.5, -0.5}, {-1.0, -1.0}, {1.0, 1.0} };
  ofParameterGroup parameters;
};
