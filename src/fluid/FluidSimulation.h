#pragma once

#include "ofxGui.h"
#include "PingPongFbo.h"
#include "AdvectShader.h"
#include "JacobiShader.h"
#include "DivergenceRenderer.h"
#include "SubtractDivergenceShader.h"
#include "VorticityRenderer.h"
#include "ApplyVorticityForceShader.h"
#include "ApplyBouyancyShader.h"
#include "AddImpulseSpotShader.h"
#include "AddRadialImpulseShader.h"

// https://developer.nvidia.com/gpugems/gpugems/part-vi-beyond-triangles/chapter-38-fast-fluid-dynamics-simulation-gpu
// https://github.com/patriciogonzalezvivo/ofxFluid
// https://github.com/moostrik/ofxFlowTools

// NOTES:
// - How to set up dissipation params (e.g. https://github.com/patriciogonzalezvivo/ofxFluid/blob/master/src/ofxFluid.cpp#L291)


class FluidSimulation {
  
public:
  struct Impulse {
    glm::vec2 position;
    float radius;
    glm::vec2 velocity;
    float radialVelocity; // applies if velocity == nullptr
    ofFloatColor color;
    float colorDensity; // resultant color is `color * colorDensity`
    float temperature;
  };

  void setup(glm::vec2 flowValuesSize) {
    flowValuesFbo.allocate(flowValuesSize.x, flowValuesSize.y, GL_RGBA32F);
    flowValuesFbo.getSource().begin();
    ofClear(ofFloatColor(0.0, 0.0, 0.0, 0.0));
    flowValuesFbo.getSource().end();

    flowVelocitiesFbo.allocate(flowValuesSize.x, flowValuesSize.y, GL_RGB32F);
    flowVelocitiesFbo.getSource().begin();
    ofClear(ofFloatColor(0.0, 0.0, 0.0, 0.0));
    flowVelocitiesFbo.getSource().end();

    valueAdvectShader.load();
    velocityAdvectShader.load();
    
    valueJacobiShader.load();
    velocityJacobiShader.load();

    divergenceRenderer.allocate(flowValuesSize.x, flowValuesSize.y);
    divergenceRenderer.load();

    pressuresFbo.allocate(flowValuesSize.x, flowValuesSize.y, GL_RGB32F);
    pressureJacobiShader.load();

    subtractDivergenceShader.load();
    
    vorticityRenderer.allocate(flowValuesSize.x, flowValuesSize.y);
    vorticityRenderer.load();
    applyVorticityForceShader.load();

    temperaturesFbo.allocate(flowValuesSize.x, flowValuesSize.y, GL_RGB32F);
    temperaturesFbo.getSource().begin();
    ofClear(applyBouyancyShader.getParameterGroup().getFloat("ambientTemperature"), 1.0);
    temperaturesFbo.getSource().end();
    
    addImpulseSpotShader.load();
    addRadialImpulseShader.load();
  }

  std::string getParameterGroupName() { return "Fluid Simulation"; }
  
  ofParameterGroup& getParameterGroup() {
    if (parameters.size() == 0) {
      parameters.setName(getParameterGroupName());
      parameters.add(dtParameter);
      parameters.add(vorticityParameter);
      valueAdvectParameters = valueAdvectShader.getParameterGroup("value:");
      parameters.add(valueAdvectParameters);
      velocityAdvectParameters = velocityAdvectShader.getParameterGroup("velocity:");
      parameters.add(velocityAdvectParameters);
      temperatureAdvectParameters = temperaturesAdvectShader.getParameterGroup("temperature:");
      parameters.add(temperatureAdvectParameters);
      valueJacobiParameters = valueJacobiShader.getParameterGroup("value:");
      parameters.add(valueJacobiParameters);
      velocityJacobiParameters = velocityJacobiShader.getParameterGroup("velocity:");
      parameters.add(velocityJacobiParameters);
      applyBouyancyParameters = applyBouyancyShader.getParameterGroup();
      parameters.add(applyBouyancyParameters);
      pressureJacobiParameters = pressureJacobiShader.getParameterGroup("pressure:");
      parameters.add(pressureJacobiParameters);
    }
    return parameters;
  }
  
