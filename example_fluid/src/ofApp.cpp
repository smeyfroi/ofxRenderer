#include "ofApp.h"

constexpr float SCALE = 1.0;

//--------------------------------------------------------------
void ofApp::setup(){
  ofSetFrameRate(60);
  ofBackground(0);
  ofDisableArbTex();
  
  fluidSimulation.setup(ofGetWindowSize()*SCALE);
  parameters.add(fluidSimulation.getParameterGroup());
  gui.setup(parameters);
}

//--------------------------------------------------------------
void ofApp::update() {
  if (ofGetMousePressed()) {
    FluidSimulation::Impulse impulse {
      { ofGetMouseX()*SCALE, ofGetMouseY()*SCALE }, // position
      50.0*SCALE, // radius
      glm::vec2 { (ofGetMouseX() - ofGetPreviousMouseX())*0.005, (ofGetMouseY() - ofGetPreviousMouseY())*0.005 } * SCALE, // velocity
      10.0*SCALE, // radialVelocity
      ofFloatColor(0.2+ofRandom(0.4), 0.05+ofRandom(0.3), 0.1+ofRandom(0.3), 0.05),
      1.0, // colorDensity
    };
    fluidSimulation.applyImpulse(impulse);
  }
  fluidSimulation.update();
}

//--------------------------------------------------------------
void ofApp::draw() {
  fluidSimulation.draw(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
  gui.draw();
  ofSetWindowTitle(ofToString(ofGetFrameRate()));
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
