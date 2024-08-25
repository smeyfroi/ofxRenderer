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
void ofApp::updateFluidSimulationForces() {
  if (! ofGetMousePressed()) return;
  
  const float BRUSH_SIZE = 20.0;
  
  fluidSimulation.getFlowValuesFbo().getSource().begin();
  {
    ofPushView();
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofSetFloatColor(0.15+ofRandom(0.2), 0.05+ofRandom(0.1), 0.1+ofRandom(0.15), 0.7);
    ofFill();
    ofDrawCircle(ofGetMouseX()*SCALE, ofGetMouseY()*SCALE, BRUSH_SIZE);
    ofPopView();
  }
  fluidSimulation.getFlowValuesFbo().getSource().end();

  fluidSimulation.getFlowVelocitiesFbo().getSource().begin();
  ofPushView();
  ofEnableBlendMode(OF_BLENDMODE_ADD);
  float dx = ofGetMouseX() - ofGetPreviousMouseX();
  float dy = ofGetMouseY() - ofGetPreviousMouseY();
  ofSetColor(ofFloatColor(dx*0.005, dy*0.005, 0.0, 1.0));
//  ofSetColor(ofFloatColor(0.005, 0.001, 0.0, 1.0));
  ofFill();
  ofDrawCircle(ofGetMouseX()*SCALE, ofGetMouseY()*SCALE, BRUSH_SIZE);
  //  addTextureShader.render(fluidSimulation.getFlowVelocitiesFbo().getSource(), opticalFlowFbo, 0.03);
  ofPopView();
  fluidSimulation.getFlowVelocitiesFbo().getSource().end();
  
  fluidSimulation.getTemperaturesFbo().getSource().begin();
  ofPushView();
  ofEnableBlendMode(OF_BLENDMODE_ADD);
  ofSetColor(ofFloatColor(0.5, 0.0, 0.0, 1.0));
  ofFill();
  ofDrawCircle(ofGetMouseX()*SCALE, ofGetMouseY()*SCALE, BRUSH_SIZE);
  ofPopView();
  fluidSimulation.getTemperaturesFbo().getSource().end();
}

void ofApp::update() {
  std::function<void()> updateForces = std::bind(&ofApp::updateFluidSimulationForces, this);
  fluidSimulation.update(updateForces);
}

//--------------------------------------------------------------
void ofApp::draw() {
  ofBlendMode(OF_BLENDMODE_DISABLED);
  ofClear(0, 255);
  
  ofPushView();
  ofBlendMode(OF_BLENDMODE_ALPHA);
  ofSetFloatColor(1.0, 1.0, 1.0, 1.0);
  fluidSimulation.draw(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
  ofPopView();
  
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
