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
  
  updateForcesFn = std::bind(&ofApp::updateFluidSimulationForces, this);
}

//--------------------------------------------------------------
void ofApp::updateFluidSimulationForces() {
  if (! ofGetMousePressed()) return;
  
  const float BRUSH_SIZE = 20.0;
  
  float dx = ofGetMouseX() - ofGetPreviousMouseX();
  float dy = ofGetMouseY() - ofGetPreviousMouseY();

  FluidSimulation::Impulse impulse {
    { ofGetMouseX()*SCALE, ofGetMouseY()*SCALE },
    BRUSH_SIZE, // radius
    { dx*0.005, dy*0.005 }, // velocity
    0.0, // radialVelocity
    ofFloatColor(0.3+ofRandom(0.2), 0.1+ofRandom(0.1), 0.2+ofRandom(0.15), 1.0),
    10.0 // temperature
  };
  
  fluidSimulation.applyImpulse(impulse);
}

void ofApp::update() {
  fluidSimulation.update(updateForcesFn);
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
