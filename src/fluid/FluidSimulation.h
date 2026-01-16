#pragma once

#include <cmath>
#include <memory>
#include <string>

#include "ofLog.h"
#include "ofTexture.h"
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
  
  bool isSetup() const { return flowValuesFboPtr && flowVelocitiesFboPtr; }
  bool isValid() const { return isSetup() && valid; }
  const std::string& getValidationError() const { return validationError; }

  void setup(glm::vec2 flowValuesSize) {
    flowValuesFboPtr = std::make_shared<PingPongFbo>();
    flowValuesFboPtr->allocate(createFboSettings(flowValuesSize, FLOAT_A_MODE));
    flowValuesFboPtr->clearFloat(0.0, 0.0, 0.0, 0.0);

    flowVelocitiesFboPtr = std::make_shared<PingPongFbo>();
    flowVelocitiesFboPtr->allocate(createFboSettings(flowValuesSize, FLOAT_MODE));
    flowVelocitiesFboPtr->clearFloat(0.0, 0.0, 0.0, 0.0);

    validateExternalBuffers();
    setupInternals();
  }

  void setup(std::shared_ptr<PingPongFbo> flowValuesFboPtr_, std::shared_ptr<PingPongFbo> flowVelocitiesFboPtr_) {
    flowValuesFboPtr = std::move(flowValuesFboPtr_);
    flowVelocitiesFboPtr = std::move(flowVelocitiesFboPtr_);
    validateExternalBuffers();
    setupInternals();
  }

  void setupInternals() {
    if (!isValid()) return;

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
    if (!isValid()) return;

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
    if (!isValid()) return;
    ofBlendMode(OF_BLENDMODE_ALPHA);
    ofSetFloatColor(1.0, 1.0, 1.0, 1.0);
    flowValuesFboPtr->draw(x, y, w, h);
//    flowVelocitiesFboPtr->draw(x, y, w, h);
  }

  PingPongFbo& getFlowValuesFbo() { return *flowValuesFboPtr; }
  PingPongFbo& getFlowVelocitiesFbo() { return *flowVelocitiesFboPtr; }
  const ofTexture& getDivergenceTexture() const { return divergenceRenderer.getFbo().getTexture(); }
  const ofTexture& getPressureTexture() const { return pressuresFbo.getSource().getTexture(); }
  const ofTexture& getCurlTexture() const { return vorticityRenderer.getFbo().getTexture(); }
//  PingPongFbo& getTemperaturesFbo() { return temperaturesFbo; }
  
  // NOTE: this is not used by the MarkSynth Fluid wrapper; it has dedicated Mods instead
   void applyImpulse(const FluidSimulation::Impulse& impulse) {
     if (!isValid()) return;

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
  bool validateExternalBuffers() {
    valid = false;
    validationError.clear();

    if (!flowValuesFboPtr || !flowVelocitiesFboPtr) {
      validationError = "FluidSimulation requires non-null flowValues and flowVelocities buffers";
      logValidationErrorOnce();
      return false;
    }

    if (!flowValuesFboPtr->getSource().isAllocated() || !flowVelocitiesFboPtr->getSource().isAllocated()) {
      validationError = "FluidSimulation requires allocated PingPongFbo sources (values/velocities)";
      logValidationErrorOnce();
      return false;
    }

    const auto validateTexture = [&](const ofTexture& tex, const char* label) -> bool {
      const auto& data = tex.getTextureData();

      if (data.textureTarget != GL_TEXTURE_2D) {
        validationError = std::string("FluidSimulation requires GL_TEXTURE_2D textures (got target=") + ofToString(data.textureTarget)
                          + ") for " + label;
        return false;
      }

      const float eps = 1e-4f;
      if (std::abs(data.tex_u - 1.0f) > eps || std::abs(data.tex_t - 1.0f) > eps) {
        validationError = std::string("FluidSimulation requires normalized texcoords (tex_u=1, tex_t=1); got tex_u=")
                          + ofToString(data.tex_u, 4) + " tex_t=" + ofToString(data.tex_t, 4) + " for " + label;
        return false;
      }

      return true;
    };

    if (!validateTexture(flowValuesFboPtr->getSource().getTexture(), "values")) {
      logValidationErrorOnce();
      return false;
    }

    if (!validateTexture(flowVelocitiesFboPtr->getSource().getTexture(), "velocities")) {
      logValidationErrorOnce();
      return false;
    }

    valid = true;
    return true;
  }

  void logValidationErrorOnce() {
    if (validationLogged) return;
    validationLogged = true;
    ofLogError("FluidSimulation") << validationError;
  }

  ofParameterGroup parameters;


  ofParameter<float> dtParameter { "dt", 0.0025, 0.001, 0.005 };
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
  

  bool valid = false;
  bool validationLogged = false;
  std::string validationError;

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