  void update() {
    // advect
    velocityAdvectShader.render(flowVelocitiesFbo, flowVelocitiesFbo.getSource().getTexture(), dtParameter);
    temperaturesAdvectShader.render(temperaturesFbo, flowVelocitiesFbo.getSource().getTexture(), dtParameter);
    valueAdvectShader.render(flowValuesFbo, flowVelocitiesFbo.getSource().getTexture(), dtParameter);
    
    applyBouyancyShader.render(flowVelocitiesFbo, temperaturesFbo, flowValuesFbo, dtParameter);

    // diffuse
//    velocityJacobiShader.render(flowVelocitiesFbo, flowVelocitiesFbo.getSource().getTexture(), dtParameter, 1.0E-3, 0.25);
//    valueJacobiShader.render(flowValuesFbo, flowValuesFbo.getSource().getTexture(), dtParameter, -1.0E-3, 0.25);
    
    // add forces
    vorticityRenderer.render(flowVelocitiesFbo.getSource());
    applyVorticityForceShader.render(flowVelocitiesFbo, vorticityRenderer.getFbo(), vorticityParameter, dtParameter);

    // compute
    divergenceRenderer.render(flowVelocitiesFbo.getSource());
    pressuresFbo.getSource().begin();
    ofClear(0, 0, 0);
    pressuresFbo.getSource().end();
    pressureJacobiShader.render(pressuresFbo, divergenceRenderer.getFbo().getTexture(), dtParameter, -1.0, 0.25); // alpha should be -cellSize * cellSize
    subtractDivergenceShader.render(flowVelocitiesFbo, pressuresFbo.getSource());
  }
  
  void draw(float x, float y, float w, float h) {
    ofPushView();
    ofBlendMode(OF_BLENDMODE_ALPHA);
    ofSetFloatColor(1.0, 1.0, 1.0, 1.0);
    flowValuesFbo.draw(x, y, w, h);
    ofPopView();
  }

  PingPongFbo& getFlowValuesFbo() { return flowValuesFbo; }
  PingPongFbo& getFlowVelocitiesFbo() { return flowVelocitiesFbo; }
  PingPongFbo& getTemperaturesFbo() { return temperaturesFbo; }
  
  void applyImpulse(const FluidSimulation::Impulse& impulse) {
    glm::vec4 colorValue { impulse.color.r, impulse.color.g, impulse.color.b, impulse.color.a };
    addImpulseSpotShader.render(flowValuesFbo, impulse.position, impulse.radius, colorValue);

    addRadialImpulseShader.render(flowVelocitiesFbo, impulse.position, impulse.radius, impulse.radialVelocity);
//    glm::vec4 velocityValue { impulse.velocity.r, impulse.velocity.g, 0.0, 0.0 };
//    addImpulseSpotShader.render(flowVelocitiesFbo, impulse.position, impulse.radius, velocityValue);
    
    glm::vec4 temperatureValue { impulse.temperature, 0.0, 0.0, 0.0 };
    addImpulseSpotShader.render(temperaturesFbo, impulse.position, impulse.radius, temperatureValue);
  }
  
private:
  ofParameterGroup parameters;

  ofParameter<float> dtParameter { "dt", 0.125, 0.01, 0.5 };
  ofParameterGroup valueAdvectParameters;
  ofParameterGroup velocityAdvectParameters;
  ofParameterGroup temperatureAdvectParameters;
  ofParameterGroup valueJacobiParameters;
  ofParameterGroup velocityJacobiParameters;
  ofParameterGroup applyBouyancyParameters;
  ofParameterGroup pressureJacobiParameters;
  ofParameter<float> vorticityParameter { "vorticity", 3.0, 0.00, 10.0 };

  PingPongFbo flowValuesFbo;
  PingPongFbo flowVelocitiesFbo;
  PingPongFbo temperaturesFbo;
  AdvectShader valueAdvectShader;
  AdvectShader velocityAdvectShader;
  AdvectShader temperaturesAdvectShader;
  JacobiShader valueJacobiShader;
  JacobiShader velocityJacobiShader;
  DivergenceRenderer divergenceRenderer;
  PingPongFbo pressuresFbo;
  JacobiShader pressureJacobiShader;
  SubtractDivergenceShader subtractDivergenceShader;
  VorticityRenderer vorticityRenderer;
  ApplyVorticityForceShader applyVorticityForceShader;
  ApplyBouyancyShader applyBouyancyShader;
  
  AddImpulseSpotShader addImpulseSpotShader;
  AddRadialImpulseShader addRadialImpulseShader;
};
