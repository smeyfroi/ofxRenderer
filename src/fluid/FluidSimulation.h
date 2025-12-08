#pragma once

#include <memory>

#include "ofxGui.h"
#include "PingPongFbo.h"
#include "AdvectShader.h"
#include "JacobiShader.h"
#include "DivergenceRenderer.h"
#include "SubtractDivergenceShader.h"
#include "VorticityRenderer.h"
#include "ApplyVorticityForceShader.h"
//#include "ApplyBouyancyShader.h"
#include "AddRadialImpulseShader.h"
#include "SoftCircleShader.h"

// https://developer.nvidia.com/gpugems/gpugems/part-vi-beyond-triangles/chapter-38-fast-fluid-dynamics-simulation-gpu
// https://github.com/patriciogonzalezvivo/ofxFluid
// https://github.com/moostrik/ofxFlowTools

// NOTES:
// - How to set up dissipation params (e.g. https://github.com/patriciogonzalezvivo/ofxFluid/blob/master/src/ofxFluid.cpp#L291)

// TODO: temperature has never worked
// TODO: bouyancy has never worked
class FluidSimulation {
  
#ifdef TARGET_MAC
const GLint FLOAT_A_MODE = GL_RGBA32F;
const GLint FLOAT_MODE = GL_RGB32F;
#else
const GLint FLOAT_A_MODE = GL_RGBA32F;
const GLint FLOAT_MODE = GL_RGB32F;
#endif

public:
  struct Impulse {
    glm::vec2 position;
    float radius;
    glm::vec2 velocity;
    float radialVelocity; // applies if velocity == nullptr. FIXME: this isn't a ptr!
    ofFloatColor color;
    float colorDensity; // resultant color is `color * colorDensity`
//    float temperature;
  };
  
  auto createFboSettings(glm::vec2 size, GLint internalFormat) {
    ofFboSettings settings;
    settings.wrapModeVertical = GL_REPEAT;
    settings.wrapModeHorizontal = GL_REPEAT;
    settings.width = size.x;
    settings.height = size.y;
    settings.internalformat = internalFormat;
    settings.numSamples = 0;
    settings.useDepth = false;
    settings.useStencil = false;
    settings.textureTarget = GL_TEXTURE_2D; // Explicit to ensure normalized texture coordinates
    return settings;
  }
  
  bool isSetup() { return flowValuesFboPtr && flowVelocitiesFboPtr; }

  void setup(glm::vec2 flowValuesSize) {
    flowValuesFboPtr = std::make_shared<PingPongFbo>();
    flowValuesFboPtr->allocate(createFboSettings(flowValuesSize, FLOAT_A_MODE));
    flowValuesFboPtr->clearFloat(0.0, 0.0, 0.0, 0.0);

    flowVelocitiesFboPtr = std::make_shared<PingPongFbo>();
    flowVelocitiesFboPtr->allocate(createFboSettings(flowValuesSize, FLOAT_MODE));
    flowVelocitiesFboPtr->clearFloat(0.0, 0.0, 0.0, 0.0);

    setupInternals();
  }
  
  void setup(std::shared_ptr<PingPongFbo> flowValuesFboPtr_, std::shared_ptr<PingPongFbo> flowVelocitiesFboPtr_) {
    flowValuesFboPtr = flowValuesFboPtr_;
    flowVelocitiesFboPtr = flowVelocitiesFboPtr_;
    setupInternals();
  }

  void setupInternals() {
    auto flowValuesSize = flowValuesFboPtr->getSize();
    auto flowVelocitiesSize = flowVelocitiesFboPtr->getSize();
    
    valueAdvectShader.load();
    velocityAdvectShader.load();
    
    valueJacobiShader.load();
    velocityJacobiShader.load();

    // Velocity-related internal buffers use velocity FBO size
    divergenceRenderer.allocate(flowVelocitiesSize.x, flowVelocitiesSize.y);
    divergenceRenderer.load();

    pressuresFbo.allocate(flowVelocitiesSize.x, flowVelocitiesSize.y, GL_RGB32F);
    pressureJacobiShader.load();

    subtractDivergenceShader.load();
    
    vorticityRenderer.allocate(flowVelocitiesSize.x, flowVelocitiesSize.y);
    vorticityRenderer.load();
    applyVorticityForceShader.load();

//    temperaturesFbo.allocate(flowValuesSize.x, flowValuesSize.y, GL_RGB32F);
//    temperaturesFbo.getSource().begin();
//    ofClear(ofFloatColor(ambientTemperatureParameter));
//    temperaturesFbo.getSource().end();
//    applyBouyancyShader.load();
    
    addRadialImpulseShader.load();
    softCircleShader.load();
  }

