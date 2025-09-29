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
  
  field1Texture.allocate(fieldWidth, fieldHeight, GL_RG16F);
  field2Texture.allocate(fieldWidth, fieldHeight, GL_RG16F);

  parameters.add(alphaParameter);
  parameters.add(mixNewParameter);
  parameters.add(translateByParameter);
  parameters.add(field1MultiplierParameter);
  parameters.add(field1BiasParameter);
  parameters.add(field2MultiplierParameter);
  parameters.add(field2BiasParameter);
  gui.setup(parameters);
}

//--------------------------------------------------------------
void ofApp::update(){
  ofFloatPixels pixels1 = makePerlin2DNoise(fieldWidth, fieldHeight, 0.005, ofGetElapsedTimef()*0.1);
  field1Texture.loadData(pixels1);
  ofFloatPixels pixels2 = makePerlin2DNoise(fieldWidth, fieldHeight, 0.02, -1000.0 + ofGetElapsedTimef()*0.1);
  field2Texture.loadData(pixels2);

  fbo.getSource().begin();
  {
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    ofSetColor(ofFloatColor(ofRandom(1.0), ofRandom(1.0), ofRandom(1.0), 1.0));
    ofDrawCircle(ofRandomWidth(), ofRandomHeight(), 20.0);
  }
  fbo.getSource().end();

//  smearShader.render(fbo, translateByParameter, mixNewParameter, alphaParameter); // no field
//  smearShader.render(fbo, translateByParameter, mixNewParameter, alphaParameter, field1Texture, field1MultiplierParameter, field1BiasParameter); // with field 1
  smearShader.render(fbo, translateByParameter, mixNewParameter, alphaParameter, field1Texture, field1MultiplierParameter, field1BiasParameter, field2Texture, field2MultiplierParameter, field2BiasParameter); // with fields 1 and 2
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
