#include "ofApp.h"


static ofFloatPixels makePerlin2DNoise(int w, int h, float scale, float z) {
  ofFloatPixels pixels;
  pixels.allocate(w, h, OF_PIXELS_RGB);
  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      float n1 = ofNoise(x * scale, y * scale, z);
      float n2 = ofNoise((x+5000) * scale, (y+5000) * scale, z);
      pixels.setColor(x, y, ofFloatColor(n1, n2, 0.0, 0.0));
    }
  }
  return pixels;
}


//--------------------------------------------------------------
void ofApp::setup(){
  ofEnableAlphaBlending();
  ofDisableArbTex();
  
  fbo.allocate(ofGetWindowWidth(), ofGetWindowHeight(), GL_RGBA); // or 16F, 32F
  fbo.getSource().clearColorBuffer(ofFloatColor(0.0, 0.0, 0.0, 0.0));

  smearShader.load();
  
  ofFbo::Settings s;
  s.width = fieldWidth;
  s.height = fieldHeight;
  s.internalformat = GL_RG16F;
  s.useDepth = false;
  s.useStencil = false;
  s.numColorbuffers = 1;
  s.textureTarget = GL_TEXTURE_2D;
  fieldFbo.allocate(s);

  parameters.add(alphaParameter);
  parameters.add(mixNewParameter);
  parameters.add(translateByParameter);
  parameters.add(fieldMultiplierParameter);
  gui.setup(parameters);
}

//--------------------------------------------------------------
void ofApp::update(){
  ofFloatPixels pixels = makePerlin2DNoise(fieldWidth, fieldHeight, 0.01, ofGetElapsedTimef()*0.1);
  fieldFbo.getTexture().loadData(pixels);

  fbo.getSource().begin();
  {
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    ofSetColor(ofFloatColor(ofRandom(1.0), ofRandom(1.0), ofRandom(1.0), 1.0));
    ofDrawCircle(ofRandomWidth(), ofRandomHeight(), 20.0);
  }
  fbo.getSource().end();

//  smearShader.render(fbo, translateByParameter, mixNewParameter, alphaParameter); // no field
  smearShader.render(fbo, translateByParameter, mixNewParameter, alphaParameter, fieldFbo.getTexture(), fieldMultiplierParameter); // with field
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
