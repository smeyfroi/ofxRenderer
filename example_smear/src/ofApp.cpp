#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
  ofEnableAlphaBlending();
  ofDisableArbTex();
  
  fbo.allocate(ofGetWindowWidth(), ofGetWindowHeight(), GL_RGBA); // or 16F, 32F
  fbo.getSource().clearColorBuffer(ofFloatColor(0.0, 0.0, 0.0, 0.0));

  smearShader.load();
  
  parameters.add(alphaParameter);
  parameters.add(translateByParameter);
  gui.setup(parameters);
}

//--------------------------------------------------------------
void ofApp::update(){
  fbo.getSource().begin();
  {
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    ofSetColor(ofFloatColor(ofRandom(1.0), ofRandom(1.0), ofRandom(1.0), 1.0));
    ofDrawCircle(ofRandomWidth(), ofRandomHeight(), 20.0);
  }
  fbo.getSource().end();

  smearShader.render(fbo, translateByParameter, alphaParameter);
}

//--------------------------------------------------------------
void ofApp::draw(){
  ofClear(0, 255);
  ofEnableBlendMode(OF_BLENDMODE_ALPHA);
  fbo.draw(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
  gui.draw();
}

//--------------------------------------------------------------
void ofApp::exit(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

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
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

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
