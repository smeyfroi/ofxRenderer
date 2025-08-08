#include "ofApp.h"

// ************************************************ DOESN'T WORK

//--------------------------------------------------------------
void ofApp::setup(){	
  ofBackground(50);
  ofEnableAlphaBlending();
  
  fbo.allocate(ofGetWindowWidth(), ofGetWindowHeight(), GL_RGBA);
  fbo.getSource().begin();
  ofClear(0, 0, 0, 255);
  fbo.getSource().end();

  thresholdedAddShader.load();
}

//--------------------------------------------------------------
void ofApp::update(){
  fbo.getTarget().begin();
  ofClear(50, 0, 0, 255);
  {
    thresholdedAddShader.begin(fbo.getSource(), ofFloatColor(0.5+ofRandom(0.5), 0.5+ofRandom(0.5), 0.5+ofRandom(0.5), ofRandom(1.0)));
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    ofDrawRectangle(ofRandomWidth(), ofRandomHeight(), 40.0+ofRandom(40.0), 40.0+ofRandom(40.0));
//    ofDrawCircle(ofRandomWidth(), ofRandomHeight(), 40.0+ofRandom(40.0));
    thresholdedAddShader.end();
  }
  fbo.getTarget().end();
  fbo.swap();
}

//--------------------------------------------------------------
void ofApp::draw(){
  ofEnableBlendMode(OF_BLENDMODE_DISABLED);
  ofSetColor(255);
  fbo.draw(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
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