  std::string getParameterGroupName() { return "Fluid Simulation"; }
  
  ofParameterGroup& getParameterGroup() {
    if (parameters.size() == 0) {
      parameters.setName(getParameterGroupName());
      parameters.add(dtParameter);
      parameters.add(vorticityParameter);
      parameters.add(valueAdvectDissipationParameter);
      parameters.add(velocityAdvectDissipationParameter);
//      parameters.add(temperatureAdvectDissipationParameter);
      parameters.add(valueDiffusionIterationsParameter);
      parameters.add(velocityDiffusionIterationsParameter);
      parameters.add(pressureDiffusionIterationsParameter);
//      applyBouyancyParameters.add(ambientTemperatureParameter);
//      applyBouyancyParameters.add(smokeBouyancyParameter);
//      applyBouyancyParameters.add(smokeWeightParameter);
//      applyBouyancyParameters.add(gravityForceXParameter);
//      applyBouyancyParameters.add(gravityForceYParameter);
//      parameters.add(applyBouyancyParameters);
    }
    return parameters;
  }
  
  void update() {
    float dt = dtParameter.get();
    
    // advect
    velocityAdvectShader.render(*flowVelocitiesFboPtr, flowVelocitiesFboPtr->getSource().getTexture(), dt, velocityAdvectDissipationParameter.get());
//    temperaturesAdvectShader.render(temperaturesFbo, flowVelocitiesFbo.getSource().getTexture(), dtParameter, temperatureAdvectDissipationParameter);
    valueAdvectShader.render(*flowValuesFboPtr, flowVelocitiesFboPtr->getSource().getTexture(), dt, valueAdvectDissipationParameter.get());
    
//    applyBouyancyShader.render(flowVelocitiesFbo, temperaturesFbo, flowValuesFbo, dtParameter, ambientTemperatureParameter, smokeBouyancyParameter, smokeWeightParameter, gravityForceXParameter, gravityForceYParameter);

    // diffuse
    velocityJacobiShader.render(*flowVelocitiesFboPtr, flowVelocitiesFboPtr->getSource().getTexture(), dt, 1.0E-3, 0.25, velocityDiffusionIterationsParameter.get());
    valueJacobiShader.render(*flowValuesFboPtr, flowValuesFboPtr->getSource().getTexture(), dt, -1.0E-10, 0.25, valueDiffusionIterationsParameter.get());
    
    // add forces
    vorticityRenderer.render(flowVelocitiesFboPtr->getSource());
    applyVorticityForceShader.render(*flowVelocitiesFboPtr, vorticityRenderer.getFbo(), vorticityParameter.get(), dt);

    // compute
    divergenceRenderer.render(flowVelocitiesFboPtr->getSource());
    pressuresFbo.getSource().begin();
    pressuresFbo.getSource().clearColorBuffer(ofFloatColor(0.0, 0.0, 0.0, 0.0));
    pressuresFbo.getSource().end();
    pressureJacobiShader.render(pressuresFbo, divergenceRenderer.getFbo().getTexture(), dt, -1.0, 0.25, pressureDiffusionIterationsParameter.get()); // alpha should be -cellSize * cellSize
    subtractDivergenceShader.render(*flowVelocitiesFboPtr, pressuresFbo.getSource());
  }
  
  void draw(float x, float y, float w, float h) {
    ofBlendMode(OF_BLENDMODE_ALPHA);
    ofSetFloatColor(1.0, 1.0, 1.0, 1.0);
    flowValuesFboPtr->draw(x, y, w, h);
//    flowVelocitiesFboPtr->draw(x, y, w, h);
  }

