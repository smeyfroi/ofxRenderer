//
//  UnitQuadMesh.h
//  example_softcircle
//
//  Created by Steve Meyfroidt on 17/08/2025.
//

#pragma once

#include "ofMesh.h"

class UnitQuadMesh {
public:
  UnitQuadMesh() {
    mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
    mesh.addVertex(glm::vec3(-0.5, -0.5, 0.0));
    mesh.addVertex(glm::vec3(0.5, -0.5, 0.0));
    mesh.addVertex(glm::vec3(-0.5, 0.5, 0.0));
    mesh.addVertex(glm::vec3(0.5, 0.5, 0.0));
    mesh.addTexCoord(glm::vec2(0.0, 0.0));
    mesh.addTexCoord(glm::vec2(1.0, 0.0));
    mesh.addTexCoord(glm::vec2(0.0, 1.0));
    mesh.addTexCoord(glm::vec2(1.0, 1.0));
  }
  
  void draw(glm::vec2 centrePosition, glm::vec2 size, float angleRad) const {
    ofPushMatrix();
    ofTranslate(centrePosition);
    ofRotateRad(angleRad);
    ofScale(size.x, size.y);
    mesh.draw();
    ofPopMatrix();
  }

  // Takes top left vertex position for painting a quad across an entire viewport
  void draw(glm::vec2 topLeftPosition, glm::vec2 size) const {
    ofPushMatrix();
    ofTranslate(topLeftPosition + size * 0.5f);
    ofScale(size.x, size.y);
    mesh.draw();
    ofPopMatrix();
  }
  
private:
  ofMesh mesh;
};
