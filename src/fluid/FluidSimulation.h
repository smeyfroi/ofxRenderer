#pragma once

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>

#include "ofAppRunner.h" // ofGetLastFrameTime()
#include "ofFbo.h"
#include "ofGraphics.h"
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
#include "VelocityBoundaryShader.h"
#include "VelocityCflClampShader.h"
#include "ApplyBouyancyShader.h"
#include "ApplyTemperatureBuoyancyShader.h"
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
    // Pixels of desired displacement per simulation step (relative to the velocity field).
    glm::vec2 velocity { 0.0f, 0.0f };

    // Pixels of desired displacement per simulation step (relative to the velocity field).
    float radialVelocity = 0.0f;
    float swirlVelocity = 0.0f;

    ofFloatColor color;
    float colorDensity; // resultant color is `color * colorDensity`
//    float temperature;
  };
  
  auto createFboSettings(glm::vec2 size, GLint internalFormat) {
    ofFboSettings settings;
    const GLint wrap = getExpectedWrapMode();
    settings.wrapModeVertical = wrap;
    settings.wrapModeHorizontal = wrap;
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

  struct DebugStepInfo {
    float rawFrameDt = 0.0f;
    float frameDt = 0.0f;
    float dtEffective = 0.0f;
    float dx = 0.0f;
    float velocityDissipation = 1.0f;
    float valueDissipation = 1.0f;
    float velocitySpreadCoeff = 0.0f;
    float valueSpreadCoeff = 0.0f;
    float temperatureDissipation = 1.0f;
    float temperatureSpreadCoeff = 0.0f;
    float vorticityStrength = 0.0f;
  };

  const DebugStepInfo& getDebugStepInfo() const { return debugStepInfo; }

  void resetTemperature() {
    if (!isValid()) return;
    if (!temperaturesFbo.isAllocated()) return;
    temperaturesFbo.clearFloat(ambientTemperatureParameter.get(), 0.0f, 0.0f, 0.0f);
  }

  // Clears pressure on next update() (warm-start uses previous pressure otherwise).
  void resetPressure() { pressureNeedsClear = true; }

  void setup(glm::vec2 flowValuesSize) {
    ownsFlowBuffers = true;
    lastBoundaryMode = boundaryModeParameter.get();

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
    ownsFlowBuffers = false;
    lastBoundaryMode = boundaryModeParameter.get();

    flowValuesFboPtr = std::move(flowValuesFboPtr_);
    flowVelocitiesFboPtr = std::move(flowVelocitiesFboPtr_);
    obstaclesFboPtr.reset();
    validateExternalBuffers();
    setupInternals();
  }

  void setup(std::shared_ptr<PingPongFbo> flowValuesFboPtr_,
             std::shared_ptr<PingPongFbo> flowVelocitiesFboPtr_,
             std::shared_ptr<PingPongFbo> obstaclesFboPtr_) {
    ownsFlowBuffers = false;
    lastBoundaryMode = boundaryModeParameter.get();

    flowValuesFboPtr = std::move(flowValuesFboPtr_);
    flowVelocitiesFboPtr = std::move(flowVelocitiesFboPtr_);
    obstaclesFboPtr = std::move(obstaclesFboPtr_);
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

    velocityBoundaryShader.load();
    velocityCflClampShader.load();

    allocateDiffusionSourceIfNeeded(velocityDiffusionSourceFbo, flowVelocitiesSize.x, flowVelocitiesSize.y, getExpectedWrapMode());
    allocateDiffusionSourceIfNeeded(valueDiffusionSourceFbo, flowValuesSize.x, flowValuesSize.y, getExpectedWrapMode());

    temperatureAdvectShader.load();
    temperatureJacobiShader.load();
    allocateDiffusionSourceIfNeeded(temperatureDiffusionSourceFbo, flowVelocitiesSize.x, flowVelocitiesSize.y, getExpectedWrapMode());

    if (!temperaturesFbo.isAllocated()
        || static_cast<int>(temperaturesFbo.getWidth()) != flowVelocitiesSize.x
        || static_cast<int>(temperaturesFbo.getHeight()) != flowVelocitiesSize.y) {
      temperaturesFbo.allocate(flowVelocitiesSize, FLOAT_A_MODE, getExpectedWrapMode());
      temperaturesFbo.clearFloat(ambientTemperatureParameter.get(), 0.0f, 0.0f, 0.0f);
    }

    applyBouyancyShader.load();
    applyTemperatureBuoyancyShader.load();

    addRadialImpulseShader.load();
    softCircleShader.load();

    applyExpectedWrapModeToInternalBuffers();
    resetPressure();
  }

  std::string getParameterGroupName() { return "Fluid Simulation"; }
  
  ofParameterGroup& getParameterGroup() {
    if (parameters.size() == 0) {
      parameters.setName(getParameterGroupName());
      parameters.add(boundaryModeParameter);
      parameters.add(dtParameter);
      parameters.add(vorticityParameter);
      parameters.add(valueAdvectDissipationParameter);
      parameters.add(velocityAdvectDissipationParameter);
      parameters.add(valueSpreadParameter);
      parameters.add(velocitySpreadParameter);
      parameters.add(valueMaxParameter);

      temperatureParameters.add(temperatureEnabledParameter);
      temperatureParameters.add(temperatureAdvectDissipationParameter);
      temperatureParameters.add(temperatureSpreadParameter);
      temperatureParameters.add(temperatureDiffusionIterationsParameter);
      parameters.add(temperatureParameters);

      obstaclesParameters.add(obstaclesEnabledParameter);
      obstaclesParameters.add(obstacleThresholdParameter);
      obstaclesParameters.add(obstacleInvertParameter);
      parameters.add(obstaclesParameters);

      parameters.add(valueDiffusionIterationsParameter);
      parameters.add(velocityDiffusionIterationsParameter);
      parameters.add(pressureDiffusionIterationsParameter);
      buoyancyParameters.add(buoyancyStrengthParameter);
      buoyancyParameters.add(buoyancyDensityScaleParameter);
      buoyancyParameters.add(buoyancyThresholdParameter);
      buoyancyParameters.add(gravityForceXParameter);
      buoyancyParameters.add(gravityForceYParameter);
      buoyancyParameters.add(buoyancyUseTemperatureParameter);
      buoyancyParameters.add(ambientTemperatureParameter);
      buoyancyParameters.add(temperatureBuoyancyThresholdParameter);
      parameters.add(buoyancyParameters);
    }
    return parameters;
  }
  
  void update() {
    if (!isSetup()) return;

    const int boundaryMode = boundaryModeParameter.get();
    if (boundaryMode != lastBoundaryMode) {
      lastBoundaryMode = boundaryMode;
      resetPressure();
      validateExternalBuffers();
    }

    const bool obstaclesEnabled = obstaclesEnabledParameter.get();
    if (obstaclesEnabled != lastObstaclesEnabled) {
      lastObstaclesEnabled = obstaclesEnabled;
      resetPressure();
      validateExternalBuffers();
    }

    if (!isValid()) return;

    const bool useObstacles = obstaclesEnabled && obstaclesFboPtr && obstaclesFboPtr->getSource().isAllocated();
    const ofTexture& obstaclesTex = useObstacles ? obstaclesFboPtr->getSource().getTexture()
                                                 : flowValuesFboPtr->getSource().getTexture();
    const float obstacleThreshold = obstacleThresholdParameter.get();
    const bool obstacleInvert = obstacleInvertParameter.get();

    const float rawFrameDt = static_cast<float>(ofGetLastFrameTime());
    const float frameDt = clampFrameDt(rawFrameDt);

    // dtParameter is tuned relative to a baseline framerate (historically 30fps).
    // At 30fps, dtEffective ~= dtParameter.
    constexpr float BASE_FPS = 30.0f;
    const float dt = dtParameter.get() * frameDt * BASE_FPS;

    debugStepInfo.rawFrameDt = rawFrameDt;
    debugStepInfo.frameDt = frameDt;
    debugStepInfo.dtEffective = dt;

    const float gridSize = std::min(flowVelocitiesFboPtr->getWidth(), flowVelocitiesFboPtr->getHeight());
    const float dx = 1.0f / std::max(1.0f, gridSize);
    debugStepInfo.dx = dx;

    const float velocityDissipation = persistenceToDissipation(velocityAdvectDissipationParameter.get(), frameDt, 0.05f, 6.0f);
    const float valueDissipation = persistenceToDissipation(valueAdvectDissipationParameter.get(), frameDt, 0.2f, 30.0f);
    debugStepInfo.velocityDissipation = velocityDissipation;
    debugStepInfo.valueDissipation = valueDissipation;

    // advect
    velocityAdvectShader.render(*flowVelocitiesFboPtr,
                                flowVelocitiesFboPtr->getSource().getTexture(),
                                dt,
                                velocityDissipation,
                                0.0f,
                                obstaclesTex,
                                useObstacles,
                                obstacleThreshold,
                                obstacleInvert);
    applyVelocityBoundariesIfNeeded();

    valueAdvectShader.render(*flowValuesFboPtr,
                              flowVelocitiesFboPtr->getSource().getTexture(),
                              dt,
                              valueDissipation,
                              valueMaxParameter.get(),
                              obstaclesTex,
                              useObstacles,
                              obstacleThreshold,
                              obstacleInvert);

    if (temperatureEnabledParameter.get()) {
      const float temperatureDissipation = persistenceToDissipation(temperatureAdvectDissipationParameter.get(), frameDt, 0.2f, 30.0f);
      debugStepInfo.temperatureDissipation = temperatureDissipation;

      temperatureAdvectShader.render(temperaturesFbo,
                                    flowVelocitiesFboPtr->getSource().getTexture(),
                                    dt,
                                    temperatureDissipation,
                                    0.0f,
                                    obstaclesTex,
                                    useObstacles,
                                    obstacleThreshold,
                                    obstacleInvert);

      debugStepInfo.temperatureSpreadCoeff = applyDiffusionIfEnabled(temperaturesFbo,
                                                                    temperatureJacobiShader,
                                                                    temperatureDiffusionSourceFbo,
                                                                    temperatureSpreadParameter.get(),
                                                                    dt,
                                                                    temperatureDiffusionIterationsParameter.get(),
                                                                    1.0e-4f,
                                                                    1500.0f,
                                                                    obstaclesTex,
                                                                    useObstacles,
                                                                    obstacleThreshold,
                                                                    obstacleInvert);
    }

    // diffuse (resolution-independent in cell units)
    debugStepInfo.velocitySpreadCoeff = applyDiffusionIfEnabled(*flowVelocitiesFboPtr,
                                                               velocityJacobiShader,
                                                               velocityDiffusionSourceFbo,
                                                               velocitySpreadParameter.get(),
                                                               dt,
                                                               velocityDiffusionIterationsParameter.get(),
                                                               1.0e-4f,
                                                               80.0f,
                                                               obstaclesTex,
                                                               useObstacles,
                                                               obstacleThreshold,
                                                               obstacleInvert);
    applyVelocityBoundariesIfNeeded();

    debugStepInfo.valueSpreadCoeff = applyDiffusionIfEnabled(*flowValuesFboPtr,
                                                            valueJacobiShader,
                                                            valueDiffusionSourceFbo,
                                                            valueSpreadParameter.get(),
                                                            dt,
                                                            valueDiffusionIterationsParameter.get(),
                                                            1.0e-4f,
                                                            1500.0f,
                                                            obstaclesTex,
                                                            useObstacles,
                                                            obstacleThreshold,
                                                            obstacleInvert);

    // add forces
    vorticityRenderer.render(flowVelocitiesFboPtr->getSource());

    // Normalized 0..1 control mapped to the empirically useful range.
    constexpr float VORTICITY_MAX = 0.3f;
    const float vorticityStrength = std::clamp(vorticityParameter.get(), 0.0f, 1.0f) * VORTICITY_MAX;
    debugStepInfo.vorticityStrength = vorticityStrength;

    applyVorticityForceShader.render(*flowVelocitiesFboPtr,
                                     vorticityRenderer.getFbo(),
                                     vorticityStrength,
                                     dt,
                                     obstaclesTex,
                                     useObstacles,
                                     obstacleThreshold,
                                     obstacleInvert);
    applyVelocityBoundariesIfNeeded();
    applyVelocityCflClamp(dt);

    if (buoyancyStrengthParameter.get() > 0.0f) {
      if (buoyancyUseTemperatureParameter.get()) {
        applyTemperatureBuoyancyShader.render(*flowVelocitiesFboPtr,
                                              temperaturesFbo,
                                              dt,
                                              buoyancyStrengthParameter.get(),
                                              ambientTemperatureParameter.get(),
                                              temperatureBuoyancyThresholdParameter.get(),
                                              gravityForceXParameter.get(),
                                              gravityForceYParameter.get(),
                                              obstaclesTex,
                                              useObstacles,
                                              obstacleThreshold,
                                              obstacleInvert);
      } else {
        applyBouyancyShader.render(*flowVelocitiesFboPtr,
                                   *flowValuesFboPtr,
                                   dt,
                                   buoyancyStrengthParameter.get(),
                                   buoyancyDensityScaleParameter.get(),
                                   buoyancyThresholdParameter.get(),
                                   gravityForceXParameter.get(),
                                   gravityForceYParameter.get(),
                                   obstaclesTex,
                                   useObstacles,
                                   obstacleThreshold,
                                   obstacleInvert);
      }

      applyVelocityBoundariesIfNeeded();
      applyVelocityCflClamp(dt);
    }
 
    // compute
    if (useObstacles) {
      divergenceRenderer.renderWithObstacles(flowVelocitiesFboPtr->getSource(), obstaclesTex, obstacleThreshold, obstacleInvert);
    } else {
      divergenceRenderer.render(flowVelocitiesFboPtr->getSource());
    }
    clearPressureIfNeeded();

    const float pressureAlpha = -(dx * dx);
    pressureJacobiShader.render(pressuresFbo,
                                divergenceRenderer.getFbo().getTexture(),
                                dt,
                                pressureAlpha,
                                0.25,
                                pressureDiffusionIterationsParameter.get(),
                                obstaclesTex,
                                useObstacles,
                                obstacleThreshold,
                                obstacleInvert);

    subtractDivergenceShader.render(*flowVelocitiesFboPtr,
                                    pressuresFbo.getSource(),
                                    obstaclesTex,
                                    useObstacles,
                                    obstacleThreshold,
                                    obstacleInvert);
    applyVelocityBoundariesIfNeeded();
  }
  
  void draw(float x, float y, float w, float h) {
    if (!isValid()) return;
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    ofSetFloatColor(1.0, 1.0, 1.0, 1.0);
    flowValuesFboPtr->draw(x, y, w, h);
//    flowVelocitiesFboPtr->draw(x, y, w, h);
  }

  PingPongFbo& getFlowValuesFbo() { return *flowValuesFboPtr; }
  PingPongFbo& getFlowVelocitiesFbo() { return *flowVelocitiesFboPtr; }
  const ofTexture& getDivergenceTexture() const { return divergenceRenderer.getFbo().getTexture(); }
  const ofTexture& getPressureTexture() const { return pressuresFbo.getSource().getTexture(); }
  const ofTexture& getCurlTexture() const { return vorticityRenderer.getFbo().getTexture(); }
  const ofTexture& getTemperatureTexture() const { return temperaturesFbo.getSource().getTexture(); }
  const ofTexture& getObstacleTexture() const {
    if (obstaclesFboPtr && obstaclesFboPtr->getSource().isAllocated()) return obstaclesFboPtr->getSource().getTexture();
    return flowValuesFboPtr->getSource().getTexture();
  }
   
  // NOTE: this is not used by the MarkSynth Fluid wrapper; it has dedicated Mods instead
   void applyImpulse(const FluidSimulation::Impulse& impulse) {
     if (!isValid()) return;

     flowValuesFboPtr->getSource().begin();

    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    softCircleShader.render(impulse.position, impulse.radius, impulse.color * impulse.colorDensity);
    ofPopStyle();
    flowValuesFboPtr->getSource().end();

    const float rawFrameDt = static_cast<float>(ofGetLastFrameTime());
    const float frameDt = clampFrameDt(rawFrameDt);
    constexpr float BASE_FPS = 30.0f;
    const float dt = dtParameter.get() * frameDt * BASE_FPS;

    const bool useObstacles = obstaclesEnabledParameter.get() && obstaclesFboPtr && obstaclesFboPtr->getSource().isAllocated();
    const ofTexture& obstaclesTex = useObstacles ? obstaclesFboPtr->getSource().getTexture()
                                                 : flowValuesFboPtr->getSource().getTexture();

    addRadialImpulseShader.render(*flowVelocitiesFboPtr,
                                  impulse.position,
                                  impulse.radius,
                                  impulse.velocity,
                                  impulse.radialVelocity,
                                  impulse.swirlVelocity,
                                  dt,
                                  obstaclesTex,
                                  useObstacles,
                                  obstacleThresholdParameter.get(),
                                  obstacleInvertParameter.get());
//    ofFloatColor velocityValue { impulse.velocity.r, impulse.velocity.g, 0.0, 1.0 };
//    softCircleShader.render(impulse.position, impulse.radius, velocityValue);
    
//    glm::vec4 temperatureValue { impulse.temperature, 0.0, 0.0, 0.0 };
//    addImpulseSpotShader.render(temperaturesFbo, impulse.position, impulse.radius, temperatureValue);
  }

  void applyTemperatureImpulse(const glm::vec2& positionPx, float radiusPx, float temperatureDelta) {
    if (!isValid()) return;
    if (temperatureDelta == 0.0f) return;

    temperaturesFbo.getSource().begin();

    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_ADD);

    // Use "dab" falloff so the injected value has a soft spatial falloff.
    softCircleShader.render(positionPx, radiusPx, ofFloatColor { temperatureDelta, 0.0f, 0.0f, 1.0f }, 0.35f, 1);

    ofPopStyle();

    temperaturesFbo.getSource().end();
  }
  
 private:
  static const char* boundaryModeToString(int mode) {
    switch (mode) {
      case 0: return "SolidWalls";
      case 1: return "Wrap";
      case 2: return "Open";
      default: return "Unknown";
    }
  }

  static const char* wrapModeToString(GLint wrap) {
    switch (wrap) {
      case GL_CLAMP_TO_EDGE: return "GL_CLAMP_TO_EDGE";
      case GL_REPEAT: return "GL_REPEAT";
      case GL_MIRRORED_REPEAT: return "GL_MIRRORED_REPEAT";
      default: return "(unknown)";
    }
  }

  GLint getExpectedWrapMode() const {
    switch (boundaryModeParameter.get()) {
      case 0: return GL_CLAMP_TO_EDGE;
      case 1: return GL_REPEAT;
      case 2: return GL_CLAMP_TO_EDGE;
      default: return GL_CLAMP_TO_EDGE;
    }
  }

  void setFboWrap(ofFbo& fbo, GLint wrap) {
    if (!fbo.isAllocated()) return;
    fbo.getTexture().setTextureWrap(wrap, wrap);
  }

  void applyExpectedWrapModeToInternalBuffers() {
    const GLint wrap = getExpectedWrapMode();

    // These buffers are always owned by the simulation.
    setFboWrap(pressuresFbo.getSource(), wrap);
    setFboWrap(pressuresFbo.getTarget(), wrap);
    setFboWrap(divergenceRenderer.getFbo(), wrap);
    setFboWrap(vorticityRenderer.getFbo(), wrap);
    setFboWrap(velocityDiffusionSourceFbo, wrap);
    setFboWrap(valueDiffusionSourceFbo, wrap);
    setFboWrap(temperatureDiffusionSourceFbo, wrap);
    setFboWrap(temperaturesFbo.getSource(), wrap);
    setFboWrap(temperaturesFbo.getTarget(), wrap);
  }

  void applyExpectedWrapModeToFlowBuffersIfOwned() {
    if (!ownsFlowBuffers) return;
    if (!flowValuesFboPtr || !flowVelocitiesFboPtr) return;

    const GLint wrap = getExpectedWrapMode();
    setFboWrap(flowValuesFboPtr->getSource(), wrap);
    setFboWrap(flowValuesFboPtr->getTarget(), wrap);
    setFboWrap(flowVelocitiesFboPtr->getSource(), wrap);
    setFboWrap(flowVelocitiesFboPtr->getTarget(), wrap);
  }

  static void allocateDiffusionSourceIfNeeded(ofFbo& fbo, int width, int height, GLint wrap) {
    if (width <= 0 || height <= 0) return;

    if (fbo.isAllocated() && static_cast<int>(fbo.getWidth()) == width && static_cast<int>(fbo.getHeight()) == height) {
      return;
    }

    ofFboSettings settings;
    settings.width = width;
    settings.height = height;
    settings.internalformat = GL_RGBA32F;
    settings.useDepth = false;
    settings.useStencil = false;
    settings.textureTarget = GL_TEXTURE_2D;
    settings.wrapModeHorizontal = wrap;
    settings.wrapModeVertical = wrap;

    fbo.allocate(settings);

    fbo.begin();
    ofClear(0, 0, 0, 0);
    fbo.end();
  }

  static void copyToFbo(const ofFbo& src, ofFbo& dst) {
    if (!dst.isAllocated()) return;

    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);

    dst.begin();
    ofClear(0, 0, 0, 0);
    src.draw(0, 0, dst.getWidth(), dst.getHeight());
    dst.end();

    ofPopStyle();
  }

  static float clampFrameDt(float frameDt) {
    // Startup frames sometimes report 0 dt; use a sane baseline so forces respond immediately.
    constexpr float STARTUP_DT = 1.0f / 30.0f;
    constexpr float MIN_DT = 1.0f / 240.0f;
    constexpr float MAX_DT = 1.0f / 15.0f;
    if (!std::isfinite(frameDt) || frameDt <= 0.0f) return STARTUP_DT;
    return std::clamp(frameDt, MIN_DT, MAX_DT);
  }

  static float persistenceToDissipation(float persistence, float frameDt, float minHalfLife, float maxHalfLife) {
    const float p = std::clamp(persistence, 0.0f, 1.0f);
    const float minHL = std::max(1.0e-3f, minHalfLife);
    const float maxHL = std::max(minHL, maxHalfLife);

    const float logMin = std::log(minHL);
    const float logMax = std::log(maxHL);
    const float halfLife = std::exp(logMin + (logMax - logMin) * p);

    if (frameDt <= 0.0f) return 1.0f;

    return std::exp(std::log(0.5f) * (frameDt / halfLife));
  }

  static float spreadToDiffusionRateCells(float spread, float minRateCells, float maxRateCells) {
    // Treat spread as a diffusion/viscosity rate in "cell units" (cells^2 per step-second).
    // This keeps behavior far more consistent across resolutions because `a = dt * rateCells`.
    const float s0 = std::clamp(spread, 0.0f, 1.0f);
    if (s0 <= 0.0f) return 0.0f;

    const float s = std::sqrt(s0);

    const float minR = std::max(1.0e-9f, minRateCells);
    const float maxR = std::max(minR, maxRateCells);

    const float logMin = std::log(minR);
    const float logMax = std::log(maxR);
    return std::exp(logMin + (logMax - logMin) * s);
  }

  static bool diffusionToJacobiParams(float rateCells, float dt, float& alpha, float& rBeta) {
    if (rateCells <= 0.0f || dt <= 0.0f) return false;

    // Here `rateCells` is the classic a = nu * dt / dx^2 expressed in cell units.
    // Our Jacobi shader computes: x_new = (sum(neighbors) + alpha * x0) * rBeta.
    // To match the standard implicit diffusion solve:
    //   x_new = (x0/a + sum(neighbors)) / (4 + 1/a)
    // so alpha = 1/a and rBeta = 1/(4 + 1/a).
    const float a = dt * rateCells;
    if (a <= 1.0e-9f) return false;

    const float invA = 1.0f / a;
    alpha = invA;
    rBeta = 1.0f / (4.0f + invA);
    return true;
  }

  static float applyDiffusionIfEnabled(PingPongFbo& field,
                                       JacobiShader& solver,
                                       ofFbo& diffusionSource,
                                       float spread,
                                       float dt,
                                       int iterations,
                                       float minRateCells,
                                       float maxRateCells,
                                       const ofTexture& obstaclesTex,
                                       bool obstaclesEnabled,
                                       float obstacleThreshold,
                                       bool obstacleInvert) {
    if (iterations <= 0) return 0.0f;

    const float rateCells = spreadToDiffusionRateCells(spread, minRateCells, maxRateCells);
    float alpha = 0.0f;
    float rBeta = 0.0f;
    if (!diffusionToJacobiParams(rateCells, dt, alpha, rBeta)) return 0.0f;

    copyToFbo(field.getSource(), diffusionSource);
    solver.render(field,
                  diffusionSource.getTexture(),
                  dt,
                  alpha,
                  rBeta,
                  iterations,
                  obstaclesTex,
                  obstaclesEnabled,
                  obstacleThreshold,
                  obstacleInvert);
    return rateCells;
  }

  void applyVelocityBoundariesIfNeeded() {
    if (boundaryModeParameter.get() != 0) return; // SolidWalls only for now
    velocityBoundaryShader.render(*flowVelocitiesFboPtr);
  }

  void applyVelocityCflClamp(float dt) {
    if (dt <= 0.0f) return;
    if (!flowVelocitiesFboPtr) return;

    const float gridSize = std::min(flowVelocitiesFboPtr->getWidth(), flowVelocitiesFboPtr->getHeight());
    const float dx = 1.0f / std::max(1.0f, gridSize);

    // Allow at most N cells displacement per step.
    constexpr float CFL_CELLS = 4.0f;
    const float maxDisp = CFL_CELLS * dx;

    velocityCflClampShader.render(*flowVelocitiesFboPtr, dt, maxDisp);
  }

  void clearPressureIfNeeded() {
    if (!pressureNeedsClear) return;

    auto clearFbo = [](ofFbo& fbo) {
      if (!fbo.isAllocated()) return;
      fbo.begin();
      fbo.clearColorBuffer(ofFloatColor(0.0f, 0.0f, 0.0f, 0.0f));
      fbo.end();
    };

    clearFbo(pressuresFbo.getSource());
    clearFbo(pressuresFbo.getTarget());
    pressureNeedsClear = false;
  }

  bool validateExternalBuffers() {
    valid = false;
    validationLogged = false;
    validationError.clear();

    applyExpectedWrapModeToFlowBuffersIfOwned();
    applyExpectedWrapModeToInternalBuffers();

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

    const int boundaryMode = boundaryModeParameter.get();
    const GLint expectedWrap = getExpectedWrapMode();

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

      if (data.wrapModeHorizontal != expectedWrap || data.wrapModeVertical != expectedWrap) {
        validationError = std::string("FluidSimulation boundary mode ") + boundaryModeToString(boundaryMode) + " requires "
                          + wrapModeToString(expectedWrap) + " wrap; got " + wrapModeToString(data.wrapModeHorizontal) + ","
                          + wrapModeToString(data.wrapModeVertical) + " for " + label;
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

    if (obstaclesEnabledParameter.get()) {
      if (!obstaclesFboPtr || !obstaclesFboPtr->getSource().isAllocated()) {
        validationError = "FluidSimulation ObstaclesEnabled requires an allocated obstacles buffer";
        logValidationErrorOnce();
        return false;
      }

      const int w = static_cast<int>(flowVelocitiesFboPtr->getWidth());
      const int h = static_cast<int>(flowVelocitiesFboPtr->getHeight());
      const int ow = static_cast<int>(obstaclesFboPtr->getWidth());
      const int oh = static_cast<int>(obstaclesFboPtr->getHeight());
      if (ow != w || oh != h) {
        validationError = std::string("FluidSimulation obstacles size must match velocities (expected " )
                          + ofToString(w) + "x" + ofToString(h) + " got " + ofToString(ow) + "x" + ofToString(oh) + ")";
        logValidationErrorOnce();
        return false;
      }

      if (!validateTexture(obstaclesFboPtr->getSource().getTexture(), "obstacles")) {
        logValidationErrorOnce();
        return false;
      }
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

  // Boundary mode is currently used primarily for validation + velocity wall handling.
  // 0: SolidWalls, 1: Wrap, 2: Open
  ofParameter<int> boundaryModeParameter { "Boundary Mode", 0, 0, 2 };

  // Interpreted as a dimensionless simulation speed tuned around a baseline framerate.
  // Effective dt used in the solver is: dtEffective = ofGetLastFrameTime() * BASE_FPS * dt.
  // With BASE_FPS=30, dtEffective ~= dt at 30fps.
  ofParameter<float> dtParameter { "dt", 0.03f, 0.0f, 0.04f };

  // Normalized artist control. Internally mapped to a vorticity confinement strength.
  ofParameter<float> vorticityParameter { "Vorticity", 0.15f, 0.0f, 1.0f };

  // NOTE: These are normalized persistence controls (0..1), not per-frame multipliers.
  // We'll rename these keys during bulk config migration.
  ofParameter<float> valueAdvectDissipationParameter { "Value Dissipation", 0.92f, 0.0f, 1.0f };
  ofParameter<float> velocityAdvectDissipationParameter { "Velocity Dissipation", 0.4f, 0.0f, 1.0f };

  // Normalized diffusion/viscosity controls.
  ofParameter<float> valueSpreadParameter { "Value Spread", 0.2f, 0.0f, 1.0f };
  ofParameter<float> velocitySpreadParameter { "Velocity Spread", 0.7f, 0.0f, 1.0f };

  // Clamp values after advection to prevent unbounded dye accumulation.
  // Set to 0 to disable.
  ofParameter<float> valueMaxParameter { "Value Max", 1.0f, 0.0f, 10.0f };

  ofParameterGroup temperatureParameters { "Temperature" };
  ofParameter<bool> temperatureEnabledParameter { "TempEnabled", false };
  ofParameter<float> temperatureAdvectDissipationParameter { "Temperature Dissipation", 0.9f, 0.0f, 1.0f };
  ofParameter<float> temperatureSpreadParameter { "Temperature Spread", 0.0f, 0.0f, 1.0f };
  ofParameter<int> temperatureDiffusionIterationsParameter = JacobiShader::createIterationsParameter("Temperature ", 0);

  ofParameterGroup obstaclesParameters { "Obstacles" };
  ofParameter<bool> obstaclesEnabledParameter { "ObstaclesEnabled", false };
  ofParameter<float> obstacleThresholdParameter { "Obstacle Threshold", 0.5f, 0.0f, 1.0f };
  ofParameter<bool> obstacleInvertParameter { "Obstacle Invert", false };

  ofParameter<int> valueDiffusionIterationsParameter = JacobiShader::createIterationsParameter("Value ", 1);
  ofParameter<int> velocityDiffusionIterationsParameter = JacobiShader::createIterationsParameter("Velocity ", 1);
  ofParameter<int> pressureDiffusionIterationsParameter = JacobiShader::createIterationsParameter("Pressure ", 10);
  ofParameterGroup buoyancyParameters { "Buoyancy" };
  ofParameter<float> buoyancyStrengthParameter = ApplyBouyancyShader::createBuoyancyStrengthParameter();
  ofParameter<float> buoyancyDensityScaleParameter = ApplyBouyancyShader::createDensityScaleParameter();
  ofParameter<float> buoyancyThresholdParameter = ApplyBouyancyShader::createDensityThresholdParameter();
  ofParameter<float> gravityForceXParameter = ApplyBouyancyShader::createGravityForceXParameter();
  ofParameter<float> gravityForceYParameter = ApplyBouyancyShader::createGravityForceYParameter();
  ofParameter<bool> buoyancyUseTemperatureParameter { "Use Temperature", false };
  ofParameter<float> ambientTemperatureParameter { "Ambient Temperature", 0.0f, -1.0f, 1.0f };
  ofParameter<float> temperatureBuoyancyThresholdParameter { "Temperature Threshold", 0.0f, 0.0f, 1.0f };
  

  bool valid = false;
  bool validationLogged = false;
  std::string validationError;

  bool pressureNeedsClear = true;

  bool ownsFlowBuffers = false;
  int lastBoundaryMode = 0;
  bool lastObstaclesEnabled = false;

  std::shared_ptr<PingPongFbo> flowValuesFboPtr;
  std::shared_ptr<PingPongFbo> flowVelocitiesFboPtr;
  std::shared_ptr<PingPongFbo> obstaclesFboPtr;
  PingPongFbo temperaturesFbo;

  AdvectShader valueAdvectShader;
  AdvectShader velocityAdvectShader;
  AdvectShader temperatureAdvectShader;

  JacobiShader valueJacobiShader;
  JacobiShader velocityJacobiShader;
  JacobiShader temperatureJacobiShader;
  DivergenceRenderer divergenceRenderer;
  PingPongFbo pressuresFbo;
  JacobiShader pressureJacobiShader;
  SubtractDivergenceShader subtractDivergenceShader;
  VorticityRenderer vorticityRenderer;
  ApplyVorticityForceShader applyVorticityForceShader;
  VelocityBoundaryShader velocityBoundaryShader;
  VelocityCflClampShader velocityCflClampShader;
  ofFbo velocityDiffusionSourceFbo;
  ofFbo valueDiffusionSourceFbo;
  ofFbo temperatureDiffusionSourceFbo;
  ApplyBouyancyShader applyBouyancyShader;
  ApplyTemperatureBuoyancyShader applyTemperatureBuoyancyShader;
  
  SoftCircleShader softCircleShader;
  AddRadialImpulseShader addRadialImpulseShader;

  DebugStepInfo debugStepInfo;
};