  PingPongFbo& getFlowValuesFbo() { return *flowValuesFboPtr; }
  PingPongFbo& getFlowVelocitiesFbo() { return *flowVelocitiesFboPtr; }
//  PingPongFbo& getTemperaturesFbo() { return temperaturesFbo; }
  
  // NOTE: this is not used by the MarkSynth Fluid wrapper; it has dedicated Mods instead
  void applyImpulse(const FluidSimulation::Impulse& impulse) {
    flowValuesFboPtr->getSource().begin();
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    softCircleShader.render(impulse.position, impulse.radius, impulse.color * impulse.colorDensity);
    ofPopStyle();
    flowValuesFboPtr->getSource().end();

    // Apply a radial velocity impulse directly via the shader,
    // which reads the previous velocity field and writes an updated one.
    addRadialImpulseShader.render(*flowVelocitiesFboPtr,
                                  impulse.position,
                                  impulse.radius,
                                  impulse.radialVelocity,
                                  dtParameter.get());
//    ofFloatColor velocityValue { impulse.velocity.r, impulse.velocity.g, 0.0, 1.0 };
//    softCircleShader.render(impulse.position, impulse.radius, velocityValue);
    
//    glm::vec4 temperatureValue { impulse.temperature, 0.0, 0.0, 0.0 };
//    addImpulseSpotShader.render(temperaturesFbo, impulse.position, impulse.radius, temperatureValue);
  }
  
private:
  ofParameterGroup parameters;

  ofParameter<float> dtParameter { "dt", 0.003, 0.001, 0.01 };
  ofParameter<float> vorticityParameter { "Vorticity", 50.0, 0.00, 100.0 };
  
  ofParameter<float> valueAdvectDissipationParameter = AdvectShader::createDissipationParameter("Value ", 0.999);
  ofParameter<float> velocityAdvectDissipationParameter = AdvectShader::createDissipationParameter("Velocity ", 0.999);
//  ofParameter<float> temperatureAdvectDissipationParameter = AdvectShader::createDissipationParameter("temperature:");
  ofParameter<int> valueDiffusionIterationsParameter = JacobiShader::createIterationsParameter("Value ", 1);
  ofParameter<int> velocityDiffusionIterationsParameter = JacobiShader::createIterationsParameter("Velocity ", 1);
  ofParameter<int> pressureDiffusionIterationsParameter = JacobiShader::createIterationsParameter("Pressure ", 25);
//  ofParameterGroup applyBouyancyParameters { "Bouyancy" };
//  ofParameter<float> ambientTemperatureParameter = ApplyBouyancyShader::createAmbientTemperatureParameter();
//  ofParameter<float> smokeBouyancyParameter = ApplyBouyancyShader::createSmokeBouyancyParameter();
//  ofParameter<float> smokeWeightParameter = ApplyBouyancyShader::createSmokeWeightParameter();
//  ofParameter<float> gravityForceXParameter = ApplyBouyancyShader::createGravityForceXParameter();
//  ofParameter<float> gravityForceYParameter = ApplyBouyancyShader::createGravityForceYParameter();
  

  std::shared_ptr<PingPongFbo> flowValuesFboPtr;
  std::shared_ptr<PingPongFbo> flowVelocitiesFboPtr;
//  PingPongFbo temperaturesFbo;
  AdvectShader valueAdvectShader;
  AdvectShader velocityAdvectShader;
//  AdvectShader temperaturesAdvectShader;
  JacobiShader valueJacobiShader;
  JacobiShader velocityJacobiShader;
  DivergenceRenderer divergenceRenderer;
  PingPongFbo pressuresFbo;
  JacobiShader pressureJacobiShader;
  SubtractDivergenceShader subtractDivergenceShader;
  VorticityRenderer vorticityRenderer;
  ApplyVorticityForceShader applyVorticityForceShader;
//  ApplyBouyancyShader applyBouyancyShader;
  
  SoftCircleShader softCircleShader;
  AddRadialImpulseShader addRadialImpulseShader;
};
