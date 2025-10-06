#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
  ofDisableArbTex(); // sampler2D not sampler2DRect
  
  fbo.allocate(ofGetWindowWidth(), ofGetWindowHeight(), GL_RGBA); // or 16F, 32F
  fbo.begin();
  ofClear(0, 0, 0, 255);
  fbo.end();

  refractiveRectangleShader.load();
  
  gui.setup(refractiveRectangleShader.getParameterGroup());
}

//--------------------------------------------------------------
void ofApp::update(){
  rectangle.position.x = std::fmodf(rectangle.position.x + 0.003, 1.0);
  rectangle.position.y = std::fmodf(rectangle.position.y + 0.001, 1.0);
  rectangle.angle += glm::radians(0.3f);
  
  fbo.begin();
  {
    ofClear(220, 220, 220, 255);  // Light gray background
    
    // Draw a grid
    ofSetColor(200, 200, 200);
    for (int x = 0; x < ofGetWidth(); x += 50) {
      ofDrawLine(x, 0, x, ofGetHeight());
    }
    for (int y = 0; y < ofGetHeight(); y += 50) {
      ofDrawLine(0, y, ofGetWidth(), y);
    }
    // Draw diagonal grid
    ofSetColor(180, 180, 180);
    for (int i = -ofGetHeight(); i < ofGetWidth(); i += 50) {
      ofDrawLine(i, 0, i + ofGetHeight(), ofGetHeight());
    }
    for (int i = 0; i < ofGetWidth() + ofGetHeight(); i += 50) {
      ofDrawLine(i, 0, i - ofGetHeight(), ofGetHeight());
    }
    
    // Draw various colored shapes
    ofSetColor(255, 100, 100);
    ofDrawRectangle(100, 100, 200, 200);
    
    ofSetColor(100, 255, 100);
    ofDrawRectangle(400, 200, 150, 300);
    
    ofSetColor(100, 100, 255);
    ofDrawCircle(600, 400, 80);
    
    ofSetColor(255, 255, 100);
    ofDrawTriangle(200, 500, 300, 500, 250, 600);
  }
  fbo.end();
}

//--------------------------------------------------------------
void ofApp::draw(){
  fbo.draw(0, 0, ofGetWindowWidth(), ofGetWindowHeight());

  ofPushMatrix();
  ofScale(ofGetWindowWidth(), ofGetWindowHeight());
  refractiveRectangleShader.render(rectangle.position, rectangle.size, rectangle.angle, fbo);
  ofPopMatrix();
  
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
