#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
  ofBackground(50);
  ofEnableAlphaBlending();
  
  fbo.allocate(ofGetWindowWidth()/4, ofGetWindowHeight()/4, GL_RGBA); // or 16F, 32F
  ofSetColor(0);
  fbo.getSource().begin();
  ofDrawRectangle(0, 0, fbo.getWidth(), fbo.getHeight());
  fbo.getSource().end();

  smearShader.load();
}

//--------------------------------------------------------------
void ofApp::update(){
  fbo.getSource().begin();
  {
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    ofSetColor(ofFloatColor(ofRandom(1.0), ofRandom(1.0), ofRandom(1.0), ofRandom(1.0)));
    ofDrawCircle(ofRandomWidth(), ofRandomHeight(), ofRandom(40.0));
  }
  fbo.getSource().end();

  smearShader.render(fbo, {0.0, 0.0005}, 0.4);
}

//--------------------------------------------------------------
void ofApp::draw(){
  ofEnableBlendMode(OF_BLENDMODE_DISABLED);
  ofSetColor(255);
//  smearShader.render();
  fbo.draw(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
  ofDrawBitmapString(ofToString(ofGetFrameRate(), 2) + " FPS", 10, 20);
}

//--------------------------------------------------------------
void ofApp::exit(){

}

//--------------------------------------------------------------
void ofApp::touchDown(ofTouchEventArgs & touch){

}

//--------------------------------------------------------------
void ofApp::touchMoved(ofTouchEventArgs & touch){

}

//--------------------------------------------------------------
void ofApp::touchUp(ofTouchEventArgs & touch){

}

//--------------------------------------------------------------
void ofApp::touchDoubleTap(ofTouchEventArgs & touch){

}

//--------------------------------------------------------------
void ofApp::touchCancelled(ofTouchEventArgs & touch){
    
}

//--------------------------------------------------------------
void ofApp::lostFocus(){

}

//--------------------------------------------------------------
void ofApp::gotFocus(){

}

//--------------------------------------------------------------
void ofApp::gotMemoryWarning(){

}

//--------------------------------------------------------------
void ofApp::deviceOrientationChanged(int newOrientation){

}

//--------------------------------------------------------------
void ofApp::launchedWithURL(std::string url){

}
