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
  
  void setup() {
    flowValuesFbo.allocate(Constants::CANVAS_WIDTH, Constants::CANVAS_HEIGHT, GL_RGBA32F);
    flowValuesFbo.getSource().begin();
    ofClear(ofFloatColor(0.0, 0.0, 0.0));
    flowValuesFbo.getSource().end();

    flowVelocitiesFbo.allocate(Constants::CANVAS_WIDTH, Constants::CANVAS_HEIGHT, GL_RGB32F);
    flowVelocitiesFbo.getSource().begin();
    ofClear(ofFloatColor(0.0, 0.0, 0.0));
    flowVelocitiesFbo.getSource().end();

    valueAdvectShader.loadShaders();
    velocityAdvectShader.loadShaders();
    valueJacobiShader.loadShaders();
    velocityJacobiShader.loadShaders();
    divergenceRenderer.allocate(Constants::CANVAS_WIDTH, Constants::CANVAS_HEIGHT);
    divergenceRenderer.loadShaders();
    pressuresFbo.allocate(Constants::CANVAS_WIDTH, Constants::CANVAS_HEIGHT, GL_RGB32F);
    pressureJacobiShader.loadShaders();
    subtractDivergenceShader.loadShaders();
    vorticityRenderer.allocate(Constants::CANVAS_WIDTH, Constants::CANVAS_HEIGHT);
    vorticityRenderer.loadShaders();
    applyVorticityForceShader.loadShaders();
    
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
    velocityAdvectShader.render(flowVelocitiesFbo, flowVelocitiesFbo.getSource().getTexture(), dtParameter);
    valueAdvectShader.render(flowValuesFbo, flowVelocitiesFbo.getSource().getTexture(), dtParameter);
    // diffuse
    velocityJacobiShader.render(flowVelocitiesFbo, flowVelocitiesFbo.getSource().getTexture(), dtParameter);
    valueJacobiShader.render(flowValuesFbo, flowValuesFbo.getSource().getTexture(), dtParameter);
    // add forces
    vorticityRenderer.render(flowVelocitiesFbo.getSource());
    applyVorticityForceShader.render(flowVelocitiesFbo, vorticityRenderer.getFbo(), 1.0, dtParameter);
    if (ofGetFrameNum() > 10) {
//      flowValuesFbo.getTarget().begin();
//      addTextureShader.render(flowValuesFbo.getSource(), backgroundFbo, 0.000000001);
//      flowValuesFbo.getTarget().end();
//      flowValuesFbo.swap();
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
  ofParameterGroup parameters;

  ofParameter<float> dtParameter {"dt", 0.01, 0.0001, 0.1 };
  ofParameterGroup valueAdvectParameters;
  ofParameterGroup velocityAdvectParameters;
  ofParameterGroup valueJacobiParameters;
  ofParameterGroup velocityJacobiParameters;

  PingPongFbo flowValuesFbo;
  PingPongFbo flowVelocitiesFbo;
  AdvectShader valueAdvectShader { 0.999 };
  AdvectShader velocityAdvectShader { 0.99 };
  JacobiShader valueJacobiShader { 10000000000.0 };
  JacobiShader velocityJacobiShader { 0.0 };
  DivergenceRenderer divergenceRenderer;
  PingPongFbo pressuresFbo;
  JacobiShader pressureJacobiShader { 10000000000.0 };
  SubtractDivergenceShader subtractDivergenceShader;
  VorticityRenderer vorticityRenderer;
  ApplyVorticityForceShader applyVorticityForceShader;

};
