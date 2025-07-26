#pragma once

#include "ofMain.h"
#include "ofxGui.h"
//#include "FadeEffect.h"
#include "TranslateEffect.h"
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
//  FadeEffect fadeEffect;
  TranslateEffect translateEffect;
  
  PingPongFbo fbo;

  ofxPanel gui;
  ofParameter<float> fadeAmountParameter { "fadeAmount", 0.001, 0.0, 0.1 };
  ofParameter<glm::vec2> translateByParameter { "translateBy", {0.0, 0.0005}, {-0.01, -0.01}, {0.01, 0.01} };
  ofParameterGroup parameters;
};
