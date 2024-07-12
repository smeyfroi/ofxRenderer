#pragma once

#include "ofxGui.h"
#include "Constants.h"
#include "PingPongFbo.h"
#include "AdvectShader.h"
#include "JacobiShader.h"
#include "DivergenceRenderer.h"
#include "SubtractDivergenceShader.h"
#include "VorticityRenderer.h"
#include "ApplyVorticityForceShader.h"

class FluidSimulation {

public:
  FluidSimulation() {}
  
  void setup(glm::vec2 flowValuesSize, float flowVelocitiesScale_) {
    flowVelocitiesScale = flowVelocitiesScale_;
    flowValuesFbo.allocate(flowValuesSize.x, flowValuesSize.y, GL_RGBA32F);
    flowValuesFbo.getSource().begin();
    ofClear(ofFloatColor(0.0, 0.0, 0.0, 0.0));
    flowValuesFbo.getSource().end();

    flowVelocitiesFbo.allocate(flowVelocitiesScale*flowValuesSize.x, flowVelocitiesScale*flowValuesSize.y, GL_RGB32F);
    flowVelocitiesFbo.getSource().begin();
    ofClear(ofFloatColor(0.0, 0.0, 0.0));
    flowVelocitiesFbo.getSource().end();

    valueAdvectShader.load();
    velocityAdvectShader.load();
    valueJacobiShader.load();
    velocityJacobiShader.load();
    divergenceRenderer.allocate(flowVelocitiesScale*flowValuesSize.x, flowVelocitiesScale*flowValuesSize.y);
    divergenceRenderer.load();
    pressuresFbo.allocate(flowVelocitiesScale*flowValuesSize.x, flowVelocitiesScale*flowValuesSize.y, GL_RGB32F);
    pressureJacobiShader.load();
    subtractDivergenceShader.load();
    vorticityRenderer.allocate(flowVelocitiesScale*flowValuesSize.x, flowVelocitiesScale*flowValuesSize.y);
    vorticityRenderer.load();
    applyVorticityForceShader.load();
    
    valueAdvectParameters = valueAdvectShader.getParameterGroup("value:");
    velocityAdvectParameters = velocityAdvectShader.getParameterGroup("velocity:");
    valueJacobiParameters = valueJacobiShader.getParameterGroup("value:");
    velocityJacobiParameters = velocityJacobiShader.getParameterGroup("velocity:");

    parameters.add(dtParameter);
    parameters.add(valueAdvectParameters);
    parameters.add(velocityAdvectParameters);
    parameters.add(valueJacobiParameters);
    parameters.add(velocityJacobiParameters);
  }
  
  ofParameterGroup& getParameterGroup() { return parameters; }
  
  template<typename F>
  void update(F& addForcesFunction) {
    // advect
    velocityAdvectShader.render(flowVelocitiesFbo, flowVelocitiesFbo.getSource().getTexture(), dtParameter*flowVelocitiesScale);
    valueAdvectShader.render(flowValuesFbo, flowVelocitiesFbo.getSource().getTexture(), dtParameter);

    if (ofGetFrameNum() % 7 == 0) return;

    // diffuse
    velocityJacobiShader.render(flowVelocitiesFbo, flowVelocitiesFbo.getSource().getTexture(), dtParameter*flowVelocitiesScale);
    valueJacobiShader.render(flowValuesFbo, flowValuesFbo.getSource().getTexture(), dtParameter);
    // add forces
    vorticityRenderer.render(flowVelocitiesFbo.getSource());
    applyVorticityForceShader.render(flowVelocitiesFbo, vorticityRenderer.getFbo(), 1.0, dtParameter*flowVelocitiesScale);
    if (ofGetFrameNum() > 10) {
      addForcesFunction();
    }
    //  TODO: Apply forces to values
    // project
    //   calc divergence of velocity
    divergenceRenderer.render(flowVelocitiesFbo.getSource());
    //   calc pressure
    pressuresFbo.getSource().begin();
    ofClear(0, 0, 0);
    pressuresFbo.getSource().end();
    //   TODO: in this loop, should be enforcing the boundaries for pressure
    pressureJacobiShader.render(pressuresFbo, divergenceRenderer.getFbo().getTexture(), -1, 0.4);
    //   TODO: enforce boundaries for velocity
    // subtract gradient
    subtractDivergenceShader.render(flowVelocitiesFbo, pressuresFbo.getSource());
  }
  
  void draw(float x, float y, float w, float h) { flowValuesFbo.draw(x, y, w, h); }

  PingPongFbo& getFlowValuesFbo() { return flowValuesFbo; }
  PingPongFbo& getFlowVelocitiesFbo() { return flowVelocitiesFbo; }
  
private:
  float flowVelocitiesScale; // calculate velocities at (e.g.) 0.5 scale of the values
  
  ofParameterGroup parameters;

  ofParameter<float> dtParameter {"dt", 0.012, 0.001, 0.03 };
  ofParameterGroup valueAdvectParameters;
  ofParameterGroup velocityAdvectParameters;
  ofParameterGroup valueJacobiParameters;
  ofParameterGroup velocityJacobiParameters;

  PingPongFbo flowValuesFbo;
  PingPongFbo flowVelocitiesFbo;
  AdvectShader valueAdvectShader { 0.997 };
  AdvectShader velocityAdvectShader { 0.99999 };
  JacobiShader valueJacobiShader { 10.0, 15 };
  JacobiShader velocityJacobiShader { 0.0, 10 };
  DivergenceRenderer divergenceRenderer;
  PingPongFbo pressuresFbo;
  JacobiShader pressureJacobiShader { 10.0, 10 };
  SubtractDivergenceShader subtractDivergenceShader;
  VorticityRenderer vorticityRenderer;
  ApplyVorticityForceShader applyVorticityForceShader;

};
