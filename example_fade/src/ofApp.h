#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "MultiplyColorShader.h"
#include "PingPongFbo.h"

class ofApp: public ofBaseApp {
	public:
		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
private:
  MultiplyColorShader multiplyColorShader;
  
  PingPongFbo fbo;

  ofxPanel gui;
  ofParameter<float> multiplyAmountParameter { "fadeAmount", 0.95, 0.0, 10.0 };
  ofParameter<float> clampFactorParameter { "clampFactor", 0.0, 0.0, 10.0 };
  ofParameterGroup parameters;
};
