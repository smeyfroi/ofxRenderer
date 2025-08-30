#include "ofApp.h"
//#include "OpenGLTimer.h"

//--------------------------------------------------------------
void ofApp::setup(){
  ofEnableAlphaBlending();
  ofDisableArbTex();
  
  fbo.allocate(ofGetWindowWidth(), ofGetWindowHeight(), GL_RGBA16F);
  fbo.getSource().clearColorBuffer(ofFloatColor(0.0, 0.0, 0.0, 0.0));

  fadeEffect.load();
//  translateEffect.load();
  
  parameters.add(fadeAmountParameter);
//  parameters.add(translateByParameter);
  gui.setup(parameters);
}

//--------------------------------------------------------------
void ofApp::update() {
  fbo.getSource().begin();
  {
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    ofSetColor(ofFloatColor(ofRandom(1.0), ofRandom(1.0), ofRandom(1.0), 1.0));
    ofDrawCircle(ofRandomWidth(), ofRandomHeight(), 20.0);
    
    fadeEffect.fadeAmount = fadeAmountParameter;
    fadeEffect.draw(ofGetWindowWidth(), ofGetWindowHeight());
  }
  fbo.getSource().end();

//  translateEffect.translateBy = translateByParameter;
//  translateEffect.alpha = 1.0 - fadeAmountParameter;
//  translateEffect.draw(fbo);
}

//--------------------------------------------------------------
void ofApp::draw() {
  ofClear(0, 255);
  ofEnableBlendMode(OF_BLENDMODE_ALPHA);
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
