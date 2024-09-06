#include "ofApp.h"

constexpr float SCALE = 0.5;

//--------------------------------------------------------------
void ofApp::setup(){
  ofSetVerticalSync(false);
  ofEnableAlphaBlending();
  ofDisableArbTex(); // required for texture2D to work in GLSL, makes texture coords normalized
  ofSetFrameRate(60);

  fluidSimulation.setup(ofGetWindowSize()*SCALE);
  parameters.add(fluidSimulation.getParameterGroup());
  gui.setup(parameters);
}

//--------------------------------------------------------------
void ofApp::update() {
  if (ofGetMousePressed()) {
    FluidSimulation::Impulse impulse {
      { ofGetMouseX()*SCALE, ofGetMouseY()*SCALE },
      30.0, // radius
      { (ofGetMouseX() - ofGetPreviousMouseX())*0.005, (ofGetMouseY() - ofGetPreviousMouseY())*0.005 }, // velocity
      0.05, // radialVelocity
      ofFloatColor(0.2+ofRandom(0.4), 0.05+ofRandom(0.3), 0.1+ofRandom(0.3), 0.05)*0.1,
      10.0 // temperature
    };
    fluidSimulation.applyImpulse(impulse);
  }
  fluidSimulation.update();
}

//--------------------------------------------------------------
void ofApp::draw() {
  ofClear(0, 255);
  fluidSimulation.draw(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
  gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
