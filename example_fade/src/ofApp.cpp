#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
  ofEnableAlphaBlending();
  ofDisableArbTex(); // required for texture2D to work in GLSL, makes texture coords normalized
  ofSetFrameRate(60);
  
  fbo.allocate(ofGetWindowWidth(), ofGetWindowHeight(), GL_RGBA16F);
  fbo.getSource().clearColorBuffer(ofFloatColor(0.0, 0.0, 0.0, 0.0));

  fadeShader.load();
  parameters.add(fadeAmountParameter);
  gui.setup(parameters);
}

//--------------------------------------------------------------
void ofApp::update() {
  fbo.getSource().begin();
  ofSetColor(ofRandom(255), ofRandom(255), ofRandom(255));
  ofDrawCircle(ofRandomWidth(), ofRandomHeight(), 20.0);
  fbo.getSource().end();
  fadeShader.render(fbo, glm::vec4 { 1.0, 1.0, 1.0, fadeAmountParameter });
}

//--------------------------------------------------------------
void ofApp::draw() {
  ofClear(0, 255);
  fbo.draw(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
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
