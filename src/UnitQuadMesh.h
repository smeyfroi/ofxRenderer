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
  
  void draw(glm::vec2 position, float size) const {
    ofPushMatrix();
    ofTranslate(position);
    ofScale(size, size);
    mesh.draw();
    ofPopMatrix();
  }
  
private:
  ofMesh mesh;
};
